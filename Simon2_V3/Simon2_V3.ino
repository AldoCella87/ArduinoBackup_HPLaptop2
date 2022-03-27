/*
 * memory / dexterit therapy game for Steve T.
 */


/*
 * TODO:  
 * add adaptive difficulty setting based on user performance with adjustable gain (low slope for beginners.
 * Add deliery of Amazon gift card via servo motor
 * add internal battery (with extension USB port) show battery level on display
 * add wav file support for 80's style arcage game sounds (asteriods, pac man, etc.)
 * use LCD display logic from SORI game (complete)
 * startup mode configuration options (press any button during startup)
 * add bluetooth interface (display score, statistics, etc.
 * capture response time for performance metrics
 * add real time clock to track performance over time
 * add timer - response must be received within specified time
 * "Press both red buttons simultaneously to end game and view stats"
 * build array of player performance statistics
 * have special challenges which display fibonacci sequence of some other well known integer sequence
 * 
 */
#include <LiquidCrystal_I2C.h>

// setup LCD display
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 
 
#define MAX_CHALLENGE_LENGTH  10      // maximum number of elements in sequence array
#define SOUND_CORRECT 0       // mode control to command audio sound signifying correct response
#define SOUND_INCORRECT 1     // mode control to command audio sound signifying correct response
#define SOUND_SUCCESSFUL_ROUND 2  // audio to signify successful round
#define END_OF_CHALLENGE_FLAG 99  // signifies end of challenge array
#define NUM_ELEMENTS 5             // number of player buttons

int iReading1 = 0;
int iReading2 = 0;
int iReading3 = 0;
int iReading4 = 0;
int iReading5 = 0;

int iPrevReading1 = 0;
int iPrevReading2 = 0;
int iPrevReading3 = 0;
int iPrevReading4 = 0;
int iPrevReading5 = 0;
  
int iResponseIndex = 0;

// caution, must be set manualy to match NUM_ELEMENTS
int buttonPins[NUM_ELEMENTS] = {2, 3, 4, 5, 6};
const int ledPins[NUM_ELEMENTS] = {9, 10, 11, 12, 13};

const int alertPin = 7;       // sonalert

const int ledPin1 =  9;       // the number of the LED pin
const int ledPin2 =  10;      // the number of the LED pin
const int ledPin3 =  11;      // the number of the LED pin
const int ledPin4 =  12;      // the number of the LED pin
const int ledPin5 =  13;      // the number of the LED pin

const unsigned int debounceDelay = 40;  
int challengeSequence[MAX_CHALLENGE_LENGTH+1];           //light sequence array, reserve one space for end flag

int iResponses[MAX_CHALLENGE_LENGTH];         // array to hold sequence of responses

bool bDisplayNextChallenge = true;     // displays light sequence when true, waits for response when false 

// response structure holds data for the five response buttons
struct ResponseStruct{    
  int iReading;
  int iPrevReading;
  unsigned long PositiveTransitionTime;
  unsigned long PreviousMillis;
  bool bButtonState;
};
ResponseStruct Responses[MAX_CHALLENGE_LENGTH];

unsigned long deadline = 0;


#define MAX_ROUNDS 50
struct StatsStruct{
  int RoundID = 0;
  unsigned long roundStartTime;
  unsigned long roundDeadline;
  unsigned long playerResponseTime;
  bool bRoundSuccess;
  bool bTimeout;
  bool bRoundFailure;
} RoundStatistics[MAX_ROUNDS];





// keep running stats on player performance
unsigned int iRoundNumber = 0;
unsigned int iTimeoutCount = 0;
unsigned int iResponseErrorCount = 0;
unsigned int iSuccessCount = 0;

void setup() {

  // LCD display setup
  lcd.off(); 
  lcd.begin (20,4);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  // end LCD setup

  for(int i = 0; i<NUM_ELEMENTS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT);
  }
   pinMode(alertPin, OUTPUT);

  Serial.begin(9600);

  displayLCD(F("Hi Steve. I'm Simone"),F("Let's play..."),"","", 0,3500);

}

