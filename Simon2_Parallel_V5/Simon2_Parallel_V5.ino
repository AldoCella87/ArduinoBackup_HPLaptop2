
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
 * 
 * prevent play from proceediing if user is holding down buttons
 * add calculation of speed of response (average, short term average)
 * add real time clock and player initials, high score calculation (like old arcade game)
 * 
 * 2/13/2020 - noticed one defect where the blue light remained on after play and was on while the next sequence was displayed. was
 * able to repeat the fault by holding all buttons down.  THe yellow light remained on until it was cycled during play.  .. investigate and fix
 * quick fix is to ensure all lights are off at start of challenge display
 * 
 * show cumulative response time
 * prevent challenge from duplicating a button (e.g. no green, green, ...)
 * 
 * have follow the leader mode for x duration.  player immediately presses indicated button until an error threshold is reached then speed declines subtly to 
 * maintain an error rate of x%
 * 
 * have user response match not only the sequence but the duration of on time and spacing between challenge elements.
 * allow multiple lights to be presented at the same time:  (green&yellow), red, blue, (red&red), etc.
 * 
 * repeat failed challenge until a max retries is reached before proceeding to a new challenge.
 * 
 * when waiting for "press any key to continue" light (or flash) one or more button lights.
 * when a wrong selection is made show the correct selection, perhaps with triple flash.   lead player through to correct solution, show correct
 * selections with single flash and next correct (missed) challenge element with triple flash.
 * or, when a player misses the game leads the player step by step through the solution, perhaps displaying it on the screen
 * or when player misses screen shows correct seuence, or correct sequence followed by player's response on the screen
 * exmaple:   Line1:   R, Y, Y, G, B, O
 *            Line2:   R, Y, Y, R <- flashing cursor
 * when player makes an error have screen automatically show the challenge   
 * 
 *  keep statistic of number of plays (and time?) spent at each level
 *  create overall score based weighted average of time spent at higher levels
 *  
 * replace button lights with RGB LED for many color options
 * Direct responses to be provided by a single hand.  Create a version with 10 buttons, arranged in convenient
 * orientaiton for 10 fingered response.  Keep stats on each finger.  Change therapy mode to exercise each finger in turn
 * thus removing randomness from trials.  track average resposne speed, error rate.  Challenge could be number of 
 * keystrokes, rhythm of keystrokes with challenge containing a variable time between keystrokes and response 
 * being measured to include the time betweek keystrokes.  connect with application on laptop or via bluetooth to phone 
 * to capture and record patient performance statistics over time.  add pressure sensing buttons to measure strength
 * of response
 * 
 * play musical note for each key during challenge and in response (optional music mode comes on under certain conditions)
 * play familiar sound for each key (car door, duck, doorbell, etc.)
 * 
 * play musical intervals to copy
 * physically deliver amazon gift card via servo motor in popup mode like a toaster.
 * solution might be persistence 
 * 
 * another challenge could be to complete the sequence (duck, ducks, dog, dogs, cat, ...?)
 * or to complete an integer sequence? (too few buttons, too confusing to implement)
 * implement multi-button detection. 
 * 
 * 
 * Add multi-player capability with each person (named) associated with a single button,  keep
 * stats on individual performance.  have multiple player pods connected wirelessly
 * 
 * Add haptic feedback,  show errors vs correct response (perhaps this is a configurable mode)
 * add light level detection for display brightness or other latent feature
 * all bluetooth interface to show game statistics
 * 
 *  have a mode where challenge is repeated when player fails to enter it correctly
 * 
 * 
 */

 // 2/29/2020  - test version - add simultaneous button press in challenge and response.
 
// 3/25/2020 - v5 - Add simultaneous challenge elements


