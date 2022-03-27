/*
 * memory / dexterit therapy game for Steve T.
 * Al Evangelista 2/2/2020
 */

/*
 * TODO:  
 * add adaptive difficulty setting based on user performance with adjustable gain (low slope for beginners) - DONE
 * Add deliery of Amazon gift card via servo motor
 * add internal battery (with extension USB port) show battery level on display
 * add wav file support for 80's style arcage game sounds (asteriods, pac man, etc.)  - Done
 * use LCD display logic from SORI game (complete) - DONE  (using 4x20 display)
 * startup mode configuration options (press any button during startup), set level, sound option, number of rounds per game   - DONE
 * add bluetooth interface (display score, statistics, etc.
 * capture response time for performance metrics - DONE
 * add real time clock to track performance over time
 * add timer - response must be received within specified time
 * "Press both red buttons simultaneously to end game and view stats"
 * build array of player performance statistics
 * have special challenges which display fibonacci sequence of some other well known integer sequence
 * need a way to suspend game?
 * printout of results?
 * add time in play
 * add delivery of amazon gift card when certain conditions are met (time in play, level, etc.)
 * only display global statistics when changing levels - DONE
 * add tone to indicate start of "your move"  - DONE
 * remove inter-element delay after delivery of last challenge element.  Having this in place prevents the system from immediately accepting 
 *    user input - DONE
 * add multi-player mode (perhaps by 5 play rounds...)  allow multiple devices to interact by bluetooth, perhaps each performing the same challenge?
 */


// following supports df mini
// dfmini connections (spkr, gnd, spkr, nc, nc, rcv to analog 0, tx w/1K resistor to analog 1, vcc)
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(A0, A1); // RX, TX+
DFRobotDFPlayerMini myDFPlayer;

// MP3 file numbers for DF player
#define SOUND_BUZZER      1
#define SOUND_DOG         2
#define SOUND_APPLAUSE    3
#define SOUND_BELL        4
#define SOUND_ALARM       5
// end df mini

// setup LCD display
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 

#define xxxEST_MODE
#ifdef TEST_MODE
    // test parameters here
    #define SOUND_VOLUME  21
    #define MAX_ROUNDS  999                 // force software reset after this many rounds
    #define LVL_DECREMENT_THRESHOLD 2       // number of consecutive failures beyond which level will revert
    #define LVL_INCREMENT_THRESHOLD 3       // number of consecutive wins beyond which level will increase
    #define MIN_PLAYS_IN_LEVEL      3       // minimum number of rounds required to be spent in each level
    #define SPECIAL_GREETING1_TIME  3       // round number to play special greeting                   
#else
    // normal parameters here
    #define SOUND_VOLUME  21
    #define MAX_ROUNDS  999                 // force software reset after this many rounds
    #define LVL_DECREMENT_THRESHOLD 2       // number of consecutive failures beyond which level will revert
    #define LVL_INCREMENT_THRESHOLD 10      // number of consecutive wins beyond which level will increase
    #define MIN_PLAYS_IN_LEVEL      20      // minimum number of rounds required to be spent in each level
    #define SPECIAL_GREETING1_TIME  92      // round number to play special greeting     

#endif

// non test mode parameters
#define MAX_CHALLENGE_LENGTH    10          // maximum number of elements in sequence array
#define PIEZO_CORRECT           0           // make click as audio feedback for key press
#define PIEZO_YOUR_MOVE         1           // make click as audio feedback for key press
#define END_OF_CHALLENGE_FLAG   99          // signifies end of challenge array
#define NUM_BUTTONS             5           // number of player buttons
#define SETUP_EXIT_KEY          2           // key used to save displayed options and exit setup
#define SETUP_TOGGLE_KEY        0           // key used to toggle between options
#define SETUP_ENTRY_KEY         0           // key used to enter setup mode, press this key during power up to enter setup
  
int iPlaysInLevel = 0;                      // number of plays which have been spent in the current level, reset at each change in level
int iResponseIndex = 0;

int iConsecutiveWins = 0;                   //governs level progression
int iConsecutiveLosses = 0;                 //governs level progression

