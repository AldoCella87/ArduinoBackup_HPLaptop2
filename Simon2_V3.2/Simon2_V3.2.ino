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
 * startup mode configuration options (press any button during startup), set level, sound option, number of rounds per game
 * add bluetooth interface (display score, statistics, etc.
 * capture response time for performance metrics
 * add real time clock to track performance over time
 * add timer - response must be received within specified time
 * "Press both red buttons simultaneously to end game and view stats"
 * build array of player performance statistics
 * have special challenges which display fibonacci sequence of some other well known integer sequence
 * need a way to suspend game?
 * printout of results?
 * add time in play
 * add delivery of amazon gift card when certain conditions are met (time in play, level, etc.)
 * 
 */


// following supports df mini
// dfmini connections (spkr, gnd, spkr, nc, nc, rcv, vcc, tx (w/1K resistor))
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
SoftwareSerial mySoftwareSerial(A0, A1); // RX, TX+

DFRobotDFPlayerMini myDFPlayer;

#define DFMINI_SUCCESS  2
#define DFMINI_ERROR  1
// end df mini


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
#define NUM_DIFFICULTY_LEVELS 3

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
int buttonPins[NUM_ELEMENTS] = {2, 3, 4, 5, 6};                  //R, Y, G, B, W
const int ledPins[NUM_ELEMENTS] = {9, 10, 11, 12, 13};           //R, Y, G, B, W

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


/*** future...
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
***/

  unsigned long setupStartTime = 0;
  unsigned long roundStartTime = 0;
  unsigned long roundDeadline = 0;
  unsigned long playerResponseTime = 0;
  bool bRoundSuccess = false;
  bool bTimeout = false;
  bool bRoundFailure = false;




// keep running stats on player performance
unsigned int iRoundNumber = 0;
unsigned int iTimeoutCount = 0;
unsigned int iErrorCount = 0;
unsigned int iSuccessCount = 0;
unsigned int iDifficultyLevel = 1;

void setup() {

  int i,j;
  // support for df mini
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  
  Serial.println();
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
 
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(25);  //Set volume value. From 0 to 30
  // end df mini
  
  
  // LCD display setup
  lcd.off(); 
  lcd.begin (20,4);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  // end LCD setup

  for(i = 0; i<NUM_ELEMENTS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT);
  }
   pinMode(alertPin, OUTPUT);

  Serial.begin(9600);
  randomSeed(analogRead(0));                

/************************** future feature
#define WAIT_FOR_SETUP_KEY 6000
  // setup
  lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Welcome to Simone V3")); 
  lcd.setCursor (0,1);        
  lcd.print(F("Hit any key to setup"));         
  lcd.on();
  if(GetKeystroke(WAIT_FOR_SETUP_KEY) > -1) Setup();
************************/

  // welcome message
  lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Hi Steve. I'm Simone")); 
  lcd.setCursor (0,1);        
  lcd.print(F("Just follow me...   "));         
  //lcd.setCursor (0,2);        
  //lcd.print(F("repeat the sequence."));    
  lcd.setCursor (0,3);        
  lcd.print(F("Hit any key to start"));
  lcd.on();
  WaitForAnyKey();
  lcd.clear();
}

