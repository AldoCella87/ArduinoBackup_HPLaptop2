
// useless machine - a box with a single toggle switch on top.
// When the switch is turned on by the user the box lid opens and an actuator (perhaps a plastic
// hand or arm from a toy doll) emerges which turns the switch to the off position. The actuator returns to the
// home position and the lid closes.
// See: https://www.youtube.com/watch?v=4wWlrBFOdBo for an example.
// The code below assumes two servos however you may simply comment out the lid servo and corresponding delay statements if you go with one servo. 
// Adding two slashes at the start of a line will cause all commands following the slashes on a given line to be considered as a comment
// and thus will not be executed. 

// Here is the starting point code.  Simply open a fresh Arduino sketch, delete all existing template code and paste this entire file (comments and all) into
// the sketch.  Save it with a unique name such as "Useless_V1".  Feel free to save your new versions of this with a different name so you can easily go
// back to a previous version if desired.  Of course, this is just the code to run the motors and read the switch.  You will have to work out the mechanical aspects
// the working of the arm, the placement of the switch, powering the Arduino, the box opening mechanism, etc.  One approach just to get up and running would be to
// simply wire up the motors, switch and LED and test it 'out of the box'.  Each time you turn the switch to the on position the LED should come on and both servos
// should cycle only once.  This would show that the controller and motors are working.  Once you have accomplished this you can build the box, perhaps one
// made of foam core posterboard and duct tape would suffice as a proof of concept. 
// Have fun and Good Luck! 



/***********************************************************************************************
 * Useless machine program for Arduino
 * 2/19/17 - first pass
 * 2/20/17 - corrected comment for resistor value for LED, lengthened actuator distance and 
 * corresponding forward and home wait times.
 * 2/26/17 - added actuator values for my specific implementation (40 and 145)
 * 2/28/17 - further refinements,  added long wait and multiple checks for manual switch deactivation
 * 3/2/17 - added LCD display
 * 3/5/17 - added password mode
 * 3/12/17 - added "speak to me" mode
 * 3/13/17 - added unlock mode parameters and corresponding display, added magnetic sensor
 * 3/14/17 - added light and distance sensors
 * 3/16/17 - added custom random function to replace buggy random();
 * 4/8/17 - refined multi toggle behavior
 ***********************************************************************************************/
 
#include <Servo.h>              // this provides access to the arduino servo functions and is required in order for the code to work

//Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define address 0x1E //0011110b, I2C 7bit address of HMC5883 magnetic sensor

#define LIGHT_SENSE_PIN  0     // analog input for light sensor
#define ACT_PIN 10             // digital pin 10 goes to the control line of the actuator servo (white (sometimes orange) wire)
#define ALERT_PIN  4           // sonic alert 

// lid and actuator servo positions - adjust these as needed     
// A typical servo motor will only rotate 180 degrees.  When given a command it will rotate to that position and stay there.
// Valid servo commands are between 0 and 180.   

#define ACTUATOR_HOME_POS 140   // the servo starts at this position and returns to this position after turning the switch off
#define ACTUATOR_FWD_POS  40    // the servo goes to this position to push the switch to the off position
#define FLAP_POS          132   // the servo goes to this position to push the switch to the off position

                                // servo objects - no need to change these
//Servo lidServo;               // create the lid servo object
Servo actServo;                 // create the actuator servo object

                                // switch and LED pins - no need to change these
#define SWITCH_PIN      2       // assign switch input to digital pin 2.  Switch goes between digital pin 2 and 5V. 
                                // Add a 4700 ohm resistor (yellow, violet, black, brown brown) between this pin and ground to guarantee that the pin
                                // remains in a low state when the switch is opened.
#define LED_PIN         3       // assign LED to digital pin 3.  Anode of LED goes to pin 3. Must use a resistor in series with the LED
                                // Cathode of LED goes to through a 5100 ohm resistor (green, brown, black, brown, brown)to ground.
                                // use a lower value (minimum 200 ohms) for a brighter light
#define US_ECHO_PIN     7       // distance output of ultrasonic sensor
#define US_TRIGGER_PIN  6       // trigger pin for ultrasonic sensor

#define LOCK_STATUS_UNLOCKED  1
#define LOCK_STATUS_LOCKED    0

int bSwitchState = 0;           // variable which holds the current switch reading
int i = 0;                      // generic loop control
int j = 0;
int iMyScore, iYourScore = 0;

long liresponseTime = 0;        // determines response duration

int  itoggleCount = 0;
long linow = 0;                  // holds current time in ms, used for multiple switch toggle count
byte iprevSwitchState = 0;