// caution, initial values must be set manualy to match NUM_BUTTONS
int buttonPins[NUM_BUTTONS] = {2, 3, 4, 5, 6};                  //R, Y, G, B, W
const int ledPins[NUM_BUTTONS] = {9, 10, 11, 12, 13};           //R, Y, G, B, W

const int alertPin = 7;       // sonalert for audible 'click' when button is pressed and other optional uses

const unsigned int debounceDelay = 20;                    // time in MS which button must be pressed to count as a press
int challengeSequence[MAX_CHALLENGE_LENGTH+1];            // challenge sequence array, reserve one additional space for end flag
int iResponses[MAX_CHALLENGE_LENGTH];                     // array to hold sequence of responses
bool bDisplayNextChallenge = true;                        // displays light sequence when true, waits for response when false 

// response structure holds data for each response button
struct ResponseStruct{    
  int iReading;
  int iPrevReading;
  unsigned long PositiveTransitionTime;
  unsigned long PreviousMillis;
  bool bButtonState;
};
ResponseStruct Responses[MAX_CHALLENGE_LENGTH];

unsigned long iResponseDeadline = 0;
unsigned long roundStartTime = 0;

// flags for results of individual rounds
bool bRoundSuccess = false;
bool bTimeout = false;
bool bRoundFailure = false;

int iDisplayDelay = 0;                    // controls how slowly challenge is presented.  Higher number displays sequence more slowly

// keep running stats on player performance  Note these
// are reset once player moves from one level to another
unsigned int iLevelRoundNumber = 0;              
unsigned int iLevelTimeoutCount = 0;
unsigned int iLevelErrorCount = 0;
unsigned int iLevelSuccessCount = 0;
unsigned int iDifficultyLevel = 1;

// global statistics  These persist for the entire game, displayed every n round.
int iTotalRounds  = 0;                      
int iTotalErrors  = 0;
int iTotalSuccess = 0;
int iTotalTimeout = 0;

int iLevelScaleFactor = 1;

void setup() {
  int i;
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
  
  myDFPlayer.volume(SOUND_VOLUME);  //Set volume value. From 0 to 30
  // end df mini
  
  // LCD display setup
  lcd.off(); 
  lcd.begin (20,4);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  // end LCD setup

  // set pinmodes for inputs and outputs for push button array
  for(i = 0; i<NUM_BUTTONS; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT);
  }

  // pinmode for sonalert pin
  pinMode(alertPin, OUTPUT);

  randomSeed(analogRead(0));                

  // detect player request to enter setup mode
  if(digitalRead(buttonPins[SETUP_ENTRY_KEY]) == HIGH){         // user hits red button during startup

    lcd.off(); 
    lcd.clear();          
    lcd.home (); 
    lcd.print(F("Level       Beginner")); 
    lcd.setCursor (0,2);        
    lcd.print(F("Red =   view options"));           
    lcd.setCursor (0,3);        
    lcd.print(F("Green =  save & exit"));
    lcd.on();
  
    // wait for user to release the button
    while(digitalRead(buttonPins[0])== HIGH);
  
    // read responses
    int j = 2;                      // start by displaying the second option when a key is pressed because the 
                                    //  first option has already been displayed on the initial setup screen above
    while((i = GetKeystroke(10000)) != SETUP_EXIT_KEY){
      switch(i)
      {
        case SETUP_TOGGLE_KEY:
          lcd.setCursor (0,0);
          switch(j)
          {
            // show setup options, the option displayed will be accepted
            // when the player exits setup mode, no need to save
            // use scale factor to moderate the difficulty
            case 1:
              lcd.print(F("Level       Beginner"));
              iLevelScaleFactor = 1; 
              break;
              
            case 2:
              lcd.print(F("Level Intermediate 1")); 
              iLevelScaleFactor = 2; 
              break;
    
            case 3:
              lcd.print(F("Level Intermediate 2")); 
              iLevelScaleFactor = 3; 
              break;
          } // end of display options switch
          j++;               // point to next choice      
          if(j==4) j=1;      // limit choices from 1 to 3 inclusive      
          break;
        default:
          break;
      } // end of switch on setup keystroke
    } // end get keystroke
  lcd.clear();      
  }
  
  // welcome message
  lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Hi Steve. I'm Simone")); 
  lcd.setCursor (0,1);        
  lcd.print(F("Just follow me...   "));           
  lcd.setCursor (0,3);        
  lcd.print(F("Hit any key to start"));
  lcd.on();
  WaitForAnyKey();
  lcd.clear();        
}