/**************   Description   ***********
 *    
 *    the game board has one 4 x 20 LCD display and a group of uniquely colored lighted pubhbuttons.
 *    The game presents the player with a series of challenges which the player
 *    must accurately replicate.  
 *    
 *    the CHALLENGE
 *    Each CHALLENGE is a series of n or more lights presented by the game to the user in sequence
 *    at a certain configurable speed.  For example, the sequence red, yellow, blue, red might be presented.  
 *    Once the challenge is presented the game pauses for the user's response.
 *    A successful response is one in which the user presses the corresponding pushbuttons in the same 
 *    order as the challenge within a certain configurable time limit.
 *    
 *    Representing the challenge as a matrix with "0" representing light off and "1" representing 
 *    light on for each of the Red, Orange, Yellow, Green and Blue lights for the example outlined above we have:
 *    
 *    Challenge x, number of element groups is 5:
 *    R O Y G B
 *    1 0 0 0 0    <- Challenge x, Element Group 1
 *    0 0 1 0 0    <- Challenge x, Element Group 2
 *    0 0 0 0 1    <- Challenge x, Element Group 3
 *    1 0 0 0 0    <- Challenge x, Element Group 4
 *    
 *    Difficulty may be increased by simultaneously lighting more than one button as in the following example. 
 *    Present (red and yellow together), (orange, yellow and blue together), and (red and blue together). The challenge 
 *    will be presented at a configurable speed.  Here is the challenge in matrix form.
 *    
 *    Challenge y, number of element groups is 3:
 *    R O Y G B
 *    1 0 1 0 0    <- Challenge y, Element Group 1
 *    0 1 1 0 1    <- Challenge y, Element Group 2
 *    1 0 0 0 1    <- Challenge y, Element Group 3
 *    
 *    As before, a successful response is one in which the player pushes the buttons in the same order as was presented 
 *    by the challenge.  A failure in responding to any challenge group results in failure of the entire challenge.
 *    
 *    The degree of difficulty is controlled by the speed at which the element groups are presented, the lentgh of the 
 *    timeout duration, the number of element groups contained in the challenge and other factors such as the memorability
 *    of the challenge pattern.  Because the buttons / lights are arranged in a fixed spatial pattern on the game board there 
 *    will be some patterns which are more easily remembered.  Another factor affecting difficulty is the amount of time required
 *    after a button is pressed to recognize a valid entry.  This combined with parameters which define when a simultaneous button 
 *    press may be recognized also have an effect on the users's experience.
 *    
 *    The term CHALLENGE designates the data associated with the problem presented to the player by the game.  The challenge
 *    matrix holds the challenges. The challnge index points to an individal challenge group.
 *    
 *    The RESPONSE
 *    
 *    Each RESPONSE is a series of n or more button presses presented by the player to the game in response to a given challenge.
 *    For example the response sequence red, yellow, blue, red might be provided by the user in response to a challenge.  
 *    
 *    Representing the response as a matrix with "1" representing a button press then the following matrix would apply.  Note that
 *    since the response is checked against the challenge on an element group by element group basis there is no need to record any responses beyond 
 *    the current element group response because if it fails then the challenge fails and if it passes the game simply records a response for the 
 *    next group in question.  
 *    
 *    Response 1 for challenge x (red button has been pressed):
 *    R O Y G B
 *    1 0 0 0 0     
 *    
 *    In the case of multiple simultaneous challenge elements the response matrix might be:
 *    
 *    Response 1 for challenge y (red and yellow buttons pressed):
 *    R O Y G B
 *    1 0 1 0 0     
 *    
 *    In addition to tracking the button state the game must also keep track of other response-related data in order to 
 *    facilitate switch debounce processing.  
 *    
 *    The term RESPONSE designates the data associated with the actions presented to the game by the player.  The response 
 *    matrix holds the responses.  The response index points to the specific response group.  
 *    
 *    The term ELEMENT represents a single component of an element group.
 *    
 */




// following supports df mini
// dfmini connections (spkr, gnd, spkr, nc, nc, rcv to analog 0, tx w/1K resistor to analog 1, vcc)
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(A0, A1); // RX, TX+
DFRobotDFPlayerMini myDFPlayer;

// MP3 file numbers for DF player
#define SOUND_BUZZER      3
#define SOUND_DOG         1
#define SOUND_APPLAUSE    2
#define SOUND_BELL        4
#define SOUND_ALARM       14    
// end df mini

// setup LCD display
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 

#define TEST_MODE
#ifdef TEST_MODE
    // test parameters here
    #define MAX_ROUNDS  999                 // force software reset after this many rounds
    #define LVL_DECREMENT_THRESHOLD 2       // number of consecutive failures beyond which level will revert
    #define LVL_INCREMENT_THRESHOLD 3       // number of consecutive wins beyond which level will increase
    #define MIN_PLAYS_IN_LEVEL      3       // minimum number of rounds required to be spent in each level
    #define MAX_ELEMENT_GROUPS      3       // maximum number of element groups in a challenge  
    #define SOUND_VOLUME            8             