int itrialCount = 0;          // holds perseverance value, incremented as user progresses
int iMagOrient = 0;             // holds magnetic compass heading
int iLightSense = 0;            // holds ambient light sensor reading
int iDistance = 0;              // holds ultrasonic distance sensor
 
// setup LCD display
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS1,2,1,0,4,5,6,7);  

// define behavior parameters

// add light, temperature, humidity, background noise, bluetooth, wifi - some provide a more benevelont behavior
// add "No Cheating" message when switch is turned off manually
// add riddles
// "Are you still there"?
// auto shut off with insult
// if switch is off unit should go back to "your move" without any other messages
// other behaviors:  box tilt, front panel lift, box scoot, wav file ("I'm sorry Dave..."), LCD display "I win again"
// enter Easter Egg mode with x rapid switch transitions,  (perhaps x is a range or it may be displayed as part of the startup screen).  
//   when in easter egg mode allow switch to enter password (cycle through A-Z plus numbers?), pre-display or not the number of digits required for password
//   accept multiple or partial passwords?  use Amazon gift card a prize.
// use photo detector to modulate the personality.. "I like a dark room"
// add reed switch 
// add magnetic orientation
// add accelerometer
// add tilt switch alarm (real or imaginary)
// add password transmission via morse code when in password mode.  Send to AM or FM radio, send via buzzer or LED
// send IR shutoff codes to nearby TVs
// have light level, distance or mag reading control probabilities
// to make more difficult remove names of the measures or change names to meaningless descriptors e.g. "presence"
// once a lock has been unlocked do not display the measurement
// have a higher difficulty mode which requires all locks to be satisfied in a single attempt as opposed to sequentially
// show locks at a designated point in play (e.g. at attempts 50, 100, 150, etc.) for easy mode
// reset timeout timer while in loop counting multiple switch operations.  currently (4/3/17) timer times out if too many multi-switch attempts are made
// use mag sensor reading divisible by x rather than greater than X to allow for any range of values.
// only issue "are you there" message if light level is above or below a certain threshold
// add score reset to zero and score penalty based on capricious infraction
// add tilt alarm, accelerometer and tap detection
// more advanced (more annoying) features should come in after initial play (slowly reveal true personality)


  
#define  TEST_MODE
#ifdef TEST_MODE

  #define PERSEVERANCE_THRESHOLD  1                 // miinimum number of rounds required to obtain perseverance unlock  
  #define SCORE_THRESHOLD  1                        // minimum score required to obtain score unlock  
  #define LIGHT_THRESHOLD 30                        // maximum reading required to unlock
  #define MAG_THRESHOLD 0                           // minimum reading required to unlock
  #define DISTANCE_THRESHOLD 16                     // maximum reading required to unlock
  #define INACTIVITY_TIME   1200                    // seconds to wait before issuing inactivity taunt  
  #define LONG_DELAY_MIN    10                      // minimum delay in seconds
  #define LONG_DELAY_MAX    20                      // maximum delay in seconds
  #define FLAP_PROB         3
  #define DISPLAY_READINGS_MIN  50                  // minimun number of trials required before sensor readings are displayed
  #define ORIENTATION_PROB  3
  #define LIGHT_PROB        3
  #define DISTANCE_PROB     3
  #define PERSEVERANCE_PROB 3
  #define FLICKER_PROB      3
  #define PEEK_PROB         3
  #define MULTI_PEEK_PROB   3      
  #define LONG_WAIT_PROB    3
  #define SLOW_DEPLOY_PROB  3
  #define SCORE_BUMP_PROB   3
  #define YOU_WIN_PROB      3
  #define GO_AWAY_PROB      3
  #define SPEAK_TO_ME_PROB  3                       // simulated voice input processing
  #define SPEAK_TO_ME_MIN   25                      // minimum number of trials before speak to me is an option
  #define I_AM_SORI_PROB    3                       // "I am Sori ... (pause) "that you are losing"
  #define SCORE_RESET_PROB  0                       // "User Response Error Your score has been reset to zero"
  #define SCORE_PENALTY_PROB  3                     // "User Response Error Your score has been reset to zero"
  #define GIVEUP_PROB       3                       // probability of displaying wisecrack message regarding persevarance
  #define VOX_RESPONSE_DELAY  4500                  // ms to wait for voice response                