void loop() {
  // game has 2 primary modes, processed in this order:  
  //      1. display the challenge (a series of lights) and 
  //      2. read the player's response. 
  // the boolean flag below controls which path to take.  
  if (bDisplayNextChallenge){
    // preliminary checks...
    // check for max plays, if reached then reset game
    if(iTotalRounds > MAX_ROUNDS) swReset();

    // proceed with game setup    
    // Setup and display next challenge sequence, prepare for player responses
    // display statistics on LCD screen
    DisplayStatistics();   

    //check performance and set difficulty level based on data from previous rounds
    if (iLevelRoundNumber > 0) SetDifficultyLevel();
    
    iLevelRoundNumber++;                          // increment round number, this is reset when moving between levels
    iTotalRounds++;                               // increment total round counter, this is never reset
    
    // implement special behavior
    if((iTotalRounds % SPECIAL_GREETING1_TIME)== 0){
      myDFPlayer.play(SOUND_DOG);
      delay(2000);
    }
    
    delay(1500);                                  // give user time to focus on new round    
    SetupNextChallenge();                         // establish array of next moves and display challenge sequence
    
    bDisplayNextChallenge = false;                // challenge has been displayed, next scan will skip this section and wait for responses
    
    // prepare to receive responses
    iResponseIndex = 0;                           // initialize index which points to array of player's responses 
    // initialize player response matrix for upcoming round
    for(int i = 0; i<NUM_BUTTONS; i++){
      Responses[i].iReading = LOW;                // current reading is off
      Responses[i].iPrevReading = LOW;            // previous response is off
      Responses[i].PositiveTransitionTime = 0;        
      Responses[i].PreviousMillis = 0;         
      Responses[i].bButtonState = false;          // all buttons off
    }

    // initialize flags for upcoming round
    bRoundSuccess = false;
    bTimeout = false;
    bRoundFailure = false;

    // increment number of plays in level  this is reset when moving up or down a level
    iPlaysInLevel++;

    // get round start time
    roundStartTime = millis();
  }
  else
  {
  // handle player responses following display of challenge
  for(int j = 0; j<NUM_BUTTONS; j++){
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
          iResponses[iResponseIndex] = ledPins[j];          // record activation of button 
          if(challengeSequence[iResponseIndex] == 
                                            ledPins[j]){    // Requested button matches button played
            PiezoControl(PIEZO_CORRECT);                    // provide audio affirmation that correct button was pushed
            digitalWrite(ledPins[j], HIGH);                 // light LED corresponding to button that was pressed
          }
          else{
            bRoundFailure = true;                           // signal failure of round          
            iLevelErrorCount++;                             // increment count of errors in this level
            iTotalErrors++;                                 // increment count of total errors
            iConsecutiveLosses++;                           // track consecutive losses for level progression 
            iConsecutiveWins = 0;                           // reset consecutive wins 
            bDisplayNextChallenge = true;                   // player fails this round, display the next challenge
            myDFPlayer.play(SOUND_BUZZER);          
            delay(800);
            }
          iResponseIndex++;                                 // prepare to store next response
          }
        }
      }
    else if ((Responses[j].iReading == LOW) && (Responses[j].iPrevReading == HIGH)){
      Responses[j].bButtonState = false;                    // player has released the button, reset to allow it to be registered again
      digitalWrite(ledPins[j], LOW);                        // extinguish LED
      // check for end of sequence
      if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG){
        // reaching this point means all responses were correct
        bDisplayNextChallenge = true;                       // exit responses and display new challenge
        bRoundSuccess = true;
        iLevelSuccessCount++;
        iTotalSuccess++;
        iConsecutiveWins++;                                 // track consecutive wins for level progression 
        iConsecutiveLosses = 0;                             // reset consecutive losses
        myDFPlayer.play(SOUND_BELL);
        }
      }
      Responses[j].iPrevReading = Responses[j].iReading;    // save current reading as previous
    } // process next button


    // Done with current scan of buttons, check for timeout only if error or success was not encountered
    if((millis() > roundStartTime + iResponseDeadline) && (bDisplayNextChallenge == false)){
      bTimeout = true;
      iLevelTimeoutCount++;                     // increment count of timmeouts within the current level
      iTotalTimeout++;                          // increment total count of timeouts
      iConsecutiveLosses++;                     // timeout counts as a loss - track consecutive losses for level progression 
      iConsecutiveWins = 0;                     // zero out consecutive wins      
      bDisplayNextChallenge = true;             // signify time to display next challenge
      myDFPlayer.play(SOUND_BUZZER);            // play buzzer sound   
      lcd.setCursor (0,3);                      // display message
      lcd.print(F("Timeout  hit any key"));     // row 4 reserved for messages        
      WaitForAnyKey();                          // wait for keystroke
      lcd.setCursor (0,3);                      // clear message line
      lcd.print(F("                    "));     
    }
  } // done processing responses
}