void loop() {
  
  if (bDisplayNextChallenge){
    // Ready for next challenge.  increment round number

    if((iRoundNumber % 5) == 0)
    {
      displayStatistics();
      //delay(3000);// FIXME
      WaitForAnyKey();  
      lcd.clear();
    }
    else
    {
      lcd.clear();
    }
    delay(1500);                // give user time to focus back on lights     

    iRoundNumber++;
        
    SetupAndDisplayNextMove();

    // prepare to receive responses
    iResponseIndex = 0;     // initialize index which stores series of responses 
    // initialize button response booleans for upcoming round
    for(int i = 0; i<NUM_ELEMENTS; i++) Responses[i].bButtonState = false; 

    // initialize flags for upcoming round
    bRoundSuccess = false;
    bTimeout = false;
    bRoundFailure = false;
    roundStartTime = millis();
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
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPins[j];             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPins[j]){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);                   // provide affirmative sound
            digitalWrite(ledPins[j], HIGH);                    // light LED
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bRoundFailure = true;
            iErrorCount++;
            bDisplayNextChallenge = true;                            // player fails this round, redisplay challenge
            // display results
            //lcd.setCursor (9,3);
            //lcd.print(iErrorCount);     
  
            myDFPlayer.play(DFMINI_ERROR); 
            
            lcd.setCursor (0,1); 
            lcd.print(F("*Error*             "));          
            delay(1000);
            
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
        //SoundFeedback(SOUND_SUCCESSFUL_ROUND);    // sound the success tone  
        bDisplayNextChallenge = true;                      // exit responses and display new challenge
        bRoundSuccess = true;
        iSuccessCount++;
        myDFPlayer.play(DFMINI_SUCCESS);
        // display results
        //lcd.setCursor (0,3);
        //lcd.print(iSuccessCount);          
        //lcd.setCursor (0,1); 
        //lcd.print(F("Success!            "));      
        //delay(1000);
     
        }
      }
      Responses[j].iPrevReading = Responses[j].iReading;  // save current reading as previous
    } // process next button


  // Done with current scan of buttons, check for timeout only if error or success was not encountered
    if((millis() > roundStartTime + deadline) && (bDisplayNextChallenge == false)){
      bTimeout = true;
      iTimeoutCount++;      
      bDisplayNextChallenge = true;
      // display results
      //lcd.setCursor (17,3);
      //lcd.print(iTimeoutCount); 
      myDFPlayer.play(DFMINI_ERROR);           
      lcd.setCursor (0,1); 
      lcd.print(F("Timeout  hit any key"));          
      WaitForAnyKey();
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
      delay(8);
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
 * setup next move then display it.  
 * Set next move based on past performance
 * set number of elements, sequence of elements, display speed and deadline.
 *********************************************************************/
void SetupAndDisplayNextMove(void)
{
  float fSuccessPct = 0.0;
  float fTimeoutPct = 0.0;
  float fErrorPct = 0.0;
  int iDisplayDelay = 300;        // higher number displays sequence more slowly

  fSuccessPct = float(iSuccessCount) / float(iRoundNumber);
  fTimeoutPct = float(iTimeoutCount) / float(iRoundNumber);
  fTimeoutPct = float(iErrorCount) / float(iRoundNumber);
  
  #define MIN_L1TOL2 40   // minimum number of rounds required to move to level 2
  #define MIN_L2TOL3 60   // minimum number of rounds required to move to level 3

  // setup difficulty level based on past performance - fine tune this!
  // can also modulate other parameters...
  if(iDifficultyLevel == 1){
    if((fSuccessPct > .75) && (iRoundNumber > MIN_L1TOL2))iDifficultyLevel++;
  }
  else if(iDifficultyLevel == 2){
         if((fSuccessPct > .85) && (iRoundNumber > MIN_L2TOL3))iDifficultyLevel++; 
  }
  // revert difficulty level if player does not continue to have success
  if((fSuccessPct < .80) && iDifficultyLevel > 1) iDifficultyLevel--;

  // setup sequence
  // initialize challenge sequence matrix
  for(int i = 0; i < MAX_CHALLENGE_LENGTH + 1; i++) challengeSequence[i] = END_OF_CHALLENGE_FLAG; 

  switch(iDifficultyLevel)
  {
  case 1:
      // set parameters
      deadline = 25000;   
      iDisplayDelay = 650;
      // establish challenge sequence
      for(int i = 0; i < random(2,4); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_ELEMENTS)];     
      }
      break;
  case 2:
      // set parameters
      deadline = 20000;   
      iDisplayDelay = 300;
      // establish challenge sequence
      for(int i = 0; i < random(3,5); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_ELEMENTS)];     
      }
      break;

  case 3:
      // set parameters
      deadline = 15000;   
      iDisplayDelay = 200;
      // establish challenge sequence
      for(int i = 0; i < random(3,6); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_ELEMENTS)];     
      }
      break;      
  default:
    break;
  }

    // display sequence
    DisplaySequence(challengeSequence, iDisplayDelay);   
    
    bDisplayNextChallenge = false;                    // challenge has been displayed, set flag to wait for responses

}

/*********************************************************************
 * display statistics
 * 
 *********************************************************************/
void displayStatistics(void)
{
    lcd.off(); 
    lcd.clear();  
    lcd.setCursor (0,0);        
    lcd.print(F("Level "));
    lcd.print(iDifficultyLevel);
    lcd.print(F(" Round "));
    lcd.print(iRoundNumber);
    lcd.setCursor (0,2);        
    lcd.print("Correct  Err Timeout");          
    lcd.setCursor (0,3);
    lcd.print(iSuccessCount);
    lcd.setCursor (9,3);
    lcd.print(iErrorCount);
    lcd.setCursor (17,3);
    lcd.print(iTimeoutCount);    
    lcd.on();
}


/*
 * waits for keystroke.   may be modified to return
 * key index if needed.  caution, this blocks further processing
 */
void WaitForAnyKey(void){
int i,j;
  do{
    for(i = 0;i<NUM_ELEMENTS; i++){
      j=digitalRead(buttonPins[i]);
      if(j == HIGH) break;
    }
  } while(j == LOW);  
}


/*
 * gets keystroke from player if provided within wait time
 * returns:  index of key number if successful, -1 if timeout
 */
int GetKeystroke(unsigned long ulWaitTime){
  unsigned long PositiveTransitionTime = 0;
  unsigned long ulDebounceDelay = 75;
  unsigned long ulStart = 0;
  bool bButtonState = false;
  int iRetnValue = 0;
  int j = 0;
  int iReading[NUM_ELEMENTS];
  int iPrevReading[NUM_ELEMENTS];
  ulStart = millis();
  while(((ulStart + ulWaitTime) > millis()) && bButtonState == false)
  {
    for(j = 0; j<NUM_ELEMENTS; j++){
    iReading[j] = digitalRead(buttonPins[j]);
    if ((iReading[j] == HIGH) && (iPrevReading[j] == LOW)) {
      // get time of newly detected high state
      PositiveTransitionTime = millis();
    }
    // button has been pushed for at least one cycle,  accumulate time until we reach threshold
    else if ((iReading[j] == HIGH) && (iPrevReading[j] == HIGH)){
      if (((millis() - PositiveTransitionTime) > ulDebounceDelay && (bButtonState == false))) {
           bButtonState = true;           // signify exit for outer loop
           break;                         // button has been identified, exit inner loop
        }   
      }
      iPrevReading[j] = iReading[j];      // store previous reading
    }
  }
  if(bButtonState == true){
    iRetnValue = j;                       // return index of search showing which button was pressed
  }
  else{
    iRetnValue = -1;                      // signify no button pressed within designated time window
  }
  return(iRetnValue);
}


/*
 * set user editable configuration values
 * 
 */

 void Setup(void)
 {

  
 }