#else

  #define PERSEVERANCE_THRESHOLD  91                // miinimum number of rounds required to obtain perseverance unlock  
  #define SCORE_THRESHOLD  42                       // minimum score required to obtain score unlock  
  #define LIGHT_THRESHOLD 30                        // maximum reading required to unlock
  #define MAG_THRESHOLD 0                           // minimum reading required to unlock
  #define DISTANCE_THRESHOLD 16                     // maximum reading required to unlock
  #define INACTIVITY_TIME   1200                    // seconds to wait before issuing inactivity taunt  
  #define LONG_DELAY_MIN    300                     // minimum delay in seconds
  #define LONG_DELAY_MAX    1200                    // maximum delay in seconds
  #define FLAP_PROB         4
  #define DISPLAY_READINGS_MIN  50                  // minimun number of trials required before sensor readings are displayed
  #define ORIENTATION_PROB  1
  #define LIGHT_PROB        1
  #define DISTANCE_PROB     1
  #define PERSEVERANCE_PROB 1
  #define FLICKER_PROB      4
  #define PEEK_PROB         4
  #define MULTI_PEEK_PROB   4      
  #define LONG_WAIT_PROB    4
  #define SLOW_DEPLOY_PROB  4
  #define SCORE_BUMP_PROB   3
  #define YOU_WIN_PROB      1
  #define GO_AWAY_PROB      2
  #define SPEAK_TO_ME_PROB  5                       // simulated voice input processing
  #define SPEAK_TO_ME_MIN   25                      // minimum number of trials before speak to me is an option
  #define I_AM_SORI_PROB    2                       // "I am Sori ... (pause) "that you are losing"
  #define SCORE_RESET_PROB  0                       // "User Response Error Your score has been reset to zero"
  #define SCORE_Penalty_PROB  0                     // penalty for violation of rule 35
  #define GIVEUP_PROB       5                       // probability of displaying wisecrack message regarding persevarance
  #define VOX_RESPONSE_DELAY  4500                  // ms to wait for voice response                               
#endif


void setup() {
  // mag sensor setup
  Wire.begin();
  
  //Setup HMC5883 magnetic sensor
  Wire.beginTransmission(address);            //open communication with HMC5883
  Wire.write(0x02);                           //select mode register
  Wire.write(0x00);                           //continuous measurement mode
  Wire.endTransmission();
  // end mag sensor setup
  
  lcd.off(); 
  lcd.begin (16,2);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  
  actuatorHome();
  
  displayLCD(F("My name is Sori"),F("Let's play..."), 0,3500);
  displayLCD(F("Score 000 to 000"),F("your move...    "), 0,0);
  
  Serial.begin(9600);                 // start the serial output stream, this enables displaying informational messages on the monitor
  pinMode(SWITCH_PIN, INPUT);         // configure switch pin as an input.  Connect a resistor between this pin and ground to 
                                      // force it to a low state when switch is open
  pinMode(LED_PIN, OUTPUT);           // configure LED pin as an output, this is for an external LED indicator to accompany the switch
  pinMode(13, OUTPUT);                // configure pin 13 which has an on-board LED, useful for diagnostics
  pinMode(ALERT_PIN, OUTPUT);         // set alert pin to drive sonic alert device
  digitalWrite(ALERT_PIN, 0);         // set alert pin to OFF
  
  liresponseTime = millis()/1000;
} // end of setup()