/*
 * creates audible click when button is pushed
 * may be used for other signaling
 */
void PiezoControl(int iMode)
{
  switch(iMode)
  {
    case PIEZO_CORRECT:                       // brief click sound for positive feedback of keypress
      digitalWrite(alertPin, HIGH);
      delay(2);
      digitalWrite(alertPin, LOW);
      break;
      
    case PIEZO_YOUR_MOVE:                     // cricket chirp signifying "your move"
      digitalWrite(alertPin, HIGH);
      delay(4);
      digitalWrite(alertPin, LOW);
      delay(50);
      digitalWrite(alertPin, HIGH);
      delay(4);
      digitalWrite(alertPin, LOW);      
      break;  
         
    default:
      break;
  }
}

void SetDifficultyLevel(void){
  // apply a simple algorithm to automatic level progression / regression.
  // if x consecutive successes imcrement level until at upper limit
  // if y consecutive failures decrement level until at lower limit
    if(iPlaysInLevel > MIN_PLAYS_IN_LEVEL){
      if(iConsecutiveLosses > LVL_DECREMENT_THRESHOLD){       // decrease difficulty level if conditions warrant
        if(iDifficultyLevel > 1){                             // decrement only if we are not already at level 1
          iDifficultyLevel--;                                 // decrease level
          iConsecutiveLosses = 0;                             // clear consecutive loss counter
          Serial.print("Decreasing difficulty to: ");         // display message to serial monitor
          Serial.println(iDifficultyLevel);
          myDFPlayer.play(SOUND_ALARM);                       // play corresponding sound
          iPlaysInLevel = 0;                                  // reset plays in level counter
          ResetAndDisplayMsg("Going down a level ");          // reset counts and display message
        }
      }
      else if(iConsecutiveWins > LVL_INCREMENT_THRESHOLD){
          if(iDifficultyLevel < 5){                           // increment level only if we are not at maximum level
          iDifficultyLevel++;                                 // increment difficulty level
          iConsecutiveWins = 0;                               // zero out consecutive win counter
          Serial.print("Increasing difficulty to: ");         // display message to serial monitor
          Serial.println(iDifficultyLevel);
          myDFPlayer.play(SOUND_APPLAUSE);                    // play corresponding sound
          iPlaysInLevel = 0;                                  // reset plays in level counter
          ResetAndDisplayMsg("Moving up a level   ");         // reset counts and display message
        }
      }
    }
}

/*********************************************************************
 * setup next challenge 
 * Set next move based on Difficulty level
 * set number of elements, sequence of elements, display speed and deadline.
 *********************************************************************/