#else
    // normal parameters here
    #define MAX_ROUNDS  999                 // force software reset after this many rounds
    #define LVL_DECREMENT_THRESHOLD 2       // number of consecutive failures beyond which level will revert
    #define LVL_INCREMENT_THRESHOLD 4      // number of consecutive wins beyond which level will increase
    #define MIN_PLAYS_IN_LEVEL      4      // minimum number of rounds required to be spent in each level
    #define MAX_ELEMENT_GROUPS      10     // maximum number of element groups in a challenge 
    #define SOUND_VOLUME            24

#endif

// non test mode parameters
#define PIEZO_CORRECT           0           // make click as audio feedback for key press
#define PIEZO_YOUR_MOVE         1           // make click as audio feedback for key press
#define NUM_BUTTONS             5           // number of player buttons
#define SETUP_EXIT_KEY          3           // key used to save displayed options and exit setup
#define SETUP_TOGGLE_KEY        0           // key used to toggle between options
#define SETUP_ENTRY_KEY         0           // key used to enter setup mode, press this key during power up to enter setup



int iMachineState = 0;

// constants for state machine
const short DisplayChallengeState = 0;
const short ProcessResponseState = 1;
const short ElementGroupSuccessState = 2;       //state where one element group of a challenge has been successfuly entered
const short ChallengeSuccessState = 3;   //state wher the entire challenge has been successfuly entered (successful response for all element groups)
const short ResponseFailureState = 4;
const short ResponseTimeoutState = 5;







  
int iPlaysInLevel = 0;                                          // number of plays which have been spent in the current level, reset at each change in level
int iConsecutiveWins = 0;                                       //governs level progression
int iConsecutiveLosses = 0;                                     //governs level progression

// caution, initial values must be set manualy to match NUM_BUTTONS
// using 10K pulldown and normally open switch to + on each input
int buttonPins[NUM_BUTTONS] = {2, 3, 4, 5, 6};                  //R, O, Y, G, B
const int ledPins[NUM_BUTTONS] = {9, 10, 11, 12, 13};           //R, O, Y, G, B

const int alertPin = 7;                                         // sonalert for audible 'click' when button is pressed and other optional uses

const unsigned int debounceDelay = 200;                         // time in MS which button must be pressed to count as a press


// response structure holds data for each response 
struct ResponseStruct{    
  int iReading;
  int iPrevReading;
  unsigned long PositiveTransitionTime;
  bool bButtonState;
};
ResponseStruct Responses[NUM_BUTTONS];


unsigned long iResponseDeadline = 30000;


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


int NewChallengeSequence[MAX_ELEMENT_GROUPS][NUM_BUTTONS];  // array holding challenges, allows multi-press of buttons   - valid values are 1,0





int NewCreateChallenge(void)
{
  bool bIsValidChallenge = false;     // protect against randomly creating a challenge containing all zeroes 
  randomSeed((int)millis());     
  int iCountOfGroups = (int)random(2,(MAX_ELEMENT_GROUPS+1)); // random upper limit must be 1 greater than maximum desired  
  // spin through challenge elements.  Each element contains NUM_BUTTONS of individual button assignments
  for (int i = 0; i< iCountOfGroups; i++){
    bIsValidChallenge = false;                    // A valid sequence must contain at least one "1"
    while(bIsValidChallenge == false){        
      // for each button assign a value of one or zero to control whether that button is displayed as "on" or "off"
      for (int j = 0; j< NUM_BUTTONS; j++){
        int k = (int)random(0,2);
        NewChallengeSequence[i][j] = k;           // assign randomly selected on/off value to button
        // if any button value in the element group is a "1" then we have a valid sequence
        if(k == 1){
          bIsValidChallenge = true;               // signify that a valid sequence exists
        }
       }// end assign values to buttons
      }// end while bValidChallenge is false
    }// end spin through challenge elements
  return iCountOfGroups;
}