void loop() {
  int iloopMax = 0;
  itoggleCount = 0;
  
  lcd.noBlink();
  
  bSwitchState = bcheckSwitchState();                   // check switch state and set LED accordingly

  // if no response from user after a period of time issue an annoyance
  if(millis()/1000 - liresponseTime > INACTIVITY_TIME){
    warble(600);
    displayLCD(F("Are you there?  "),F(""), 0,0);
    liresponseTime = millis()/1000;
    }

  if(bSwitchState == 1)                                 // if switch is on then process behaviors
  { 
    iprevSwitchState = 1;            
    itoggleCount = 1;                                    
    // count number of switch activations
    linow = millis();
    while(millis() < linow + 800)
    {
      if(bSwitchState == 1 && iprevSwitchState == 0)
      {
        itoggleCount++; 
        linow = millis();                                   // user is toggling switch, continue to wait for more input     
      }
      iprevSwitchState = bSwitchState;
      bSwitchState = bcheckSwitchState();
    }
  
    // process various behaviors based on toggle count, then proceed to normal behaviors (replace with switch statement...)
    // display lock status
 
    if(itoggleCount == 42) 
    {
      if(iYourScore > SCORE_THRESHOLD)
      {
         /*************************************
         * check status of all locks.  If all 
         * conditions are met then allow password attempt
         * 
         *************************************/
        if(checkLockStatus() == LOCK_STATUS_UNLOCKED)
        {
          if(1) //  add password check, score check or other gate here if desired...
          {
            displayLCD(F("Amazon gift"),F("card number is:"), 60,5000);       
            displayLCD(F("0000-0000-0000"),F("0000-0000-0000"), 0,6000);   // just short enouth to cause consternation (next time have a pencil and paper handy)
          }
          else
          { 
            displayLCD(F(" wrong password"),F(""), 60,4000);        
          }
        }
        else
        {
            displayLCD(F("One or more"),F("locks remain"), 0,4000);       
        }
      }
      else
      {
         displayLCD(F("Score too low to"),F("for that..."), 0,4000);   
      }
    }
    else if (itoggleCount > 33)
    {
      displayLCD(F("Cut it out!"),F(""), 2000,4000);
    }
    else if (itoggleCount > 22)
    {
      displayLCD(F("Are you trying"),F("to wear me out?"), 120,2000);
    }
    else if (itoggleCount > 14)
    {
      displayLCD(F("Are you trying"),F("to annoy me?"), 60,2000);
    }
    else if (itoggleCount > 8)
    {
      displayLCD(F("Are you trying"),F("to confuse me?"), 0,2000);
    }
    // display hint
    else if (itoggleCount == 8)
    {
      if(analogRead(LIGHT_SENSE_PIN) < LIGHT_THRESHOLD)
      {
        displayLCD(F("Hint must unlock"),F("all locks"), 0,2000);         
        displayLCD(F("... to reveal"),F("prize"), 0,2000);         
        // add other specific hints here if desired
      }
      else
      {
        displayLCD(F("Too much light"),F("to reveal hint"), 0,2000);      
      }
    }
    // flap door based on number of switch toggles
    else if(itoggleCount == 3) flap(itoggleCount);

    liresponseTime = millis()/1000;                   // get current on time when switch was set

    // done with multi toggle processing, proceed to one or more random behaviors
    
    /*********************************************************
    * 
    * long wait - delay response
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,100) <= LONG_WAIT_PROB))
    {   
      displayLCD(F("Calculating next"),F("move..."), 0,0);       
      i = myrandom(LONG_DELAY_MIN,LONG_DELAY_MAX);                                 
      while(j++ < i && bSwitchState == 1){
        delay(1000);                  
        lcd.setCursor (8,1);                    
        lcd.print(myrandom(7,9678));
        bSwitchState = bcheckSwitchState();               
      }
      lcd.clear();   
    }

    /*********************************************************
    * 
    * slow deploy slowly deploy and then retract actuator without
    * turning switch off
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= SLOW_DEPLOY_PROB)) slowDeploy();

    /*********************************************************
    * 
    * Peek - open actuator door once and retract actuator without 
    * turning switch off
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= PEEK_PROB)) peekout(myrandom(600,1200), myrandom(900, 1700));
  
    /*********************************************************
    * 
    * flicker LED, show calculation of next move
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= FLICKER_PROB)) flicker(myrandom(3,27));

    /*********************************************************
    * 
    * multi peek behavior
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= MULTI_PEEK_PROB))
    {   
      delay(myrandom(500, 1800));  
      bSwitchState = bcheckSwitchState();
      i = 0;
      iloopMax = myrandom(2,7);
      while(i++ < iloopMax && bSwitchState == 1)
      {
        peekout(myrandom(90,1200), myrandom(600, 2100));
        bSwitchState = bcheckSwitchState();
      }
    }

    /*********************************************************
    * 
    * "Speak to me"   - ask user to speak a command
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= SPEAK_TO_ME_PROB) && (itrialCount > SPEAK_TO_ME_MIN)) speak();

    /*********************************************************
    * 
    * display orientation reading
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= ORIENTATION_PROB) && (itrialCount > DISPLAY_READINGS_MIN + 20)) DisplayMagReading();

    /*********************************************************
    * 
    * display light reading
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= LIGHT_PROB) && (itrialCount > DISPLAY_READINGS_MIN + 40)) iDisplayLightReading();

    /*********************************************************
    * 
    * display distance reading
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= DISTANCE_PROB) && (itrialCount > DISPLAY_READINGS_MIN +60)) iDisplayDistanceReading();

    /*********************************************************
    * 
    * display perseverence reading
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,101) <= PERSEVERANCE_PROB) && (itrialCount > DISPLAY_READINGS_MIN + 80)) iDisplayPerseverance();

    /*********************************************************
    * 
    * Flap - bounce actuator door in a random fashion 
    * 
    *********************************************************/
    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1 && (myrandom(1,100) <= FLAP_PROB)) flap(myrandom(2,13));

    /*********************************************************
    * 
    * Done with behaviors, apply perseverance credit, turn off 
    * switch if it remains on.  
    * 
    *********************************************************/

    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1) 
    {
      // switch remains on, turn it off, calculate and display scores
      turnSwitchOff(); 
      displayScores();                                            
    }
    else
    {
      // switch was manually turned off during processing above
      // inform user to turn switch back on to continue game
      lcd.off();
      actuatorHome();            
      // add one or more conditional taunts here...
      
      if((myrandom(1,101) <= GIVEUP_PROB) || itrialCount < 10) 
      {
        displayLCD(F("Giving up after"),F("only"), 0,0);  
        lcd.setCursor (5,1); 
        lcd.print(itrialCount);   
        lcd.print(F(" tries?"));              
      }
      else
      {
        displayLCD(F("Turn switch on"),F("to continue game"), 0,0);            
      }
    }
    liresponseTime = millis()/1000;                   // reset response time counter to account for time spent playing this round
                                                      // this prevents premature inactivity warning                                                         
  } // end of if(bSwitchState == 1)  
} // end of loop()