void SetupNextChallenge(void)
{
  int i;
  randomSeed(millis());                         // randomize     
  // setup challenge sequence
  // initialize challenge sequence matrix
  for(i = 0; i < MAX_CHALLENGE_LENGTH + 1; i++) challengeSequence[i] = END_OF_CHALLENGE_FLAG; 
  switch(iDifficultyLevel)
  {
  case 1:
      // set parameters
      iResponseDeadline = 25000 / iLevelScaleFactor;   
      iDisplayDelay = 750 / iLevelScaleFactor;
      // establish challenge sequence
      for(i = 0; i < random(2,4); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_BUTTONS)];     
      }
      break;
  case 2:
      // set parameters
      iResponseDeadline = 23000 / iLevelScaleFactor;   
      iDisplayDelay = 550 / iLevelScaleFactor;
      // establish challenge sequence
      for(i = 0; i < random(3,5); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_BUTTONS)];     
      }
      break;

  case 3:
      // set parameters
      iResponseDeadline = 21000 / iLevelScaleFactor;   
      iDisplayDelay = 475 / iLevelScaleFactor;
      // establish challenge sequence
      for(i = 0; i < random(3,6); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_BUTTONS)];     
      }
      break;  

  case 4:
      // set parameters
      iResponseDeadline = 19000 / iLevelScaleFactor;   
      iDisplayDelay = 325 / iLevelScaleFactor;
      // establish challenge sequence
      for(i = 0; i < random(4,7); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_BUTTONS)];     
      }
      break;      

  case 5:
      // set parameters
      iResponseDeadline =  17000 / iLevelScaleFactor;   
      iDisplayDelay = 300 / iLevelScaleFactor;
      // establish challenge sequence
      for(i = 0; i < random(5,7); i++) // random upper limit is number specified minus 1
      {
        challengeSequence[i] = ledPins[random(0,NUM_BUTTONS)];     
      }
      break;      
  default:
    break;
  }
      
    // display sequence,  i will be the index of the last element +1, use
    // it to contorl display of challenge elements
    DisplayChallenge(challengeSequence, i, iDisplayDelay);   
}

/*
 * DisplayChallenge - displays the challenge to the user
 * by sequencing a number of lights on the front panel.
 * avoid latency by NOT invoking the delay after the last element
 */
void DisplayChallenge(int challengeElements[], int iNumElements, int iDuration)
{
  
  for(int i = 0; i<iNumElements; i++){
    digitalWrite(challengeElements[i], HIGH);
    delay(iDuration);
    digitalWrite(challengeElements[i], LOW);
    // don't delay after the last element
    if(i < (iNumElements - 1)) delay(iDuration * 1.5);   
  }
  //sonic alert for "Your Move"
  PiezoControl(PIEZO_YOUR_MOVE);               
}

/*********************************************************************
 * Reset counters and display message. Used when changing levels
 * 
 *********************************************************************/
void ResetAndDisplayMsg(char* msg)
{
  iLevelSuccessCount = 0;                 // reset level-specific counters
  iLevelErrorCount = 0;
  iLevelTimeoutCount = 0;
  iLevelRoundNumber = 0;
  iPlaysInLevel = 0;                      // reset plays in level counter
  lcd.setCursor (0,3);                    //row 4 reserved for messages      
  lcd.print(msg);
  WaitForAnyKey();                  
  lcd.setCursor (0,3);                    //row 4 reserved for messages      
  lcd.print(F("                    "));  
  lcd.clear();                            // clear LCD and redisplay stats since new counts will occupy  
                                          // less space.  this approach prevents having to clear the
                                          // display with every update
  // display overall stats
  DisplayGlobalStats();
  WaitForAnyKey();
  lcd.clear();
  
  // display stats for previous level
  DisplayStatistics();                  // needs to be last here wiht no screen clearing
                                        // because the next round depends on the screen being
                                        // populated here
}


/*********************************************************************
 * display statistics for each round
 * 
 *********************************************************************/
void DisplayStatistics(void)
{
    lcd.setCursor (0,0);        
    lcd.print(F("Level "));
    lcd.print(iDifficultyLevel);
    lcd.print(F("   Round "));
    if(iLevelRoundNumber > 0) lcd.print(iLevelRoundNumber);
    lcd.setCursor (0,1);        
    lcd.print("Won    Lost  Timeout"); 
    lcd.setCursor (2,2);
    lcd.print(iLevelSuccessCount);
    lcd.setCursor (9,2);
    lcd.print(iLevelErrorCount);
    lcd.setCursor (16,2);
    lcd.print(iLevelTimeoutCount);    
}

/*
 * display cumulative statistics
 */ 