void loop() {

  if (bDisplayNextChallenge){
    // increment round number
    iRoundNumber++;
    
    SetupAndDisplayNextMove();

    // prepare to receive responses
    iResponseIndex = 0;     // initialize index which stores series of responses 
    // initialize button response booleans
    for(int i = 0; i<NUM_ELEMENTS; i++) Responses[i].bButtonState = false;

    // notify player 
    displayLCD(F("Your turn..."),String(iRoundNumber),"","", 0,0);

    // prepare flags for next round
    RoundStatistics[iRoundNumber].bRoundSuccess = false;
    RoundStatistics[iRoundNumber].bTimeout = false;
    RoundStatistics[iRoundNumber].bRoundFailure = false;
    RoundStatistics[iRoundNumber].roundStartTime = millis();

  }
  else
  {
  // handle player response
  
  for(int j = 0; j<NUM_ELEMENTS; j++){
  /*
   * scan all buttons 
   * 
   */
  Responses[j].iReading = digitalRead(buttonPins[j]);
  if ((Responses[j].iReading == HIGH) && (Responses[j].iPrevReading == LOW)) {
    // get time of newly detected high state
    Responses[j].PositiveTransitionTime = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((Responses[j].iReading == HIGH) && (Responses[j].iPrevReading == HIGH)){
    if (((millis() - Responses[j].PositiveTransitionTime) > debounceDelay && (Responses[j].bButtonState == false))) {
         Responses[j].bButtonState = true;
         Serial.print("Button state ON registered for button ");
         Serial.println(j);
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPins[j];             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPins[j]){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);                   // provide affirmative sound
            digitalWrite(ledPins[j], HIGH);                    // light LED
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            RoundStatistics[iRoundNumber].bRoundFailure = true;
            bDisplayNextChallenge = true;                            // player fails this round, redisplay challenge
            }
          // we have a response
          iResponseIndex++;                                 // prepare to store next response
          }
        }
      }
    else if ((Responses[j].iReading == LOW) && (Responses[j].iPrevReading == HIGH)){
      Responses[j].bButtonState = false;      // player has released the button, reset to allow it to be registered again
      digitalWrite(ledPins[j], LOW);                    // extinguish LED
      // check for end of sequence
      if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG){
        // reaching this point means all responses were correct
        bDisplayNextChallenge = true;                      // exit responses and display new challenge
        RoundStatistics[iRoundNumber].bRoundSuccess = true;
        displayLCD(F("Success!"),F(""),"","", 0,1000);
        SoundFeedback(SOUND_SUCCESSFUL_ROUND);    // sound the success tone
        delay(500);       
        // TODO - add "press any key to continue"??
        }
      }
      Responses[j].iPrevReading = Responses[j].iReading;  // save current reading as previous
    } // process next button


    // Done with current scan of buttons, check for timeout
    if(millis() > RoundStatistics[iRoundNumber].roundStartTime + deadline){
      RoundStatistics[iRoundNumber].bTimeout = true;
      bDisplayNextChallenge = true;
      displayLCD(F("Timeout"),F(""),"","", 0,2000);
    }
    
  } // done processing responses
}




/*
 * DisplaySequence - displays the challenge to the user
 * by sequencing a number of lights on the front panel.
 */
void DisplaySequence(int challengeElements[], int iDuration)
{
  int i = 0;
  while(challengeElements[i] != END_OF_CHALLENGE_FLAG) 
  {
    digitalWrite(challengeElements[i], HIGH);
    delay(iDuration);
    digitalWrite(challengeElements[i], LOW);
    delay(iDuration * 1.5); 
    i++;   
  }
}

void SoundFeedback(int iMode)
{
  switch(iMode)
  {
    case SOUND_CORRECT:
      digitalWrite(alertPin, HIGH);
      delay(10);
      digitalWrite(alertPin, LOW);
      break;   
    
    case SOUND_INCORRECT:
      for(int i = 0;i<9;i++)
      {
      digitalWrite(alertPin, HIGH);
      delay(6);
      digitalWrite(alertPin, LOW);
      delay(15);
      }     
      break;   

    case SOUND_SUCCESSFUL_ROUND:

      digitalWrite(alertPin, HIGH);
      delay(50);
      digitalWrite(alertPin, LOW);
      delay(120);
      digitalWrite(alertPin, HIGH);
      delay(400);
      digitalWrite(alertPin, LOW);
      delay(500);          
      break;   
   
    default:
      break;
  }

}

/*********************************************************************
 * general purpose display manager with optional tone alert and delay
 * 
 *********************************************************************/
void displayLCD(String sline1, String sline2, String sline3, String sline4, int itoneLen, int idelay)
{
    lcd.off(); 
    lcd.clear();          
    lcd.home (); 
    lcd.print(sline1); 
    lcd.setCursor (0,1);        
    lcd.print(sline2);
    lcd.setCursor (0,2);        
    lcd.print(sline3); 
    lcd.setCursor (0,3);        
    lcd.print(sline4);          
    lcd.on();  
    if (itoneLen > 0){
      digitalWrite(alertPin, HIGH);
      delay(itoneLen);
      digitalWrite(alertPin, LOW);
    }
    if (idelay > 0) delay(idelay);
}

/*********************************************************************
 * setup next move then display it.  
 * Set next move based on past performance
 * set number of elements, sequence of elements, display speed and deadline.
 *********************************************************************/
void SetupAndDisplayNextMove(void)
{

  displayLCD(F("Calculating"),F("next move..."),F(""),F(""),0,1000);

    // setup sequence
    for(int i = 0; i < MAX_CHALLENGE_LENGTH + 1; i++) challengeSequence[i] = END_OF_CHALLENGE_FLAG; 
    /***
    challengeSequence[0] = ledPin1;
    challengeSequence[1] = ledPin2;
    challengeSequence[2] = ledPin3;
    challengeSequence[3] = ledPin4;
    challengeSequence[4] = ledPin5;
    challengeSequence[5] = ledPin1;
    challengeSequence[6] = ledPin2;
    challengeSequence[7] = ledPin3;
    challengeSequence[8] = ledPin4;
    challengeSequence[9] = ledPin5;
    ***/
    
    // clear display so as not to distract player
    lcd.clear();     
  
    for(int i = 0; i < random(2,5); i++) // random upper limit is number specified minus 1
    {
      challengeSequence[i] = ledPins[random(0,NUM_ELEMENTS+1)];    
    }

    // display sequence
    DisplaySequence(challengeSequence, 300);   
    bDisplayNextChallenge = false;                    // challenge has been displayed, now get responses
  

  // TODO - add dynamic calculation of deadline
  deadline = 12000;
  
  displayLCD(F(""),F(""),F(""),F(""), 0,2000);
    


    
}