/*********************************************************************
 * Check state of toggle switch and set LED accordingly, must have 
 * stable switch reading for x duration
 * 
 *********************************************************************/
bool bcheckSwitchState()
{
  bool bSwitchState;
  bool bPrevSwitchState;
  int i = 0;

  bSwitchState = digitalRead(SWITCH_PIN);               // read digital input connected to switch 
  bPrevSwitchState = digitalRead(SWITCH_PIN);           // read digital input connected to switch 
  while(i++ < 6)                                        // must have stable signal for a minimum of time before accepting
  {
    bSwitchState = digitalRead(SWITCH_PIN);             // read digital input connected to switch 
    if (bSwitchState != bPrevSwitchState)
    {
      i = 0;                                            // reset counter, begin new wait 
    }
    delay(8);
    bPrevSwitchState = bSwitchState;
  }
  digitalWrite(LED_PIN, bSwitchState);                  // set LED according to switch reading (switch on turns LED on)
  return bSwitchState;
}

/*********************************************************************
 * deploy actuator slowly to a point almost touching the switch then
 * retract without turning switch off.  
 * 
 *********************************************************************/
void slowDeploy(void)
{
  int i = ACTUATOR_HOME_POS;
  lcd.off();
  actServo.attach(ACT_PIN);    
  while(i-- > ACTUATOR_FWD_POS + 12 && bSwitchState == 1)
  { 
    actServo.write(i);
    delay(60);
    bSwitchState = bcheckSwitchState();
  }
  actServo.detach();    
  delay(myrandom(900,1800));
  actuatorHome();
}

/*********************************************************************
 * flap actuator door n times based on iflapCount parameter
 * 
 * 
 *********************************************************************/
void flap(int iflapCount)
{
  int i = 0;
  lcd.off();
  actServo.attach(ACT_PIN);    
  while(i++ < iflapCount && bSwitchState == 1)
  {
    actServo.write(FLAP_POS);
    delay(80);
    actServo.write(ACTUATOR_HOME_POS);
    delay(80);
    bSwitchState = bcheckSwitchState();
  }
  actServo.detach();    
}

/*********************************************************************
 * similar to flap, open actuator door and start actuator according
 * to parameters
 * 
 *********************************************************************/
void peekout(int ipeekTime, int ipeekDelay)
{
  lcd.off();
  actServo.attach(ACT_PIN);    
  actServo.write(FLAP_POS);
  delay(ipeekTime);
  actServo.write(ACTUATOR_HOME_POS);
  delay(600);
  actServo.detach();    
  delay(ipeekDelay);
}  

/*********************************************************************
 * ask user to speak a command then provide a response
 * 
 * 
 *********************************************************************/