void DisplayChallenge( int iNumGroups, int iDuration)
{
  Serial.println("New challenge in DisplayChallenge");
  for(int i = 0; i<iNumGroups; i++){
    // display lights according to challenge settings
    for(int j = 0; j< NUM_BUTTONS; j++){
        digitalWrite(ledPins[j],NewChallengeSequence[i][j]);
        Serial.print(NewChallengeSequence[i][j]);    
        if(j < NUM_BUTTONS - 1) Serial.print(", ");
    }  
    Serial.println();  
    // delay specified duration
    delay(iDuration);
    // set all buttons low
    for(int j = 0; j< NUM_BUTTONS; j++){
         digitalWrite(ledPins[j], LOW);
    }
    // don't delay after the last element
    if(i < (iNumGroups - 1)) delay(iDuration * 1.5);   
  }
  //sonic alert for "Your Move"
  PiezoControl(PIEZO_YOUR_MOVE);               
}






/*
 * checks return code, returns true if any button is pressed
 * otherwise returns false
 */
bool bCheckButtons(void){
bool bRetnCode = false;
  for (int i = 0; i < NUM_BUTTONS; i++){
    if(digitalRead(buttonPins[i]) == HIGH){
      bRetnCode = true;
      break;
    }
  }
  return(bRetnCode);
}



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
  lcd.print(F("Hello,    I'm Simone")); 
  lcd.setCursor (0,1);        
  lcd.print(F("Just follow me...   "));           
  lcd.setCursor (0,3);        
  lcd.print(F("Hit any key to start"));
  lcd.on();
  WaitForAnyKey();
  lcd.clear();    


  #ifdef xxTEST_MODE
  for(i=1;i<23;i++){
      myDFPlayer.play(i);
      delay(2000);      
  }
  #endif


  iMachineState = DisplayChallengeState;               //setup to formulate and display a challenge
  
}