void DisplayGlobalStats(void){

  float fErrorPct   = 0.0;
  float fSuccessPct = 0.0;
  float fTimeoutPct = 0.0;
  int iErrorPct = 0;
  int iSuccessPct = 0;
  int iTimeoutPct = 0;

  // get integer percentages for display - probably a more elegant way to do this...
  fErrorPct   = iTotalErrors / (float)iTotalRounds;
  fSuccessPct = iTotalSuccess / (float)iTotalRounds;
  fTimeoutPct = iTotalTimeout / (float)iTotalRounds;
  iErrorPct = (int)(fErrorPct * 100);
  iSuccessPct = (int)(fSuccessPct * 100);
  iTimeoutPct = (int)(fTimeoutPct * 100);
  
  lcd.clear();          
  lcd.home (); 
  lcd.setCursor (0,0);        
  lcd.print(F("Won"));
  lcd.setCursor (9,0);    
  lcd.print(iTotalSuccess);
  lcd.setCursor (14,0);    
  lcd.print(iSuccessPct);
  lcd.print("%");

  lcd.setCursor (0,1);        
  lcd.print(F("Lost"));
  lcd.setCursor (9,1);    
  lcd.print(iTotalErrors);
  lcd.setCursor (14,1);    
  lcd.print(iErrorPct);
  lcd.print("%"); 

  lcd.setCursor (0,2);        
  lcd.print(F("Timeout"));
  lcd.setCursor (9,2);    
  lcd.print(iTotalTimeout);
  lcd.setCursor (14,2);    
  lcd.print(iTimeoutPct);
  lcd.print("%");   

  lcd.setCursor (0,3);        
  lcd.print(F("any key to continue "));
  
}

/*
 * waits for keystroke.   
 * caution, this blocks further processing
 *  
 */
void WaitForAnyKey(void){
  while(GetKeystroke(1000) == -1);
}


/*
 * gets keystroke from player if provided within wait time
 * returns:  index of key number if successful, -1 if timeout
 * do not return until key has been released to prevent key from 
 * being consumed by a downstream process
 */
int GetKeystroke(unsigned long ulWaitTime){
  unsigned long PositiveTransitionTime = 0;
  unsigned long ulDebounceDelay = 25;
  unsigned long ulStart = 0;
  bool bButtonState = false;
  int iRetnValue = 0;
  int j = 0;
  int iReading[NUM_BUTTONS];
  int iPrevReading[NUM_BUTTONS];

  for(j = 0; j< NUM_BUTTONS; j++){
    iReading[j] = 0;
    iPrevReading[j] = 0;
  }
  
  ulStart = millis();
  while(((ulStart + ulWaitTime) > millis()) && bButtonState == false)
  {
    for(j = 0; j<NUM_BUTTONS; j++){
      iReading[j] = digitalRead(buttonPins[j]);
      // button transition to high - get timestamp
      if ((iReading[j] == HIGH) && (iPrevReading[j] == LOW)) {
        // get time of newly detected high state
        PositiveTransitionTime = millis();
      }
      // button transitioned to low - was it on long enough to count?
      if ((iReading[j] == LOW) && (iPrevReading[j] == HIGH)) {
        if(millis() > PositiveTransitionTime + ulDebounceDelay){
           bButtonState = true;             // signify exit for outer loop
           break;                           // button has been identified and released, exit inner loop
        }
      }
      iPrevReading[j] = iReading[j];      // store previous reading
    }  // end scan of buttons
  } // end check for timeout

  // setup return value
  if(bButtonState == true){
    iRetnValue = j;                       // return index of search showing which button was pressed
  }
  else{
    iRetnValue = -1;                      // signify no button pressed within designated time window
  }
  return(iRetnValue);
}


/*
 * Reset game
 */
void swReset() // Restarts program from beginning but does not reset the peripherals and registers
{
  lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Congratulations     ")); 
  lcd.setCursor (0,1);        
  lcd.print(F("max rounds reached  "));           
  lcd.setCursor (0,3);        
  lcd.print(F("any key to reset    "));
  lcd.on();
  WaitForAnyKey();
  lcd.clear();
  asm volatile ("  jmp 0");         // boom boom... out go the lights
} 