void speak(void)
{
  warble(100);
  lcd.clear();                              
  lcd.home();  
  switch (myrandom(1,8))
  {
    case 1:  
      displayLCD(F("Go up one level?"),F("say YES or NO"), 0,0);             
      break;

     case 2:   
      displayLCD(F("Read light lvl?"),F("say YES or NO"), 0,0);             
      break;

    case 3:       
      displayLCD(F("Clearly say the"),F("password"), 0,0);          
      break;
      
   case 4:     
      displayLCD(F("Change mode?"),F("say YES or NO"), 0,0);              
      break;
      
    case 5:       
      displayLCD(F("I am listening.."),F(""), 0,0);   
      break;

    case 6: 
      displayLCD(F("Speak a command"),F("microphone is on"), 0,0);             
      break;
    
    case 7:   
      displayLCD(F("Multiply score?"),F("say YES or NO"), 0,0);           
      break;
    }
    
  // give user time to respond to request  
  delay(VOX_RESPONSE_DELAY);

  // continue only if switch is on
  bSwitchState = bcheckSwitchState();
  if(bSwitchState == 1)
  {

    // setup responses to voice input
    switch(myrandom(1,8))
    {
      case 1:
        displayLCD(F("I dont recognize"),F("that command"), 0,0);           
        break;
        
      case 2:
        displayLCD(F("Please try again"),F("later"), 0,0);          
        break;
        
      case 3:
        displayLCD(F("Score too low to"),F("take that action"), 0,0);            
        break;
        
      case 4:
        displayLCD(F("That function is"),F("in lockout mode"), 0,0);       
        break;
        
      case 5:
        displayLCD(F("Unavailable at"),F("this time"), 0,0);          
        break;              
        
     case 6:
        displayLCD(F("Command not"),F("understood"), 0,0);         
        break;       
        
     case 7:
  
        if(myrandom(1,100) < 2)
        {
          displayLCD(F("The answer is:"),F("42"), 0,0);              
        }
        else
        {
          displayLCD(F("Can not process"),F("that input"), 0,0);             
        }
        break;          
    }
    // give user time to digest response 
    delay(VOX_RESPONSE_DELAY);
  }
}

/*********************************************************************
 * display real time magnetic sensor reading for a brief period of time
 * 
 * 
 *********************************************************************/
void DisplayMagReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Orientation is: ");
  linow = millis();
  while((millis() < linow + 6000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("            "));  
    lcd.setCursor (0,1); 
    lcd.print(ireadMagSensor());  
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display real time distance sensor reading for a brief period of time
 * 
 * 
 *********************************************************************/
void iDisplayDistanceReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Distance is:    ");
  linow = millis();
  while((millis() < linow + 6000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("            "));  
    lcd.setCursor (0,1); 
    lcd.print(getDistance());   
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display real time light sensor reading for a brief period of time
 * 
 * 
 *********************************************************************/
void iDisplayLightReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Light reading:  ");
  linow = millis();
  while((millis() < linow + 6000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("            "));  
    lcd.setCursor (0,1); 
    lcd.print(analogRead(LIGHT_SENSE_PIN));   
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display perseverance.  This is the total number of rounds which the 
 * player has completed
 * 
 *********************************************************************/
void iDisplayPerseverance(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print(F("Persevarance is:"));
  lcd.setCursor (0,1);  
  lcd.print(itrialCount);  
  delay(4000);
}

/*********************************************************************
 * flicker led, display next move message for random amount of time
 * 
 * 
 *********************************************************************/
void flicker(int iflickerCount)
{
  i = 0;
  j = 0;
  displayLCD(F("Calculating next"),F("move..."), 0,0);       

  while(i++ < iflickerCount && bSwitchState == 1)
  {
    digitalWrite(LED_PIN,1);                            // turn LED off
    delay(myrandom(23,871));
    bSwitchState = bcheckSwitchState();                 // user may have changed switch setting, re-read and act accordingly 
    lcd.setCursor (8,1);                    
    lcd.print(myrandom(2,9897));
    digitalWrite(LED_PIN,0);                            // turn LED off
    delay(myrandom(23,871));
    bSwitchState = bcheckSwitchState();                 // user may have changed switch setting, re-read and act accordingly 
    lcd.setCursor (8,1);                    
    lcd.print(myrandom(2,9897));
  }
}

/*********************************************************************
 * use actuator to turn switch off
 * 
 * 
 *********************************************************************/
void turnSwitchOff(void)
{
    actuatorForward();
    actuatorHome();
    itrialCount++;                    // count number of rounds in game (one round = one switch turn off by machine)
}

/*********************************************************************
 * move actuator forward to switch off position
 * 
 * 
 *********************************************************************/
void actuatorForward(void)
{
  lcd.off();
  actServo.attach(ACT_PIN);           
  actServo.write(ACTUATOR_FWD_POS);                 // Step 3b - move actuator forward to turn switch off
  delay(650);                                       // give the servo time to reach its position 
  actServo.detach();         
                                                    // read the switch so as to immediately change the LED indicator
  bSwitchState = digitalRead(SWITCH_PIN);           // get the switch reading
  digitalWrite(LED_PIN, bSwitchState);              // set LED according to switch reading (should be off at this point)
  lcd.on();
}

/*********************************************************************
 * move actuator back to home position
 * 
 * 
 *********************************************************************/
void actuatorHome(void)
// called as part of turnSwitchOff or may be called independently if switch is turned off by user
{
  lcd.off();
  actServo.attach(ACT_PIN);         
  actServo.write(ACTUATOR_HOME_POS);                // move the actuator to home position (back inside the box)
  delay(650);                                       // give the servo time to reach its position
  actServo.detach();      
  lcd.on();
}

/*********************************************************************
 * turn sonic alert on for duration specified by parameter.  Using
 * sonic alert which has a built in alternating high-low tones.
 * 
 *********************************************************************/
void warble(int iduration)
{
  digitalWrite(ALERT_PIN, 1);         // sonic alert on
  delay(iduration);
  digitalWrite(ALERT_PIN, 0);         // sonic alert off
}

/*********************************************************************
 * calculate score totals for current round and displays
 * 
 * 
 *********************************************************************/
void displayScores()
{
  // calculate scores, of course they are random and the outcome is that the box always wins
  iMyScore += myrandom(4,7);
  if(myrandom(1,101) <= SCORE_BUMP_PROB)
  {                            
    iYourScore = iMyScore - 1;    
  }
  else
  {
   if(myrandom(1,101) <= SCORE_PENALTY_PROB)   
   {
      displayLCD(F("Score penalty"),F("due to violation"), 0,1500);        
      iYourScore = iYourScore - (iYourScore / 5);
   }
    else
    {
        iYourScore += myrandom(1,4);
    }
  }
  // maintain maximum of 3 digits for score
  if(iMyScore > 999)
  {
     iMyScore = 1;
     iYourScore = 0;
  }
  
  // display results
  if(myrandom(1,101) <= YOU_WIN_PROB)
  {      
    // taunt option 1   
    displayLCD(F("Congratulations"),F("You won!"), 780,2000);   
    displayLCD(F("Just kidding..."),F(""), 0,2000);   
  }
  // taunt 2
  else if(myrandom(1,101) <= I_AM_SORI_PROB)
  {      
    displayLCD(F("I am Sori"),F(""), 0,1500);   
    lcd.setCursor (0,1);  
    lcd.print("..that you lost!");
    delay(3000);
  }
  if(myrandom(1,101) <= GO_AWAY_PROB)
  {                    
    displayLCD(F("I win again"),F("please go away!"), 0,0);   
  }
  else
  {
    // normal behavior          
    lcd.clear();                              
    lcd.home();    
    lcd.print("I win ");
    lcd.print(iMyScore);
    lcd.print(" - ");  
    lcd.print(iYourScore);   
    lcd.setCursor (0,1);  
    lcd.print("your move...    ");    
  }
}

/*********************************************************************
 * get magnetic sensor reading
 * 
 * 
 *********************************************************************/
int ireadMagSensor(void)
{
  int x = 0;
  //Tell the HMC5883L where to begin reading data
  Wire.beginTransmission(address);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  
  //Read data from each axis, 2 registers per axis
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
  }
  return x;  
}


/*****************************************************************
 * checkLockStatus
 * Displays status of individual locks, once all locks have been
 * unlocked a password screen may be displayed.
 * 
 * This function may be setup in while loop to continuously 
 * display lock statuses and allow user input to attempt unlock
 * or statuses may be shown once only.  Also may toggle unlock flags
 * for a 'once and done' effect or reset them each time through
 * depending on degree of difficulty desired
 * Option:  Allow user to dwell on each reading and have display show
 * too high - too low (bargraph style with a mid-point target)
 * once unlocked progress to next.
 *****************************************************************/
int checkLockStatus()
{    
    bool bPerseverance = 0;         // lock status flag
    bool bScore = 0;                // lock status flag
    bool bMagStatus = 0;            // lock status flag
    bool bLightStatus = 0;          // lock status flag
    bool bDistanceStatus = 0;       // lock status flag
    
    // Display lock statistics
    /*************************************
     * Perseverance Score
     * Note - option to make game more difficult
     * by resetting locks on each attempt.
     *************************************/
    bSwitchState = bcheckSwitchState();
    while(bSwitchState == 1)
    {

    /*************************************
     * perseverance check
     * 
     *************************************/      
    lcd.clear();          
    lcd.home (); 
    lcd.print(F("Perseverance:   ")); 
    lcd.setCursor (0,1);        
    if(itrialCount > PERSEVERANCE_THRESHOLD)
    {
      bPerseverance = 1;           // once unlocked remain unlocked
      lcd.print(itrialCount);       
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);
    }
    else
    {
      lcd.print(itrialCount);   
      lcd.print(F(" = LOCKED"));   
    }
    delay(2500);
    bSwitchState = bcheckSwitchState(); 

    /*************************************
     * Score check
     * 
     *************************************/      
    lcd.clear();          
    lcd.home (); 
    lcd.print(F("Score:")); 
    lcd.setCursor (0,1);        
    if(iYourScore > SCORE_THRESHOLD)
    {
      bScore = 1;                 // once unlocked remain unlocked
      lcd.print(iYourScore);       
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);
    }
    else
    {
      lcd.print(iYourScore);   
      lcd.print(F(" = LOCKED"));   
    }
    delay(2500);
    bSwitchState = bcheckSwitchState(); 

    /*************************************
     * Magnetic Sensor Reading
     * 
     *************************************/
    lcd.clear();          
    lcd.home (); 
    lcd.print(F("Orientation:    ")); 
    lcd.setCursor (0,1);  
    if((iMagOrient = ireadMagSensor()) > 0)
    {
      bMagStatus = 1;            
      lcd.print(iMagOrient);    
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);
    }
    else
    {
       bMagStatus = 0;            
       lcd.print(iMagOrient);       
       lcd.print(F(" = LOCKED")); 
    }
    delay(2500);
    bSwitchState = bcheckSwitchState(); 
    
    /*************************************
     * Light Sensor Reading
     * 
     *************************************/
    lcd.clear();          
    lcd.home (); 
    lcd.print(F("Light level:   ")); 
    lcd.setCursor (0,1);    
    iLightSense = analogRead(LIGHT_SENSE_PIN);
    if(iLightSense < LIGHT_THRESHOLD)
    {
      bLightStatus = 1;           
      lcd.print(iLightSense); 
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);   
    }
    else
    {
      bLightStatus = 0; 
      lcd.print(iLightSense); 
      lcd.print(F(" = LOCKED")); 
    }
    delay(2500);
    bSwitchState = bcheckSwitchState(); 

    /*************************************
     * Distance Sensor Reading
     * 
     *************************************/
    lcd.clear();          
    lcd.home (); 
    lcd.print(F("Distance:       ")); 
    lcd.setCursor (0,1);   
    iDistance = getDistance();   
    if(iDistance < DISTANCE_THRESHOLD)
    {
      bDistanceStatus = 1;           
      lcd.print(iDistance); 
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);   
    }
    else
    {
      bDistanceStatus = 0;   
      lcd.print(iDistance); 
      lcd.print(F(" = LOCKED")); 
    }
    delay(2500);
    bSwitchState = bcheckSwitchState(); 
        
    /*************************************
     * 
     * 
     *************************************/
    } // end while

    if(bPerseverance && bMagStatus && bLightStatus && bDistanceStatus && bScore) 
    {
      return LOCK_STATUS_UNLOCKED;
    }
    else
    {
      return LOCK_STATUS_LOCKED;
    }
}