void loop() {

static int iResponseIndex = 0;                        // the element group number of the response
static int iElementGroupIndex = 0;                    // the element group number of the challenge
static int iNumOfGroups = 0;                          // number of element groups in the current challenge
static int iElementIndex = 0;                         // spins through the buttons / LEDs once per scan


 switch(iMachineState)
    {      
      case DisplayChallengeState:  
        Serial.println();
        Serial.println("Display Next Challenge");
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
        iPlaysInLevel++;                              // increment number of plays in level.  This is reset when moving up or down a level
   
        iNumOfGroups = NewCreateChallenge();          // populate challenge matrix with next challenge
        Serial.print("Number of element groups:  ");  // display number of element groups
        Serial.println(iNumOfGroups);   
        DisplayChallenge( iNumOfGroups, 500);         // display challenge to player pausing as specified between element groups
        
        // prepare to receive responses
        iResponseIndex = 0;                           // initialize index which points to array of player's responses 

        iElementGroupIndex = 0;                       // point to first element group in the challenge
        iMachineState = ProcessResponseState;         // change state to process responses next time through the loop
        break;

      case ProcessResponseState:
        iMachineState = GetResponses( iElementGroupIndex, iNumOfGroups);                                // process user input         
        break;

      case ElementGroupSuccessState:
        iMachineState = ProcessResponseState;          // continue to process user input on the existing challenge 
        iElementGroupIndex ++;          
        break;        

      case ChallengeSuccessState:
        iLevelSuccessCount++;
        iTotalSuccess++;
        iConsecutiveWins++;                                 // track consecutive wins for level progression
        // TODO:  move consecutive loss reset to where level is incremented 
        iConsecutiveLosses = 0;                             // reset consecutive losses
        myDFPlayer.play(SOUND_BELL);
        iMachineState = DisplayChallengeState;               //setup to formulate and display a challenge
        delay(1500);                                         // slight delay to allow player to refocus, make this configurable
        break;

      case ResponseFailureState:
        iLevelErrorCount++;                             // increment count of errors in this level
        iTotalErrors++;                                 // increment count of total errors
        iConsecutiveLosses++;                           // track consecutive losses for level progression 
        iConsecutiveWins = 0;                           // reset consecutive wins 
        myDFPlayer.play(SOUND_BUZZER);          
        delay(800);
        iMachineState = DisplayChallengeState;               //setup to formulate and display a challenge
        break;     

      case ResponseTimeoutState:
        iLevelTimeoutCount++;                     // increment count of timmeouts within the current level
        iTotalTimeout++;                          // increment total count of timeouts
        iConsecutiveLosses++;                     // timeout counts as a loss - track consecutive losses for level progression 
        iConsecutiveWins = 0;                     // zero out consecutive wins      
        myDFPlayer.play(SOUND_BUZZER);            // play sound   
        lcd.setCursor (0,3);                      // display message
        lcd.print(F("Timeout  hit any key"));     // row 4 reserved for messages        
        WaitForAnyKey();                          // wait for keystroke
        lcd.setCursor (0,3);                      // clear message line
        lcd.print(F("                    "));  
        iMachineState = DisplayChallengeState;               //setup to formulate and display a challenge
        break;
    }
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



 /*****
 * GetResponses processes multi-button pushes one response element at a time
 * parameters:   response element group and challenge element array
 * returns: condition code representing next processing state
 */
int GetResponses(int iElementGroup, int iNumGrps){


  byte RetnCode = ResponseTimeoutState;
  unsigned long ulRoundStartTime = millis();         // get initial time for timeout check
  bool bResponseReceived = false;               // indicates that at least one button has been pressed
  bool bAllButtonsReleased = false;             // is true when no buttons are pressed
  bool bTimeout = false;                        // flags timeout condition    

  // initialize response matrix
  for(int i = 0; i<NUM_BUTTONS; i++){
  Responses[i].iReading = 0;                 
  Responses[i].iPrevReading = 0;             
  Responses[i].PositiveTransitionTime = 0;     
  Responses[i].bButtonState = false;           
  }

  // get button state, exit while loop when at least one button has been pressed and all are off
  while(bAllButtonsReleased == false){
      // scan all buttons
      for(int j = 0; j<NUM_BUTTONS; j++){
        Responses[j].iReading = digitalRead(buttonPins[j]);       // get button j state
        digitalWrite(ledPins[j],Responses[j].iReading);           // set LED accordingly
        // if button was pressed for longer than threshold then register the response

        if ((Responses[j].iReading == 1) && (Responses[j].iPrevReading == 0)) {
        // get time of newly detected high state
        Responses[j].PositiveTransitionTime = millis();
        }
        else if ((Responses[j].iReading == 1) && (Responses[j].iPrevReading == 1)) {
        // button remains on; if it has been on long enough then register a response then do so
          if ((millis() - Responses[j].PositiveTransitionTime) > debounceDelay) {
             Responses[j].bButtonState = true;    
             bResponseReceived = true;                
          }
        }
        Responses[j].iPrevReading = Responses[j].iReading;            // save current reading as previous
    }


    //check if response has been received and all buttons have been released, 
    // if so then free to check responses against challenge
    if(bResponseReceived == true){
      bAllButtonsReleased = true;
      for (int i = 0; i < NUM_BUTTONS; i++){
        if(Responses[i].iReading == 1) bAllButtonsReleased = false;
      }
    }
    
    // check for timeout
    if((millis() > ulRoundStartTime + iResponseDeadline)){
      bTimeout = true;
      Serial.println("Timeout");
      break;
    }
  }
  

  Serial.println();

  // prepare return code based on conditions.  If timeout was encountered then indicate it.
  // if we have reached last element group in the challenge then signify success of the entire challenge
  // otherwise signify element group success

  if(bTimeout == true){
      RetnCode = ResponseTimeoutState;
  }
  else{    
    // if we have processed the last element group
    if(iElementGroup == (iNumGrps - 1)){
      // tentative success - to be set to failure below if failures are found
      RetnCode = ChallengeSuccessState;     // all element groups have successfuly been entered for this challenge     
    }
    // otherwise more element groups to process
    else{
      RetnCode = ElementGroupSuccessState;       // The current group has been successfuly entered, need to process response for next element group     
    }
  
    // check for failure.  An element group failure results in failure of the entire challenge
    Serial.println();
    Serial.print(" *** Element group: ");
    Serial.println(iElementGroup);
    for (int i = 0; i < NUM_BUTTONS; i++){
      Serial.print("Comparing challenge element: ");
      Serial.print(NewChallengeSequence [iElementGroup] [i]);
      Serial.print("     response element:  ");
      Serial.println(Responses[i].bButtonState);
      if(Responses[i].bButtonState != NewChallengeSequence[iElementGroup][i]){
        RetnCode = ResponseFailureState;
        Serial.println("   *** Error in response");
      }
    }
  }
  return RetnCode;
}



/*
 * Reset game
 */
void swReset(void) // Restarts program from beginning but does not reset the peripherals and registers
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