/*********************************************************************
 * get distance sensor reading
 * 
 * 
 *********************************************************************/
int getDistance()
{
  // adapted from: https://gist.github.com/flakas/3294829
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(US_TRIGGER_PIN, OUTPUT);
  digitalWrite(US_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIGGER_PIN, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(US_ECHO_PIN, INPUT);
  return (pulseIn(US_ECHO_PIN, HIGH) / 72 / 2);
}

/*********************************************************************
 * function to deliver pseudo-random number
 * 
 * 
 *********************************************************************/
int myrandom(int min, int max)
{
  randomSeed(analogRead(0));
  int iret = 0;
  //iret = min + rand()% (max - min); 
  // replacing with call to micros() will only seem random because of difference in timing of user input.
  iret = min + micros()% (max - min); 
  
  #define xAND_TEST_MODE
  #ifdef RAND_TEST_MODE
      Serial.println();
      Serial.print(min);
      Serial.print(", ");
      Serial.print(max);
      Serial.println();
      for (int i = 0; i<100; i++)
      {
        iret = min + rand() % (max - min);
        Serial.println(iret);
      }
  #endif

  #define DISP_RAND
  #ifdef DISP_RAND
      Serial.println();
      Serial.print(min);
      Serial.print(", ");
      Serial.print(max);
      Serial.println();
      Serial.println(iret);
  #endif

  
  return iret;
}

/*********************************************************************
 * general purpose display manager with optional tone alert and delay
 * 
 * 
 *********************************************************************/
void displayLCD(String sline1, String sline2, int itoneLen, int idelay)
{
    lcd.off(); 
    lcd.clear();          
    lcd.home (); 
    lcd.print(sline1); 
    lcd.setCursor (0,1);        
    lcd.print(sline2); 
    lcd.on();  
    if (itoneLen > 0) warble(itoneLen);
    if (idelay > 0) delay(idelay);
}


  

