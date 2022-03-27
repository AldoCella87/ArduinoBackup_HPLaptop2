// Jake and Ben - Thanks for showing me the concept of the useless machine during our last visit.  I think it is pretty
// cool and could be made using an Arduino and a couple of servo motors and a switch.  Here is a starting point.
// You will have to fine tune the servo position numbers below to match your design.  This code uses two servo motors, one to open the box lid
// and one to move the arm which turns the switch off.  You could use a single servo if you design the arm to also lift the box lid on the way out. 

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
#define LID_CLOSED        10    // the servo starts at this position and returns to this position to close the lid
#define LID_OPEN          90    // the servo goes to this position to open the lid
#define ACTUATOR_HOME_POS 143   // the servo starts at this position and returns to this position after turning the switch off
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

int iSwitchState = 0;           // variable which holds the current switch reading
int i = 0;                      // generic loop control
int j = 0;
int iMyScore, iYourScore = 0;
int inextMoveNumber = 0;        // number to display to user purporting to be calculation of next move
long liresponseTime = 0;        // determines response duration

int  itoggleCount = 0;
long linow = 0;                  // holds current time in ms, used for multiple switch toggle count
byte iprevSwitchState = 0;

bool bPerseverance = 0;         // one of a group of password unlock flags
bool bMagStatus = 0;            // one of a group of password unlock flags
bool bLightStatus = 0;          // one of a group of password unlock flags
bool bDistanceStatus = 0;       // one of a group of password unlock flags

int iPerseverance = 0;          // holds perseverance value, incremented as user progresses
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


  #define LOCK_STATUS_UNLOCKED 1
  #define LOCK_STATUS_LOCKED    0

  
#define  TEST_MODE

#ifdef TEST_MODE

  #define PERSEVERANCE_THRESHOLD  0                 // reading greater than this value required to unlock
  #define LIGHT_THRESHOLD 90                        // reading less than this value required to unlock
  #define MAG_THRESHOLD 0                           // reading greater than this value required to unlock
  #define DISTANCE_THRESHOLD 16                     // reading less than this value required to unlock
  
  #define INACTIVITY_TIME   600                     // seconds to wait before issuing inactivity taunt    
  #define LONG_DELAY_MIN    10 
  #define LONG_DELAY_MAX    25
  #define MAGNETO_PROB      100
  #define LIGHT_PROB        100
  #define DISTANCE_PROB     100
  #define PERSEVERANCE_PROB 100
  #define FLAP_PROB         0
  #define FLICKER_PROB      0
  #define PEEK_PROB         0
  #define MULTI_PEEK_PROB   0      
  #define LONG_WAIT_PROB    0
  #define SLOW_DEPLOY_PROB  0
  #define SCORE_BUMP_PROB   0
  #define YOU_WIN_PROB      0
  #define GO_AWAY_PROB      0
  #define SPEAK_TO_ME_PROB  0                       // simulated voice input processing
  #define I_AM_SORI_PROB    0                       // "I am Sori ... (pause) "that you are losing"
  #define SCORE_RESET_PROB  100                     // "User Response Error Your score has been reset to zero"
  #define VOX_RESPONSE_DELAY  4500                  // ms to wait for voice response

#else

  #define PERSEVERANCE_THRESHOLD  300               // number of rounds required to obtain perseverance unlock  
  #define LIGHT_THRESHOLD 90                        // reading required to unlock
  #define MAG_THRESHOLD 0                           // reading required to unlock
  #define DISTANCE_THRESHOLD 16                     // reading required to unlock
  #define INACTIVITY_TIME   1200                    // seconds to wait before issuing inactivity taunt  
  #define LONG_DELAY_MIN    300                     // minimum delay in seconds
  #define LONG_DELAY_MAX    1200                    // maximum delay in seconds
  #define FLAP_PROB         4
  #define MAGNETO_PROB      1
  #define LIGHT_PROB        1
  #define DISTANCE_PROB     1
  #define PERSEVERANCE_PROB 1
  #define FLICKER_PROB      4
  #define PEEK_PROB         4
  #define MULTI_PEEK_PROB   4      
  #define LONG_WAIT_PROB    4
  #define SLOW_DEPLOY_PROB  4
  #define SCORE_BUMP_PROB   8
  #define YOU_WIN_PROB      2
  #define GO_AWAY_PROB      2
  #define SPEAK_TO_ME_PROB  5
  #define I_AM_SORI_PROB    2    
  #define SCORE_RESET_PROB  0  
  #define VOX_RESPONSE_DELAY  4500                  // ms to wait for voice response                               

#endif




/***********************************************************************************************
 * The setup function runs only one time when the Arduino is reset or powered on
 * This is where initial values are setup.   You shouldn't have to change anything here except
 * the servo motor positions below. 
 *
 ***********************************************************************************************/
void setup() {
  // mag sensor setup
    Wire.begin();
  
  //Setup HMC5883 sensor
  Wire.beginTransmission(address);            //open communication with HMC5883
  Wire.write(0x02);                           //select mode register
  Wire.write(0x00);                           //continuous measurement mode
  Wire.endTransmission();
  // end mag sensor setup

  randomSeed(analogRead(0));
  
  actServo.attach(ACT_PIN);                   // start with actuator in home position
  actServo.write(ACTUATOR_HOME_POS);
  
  lcd.off(); 
  lcd.begin (16,2);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();  
  lcd.print("My name is Sori");
  lcd.setCursor (0,1);  
  lcd.print("Personality: ");
  lcd.print(random(42,121)); 
  lcd.on();  
  
  delay(4000);
  lcd.off();
  lcd.home ();  
  lcd.print("Score 000 to 000");
  lcd.setCursor (0,1);  
  lcd.print("your move...    ");
  lcd.on();

  
  Serial.begin(9600);                 // start the serial output stream, this enables displaying informational messages on the monitor

  pinMode(SWITCH_PIN, INPUT);         // configure switch pin as an input.  Connect a resistor between this pin and ground to 
                                      // force it to a low state when switch is open
  pinMode(LED_PIN, OUTPUT);           // configure LED pin as an output, this is for an external LED indicator to accompany the switch
  pinMode(13, OUTPUT);                // configure pin 13 which has an on-board LED, useful for diagnostics
  pinMode(ALERT_PIN, OUTPUT);         // set alert pin to drive sonic alert device
  digitalWrite(ALERT_PIN, 0);         // set alert pin to OFF
  
  // detach servos - leaving servos detached eliminates occasional chatter while in wait state
  //  lidServo.detach();                                
  actServo.detach();
  liresponseTime = millis()/1000;
} // end of setup()


/***********************************************************************************************
 * The loop function runs after setup has been completed.  It will continue to run until the
 * Arduino is turned off or reset using the reset switch
 * This code simply checks to see if the switch has been turned on, sets the
 * LED according to switch state then, if switch is on, it attempts to turn it off.
 *
 ***********************************************************************************************/
void loop() {

  int iloopMax = 0;
  itoggleCount = 0;

  lcd.noBlink();
  iSwitchState = digitalRead(SWITCH_PIN);             // Step 1 - get the switch reading
  digitalWrite(LED_PIN, iSwitchState);                // Step 2 - set LED according to switch reading (switch on turns LED on)
  //digitalWrite(13, iSwitchState);                     // also set the user assignable LED which resides on the Arduino board

  // if no response from user after a period of time issue an annoyance
  if(millis()/1000 - liresponseTime > INACTIVITY_TIME){
    warble(600);
    lcd.off(); 
    lcd.clear();          
    lcd.home (); 
    lcd.print("Are you there?  "); 
    lcd.setCursor (0,1);        
    lcd.print("                "); 
    lcd.on();  
    // reset counter
    liresponseTime = millis()/1000;
    }


  if(iSwitchState == 1)                                 // Step 3 - if switch is on then process behaviors
  { 
  iprevSwitchState = 1;            
  itoggleCount = 1;                                       // we have detected the first switch on state
  
  // check for multiple switch activations
  linow = millis();
  while(millis() < linow + 1000)
  {
    if(iSwitchState == 1 && iprevSwitchState == 0)
    {
      itoggleCount++; 
      linow = millis();                                   // user is toggling switch, continue to wait for more input     
    }
    iprevSwitchState = iSwitchState;
    iSwitchState = checkSwitchState();
  }



  if (itoggleCount > 7)
  {
    lcd.clear();          
    lcd.home (); 
    lcd.print("Are you trying  "); 
    lcd.setCursor (0,1);        
    lcd.print("to confuse me?  "); 
    warble(60);
    delay(1500);
  }
  else if(itoggleCount == 5) 
  {
     /*************************************
     * check status of all locks.  If all 
     * conditions are met then allow password attempt
     * 
     *************************************/
    if(checkLockStatus() == LOCK_STATUS_UNLOCKED)
    {
      lcd.clear();          
      lcd.home (); 
      lcd.print("Enter Password  "); 
      lcd.setCursor (0,1);        
      lcd.print(">               "); 
      lcd.setCursor (1,1); 
      lcd.blink();       
      delay(8000);
      lcd.noBlink();
    }
  }

    randomSeed(analogRead(0));
    inextMoveNumber = random(39,2723);                // get random number to display as next move calculation
    liresponseTime = millis()/1000;                   // get current on time when switch was set
    lcd.off(); 
    lcd.clear();          
    lcd.home (); 
    lcd.print("calculating next"); 
    lcd.setCursor (0,1);        
    lcd.print("move..."); 
    lcd.print(inextMoveNumber++);
    lcd.on();  
    
    delay(900);                
    // attach servos  
    actServo.attach(ACT_PIN);                           // assign servos to control pins

    
    /*********************************************************
    * 
    * Flap - bounce actuator door in a random fashion 
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,100) <= FLAP_PROB)) flap(random(2,13));
    
    /*********************************************************
    * 
    * long wait - delay response
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,100) <= LONG_WAIT_PROB))
    {   
      lcd.on();                                         // turn on, may have been turned off in previous processing
      i = random(LONG_DELAY_MIN,LONG_DELAY_MAX);                                 
      while(j++ < i && iSwitchState == 1){
        delay(1000);                  
        lcd.setCursor (8,1);                    
        lcd.print(inextMoveNumber++);
        iSwitchState = checkSwitchState();               
      }
    }

    /*********************************************************
    * 
    * slow deploy slowly deploy and then retract actuator without
    * turning switch off
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= SLOW_DEPLOY_PROB)) slowDeploy();

    /*********************************************************
    * 
    * Peek - open actuator door once and retract actuator without 
    * turning switch off
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= PEEK_PROB)) peekout(random(600,1200), random(900, 1700));
  
    /*********************************************************
    * 
    * flicker LED, show calculation of next move
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= FLICKER_PROB)) flicker(random(3,27));

    /*********************************************************
    * 
    * multi peek behavior
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= MULTI_PEEK_PROB))
    {   
      delay(random(500, 1800));  
      iSwitchState = checkSwitchState();
      i = 0;
      iloopMax = random(2,7);
      while(i++ < iloopMax && iSwitchState == 1)
      {
        peekout(random(90,1200), random(600, 2100));
        iSwitchState = checkSwitchState();
      }
    }

    /*********************************************************
    * 
    * "Speak to me"   - ask user to speak a command
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= SPEAK_TO_ME_PROB)) speak();

    /*********************************************************
    * 
    * display magnetometer reading
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= MAGNETO_PROB)) iDisplayMagReading();

    /*********************************************************
    * 
    * display light reading
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= LIGHT_PROB)) iDisplayLightReading();

    /*********************************************************
    * 
    * display distance reading
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= DISTANCE_PROB)) iDisplayDistanceReading();

    /*********************************************************
    * 
    * display perseverence reading
    * 
    *********************************************************/
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1 && (random(1,101) <= PERSEVERANCE_PROB)) iDisplayPerseverance();
    
    /*********************************************************
    * 
    * Done with behaviors, apply perseverance credit, turn off 
    * switch if it had not been done so already.  
    * 
    *********************************************************/
    iPerseverance++;                    // one of a number of values required for password unlock
    iSwitchState = checkSwitchState();  
    if(iSwitchState == 1) 
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
      lcd.clear();                              
      lcd.home();  
      // add one or more conditional taunts here...
      lcd.print("Turn switch on  ");
      lcd.setCursor (0,1);  
      lcd.print("to continue game");
      lcd.on();
    }
    liresponseTime = millis()/1000;                   // reset response time counter to account for time spent playing this round
                                                      // this prevents premature inactivity warning
    actServo.detach();                                // silence servos during wait time
                                                          
  } // end of if(iSwitchState == 1)  
} // end of loop()


byte checkSwitchState()
{
  byte iSwitchState;
  byte iPrevSwitchState;
  int i = 0;

  iSwitchState = digitalRead(SWITCH_PIN);             // read digital input connected to switch 
  iPrevSwitchState = digitalRead(SWITCH_PIN);           // read digital input connected to switch 
  while(i++ < 10)                                       // must have stable signal for a minimum of time before accepting
  {
    iSwitchState = digitalRead(SWITCH_PIN);             // read digital input connected to switch 
    if (iSwitchState != iPrevSwitchState)
    {
      i = 0;                                            // reset counter, start over
    }
    delay(10);
    iPrevSwitchState = iSwitchState;
  }
  digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
  return iSwitchState;
}

void slowDeploy(void)
{
  int i = ACTUATOR_HOME_POS;
  lcd.off();

  while(i-- > ACTUATOR_FWD_POS +13 && iSwitchState == 1)
  { 
    actServo.write(i);
    delay(100);
    iSwitchState = checkSwitchState();
  }
  delay(random(900,1800));
  actuatorHome();
}


void flap(int iflapCount)
{
  int i = 0;
  lcd.off();
  while(i++ < iflapCount && iSwitchState == 1)
  {
    actServo.write(FLAP_POS);
    delay(80);
    actServo.write(ACTUATOR_HOME_POS);
    delay(80);
    iSwitchState = checkSwitchState();
  }
}

void peekout(int ipeekTime, int ipeekDelay)
{
    lcd.off();
    actServo.write(FLAP_POS);
    delay(ipeekTime);
    actServo.write(ACTUATOR_HOME_POS);
    delay(ipeekDelay);
}  

void speak(void)
{
  warble(100);
  lcd.clear();                              
  lcd.home();  
  switch (random(1,8))
  {
    case 1:           
      lcd.print("Go up one level?");
      lcd.setCursor (0,1);  
      lcd.print("say YES or NO   ");
      break;

     case 2:              
      lcd.print("Add light input?");
      lcd.setCursor (0,1);  
      lcd.print("say YES or NO   ");
      break;

    case 3:               
      lcd.print("Clearly say the ");
      lcd.setCursor (0,1);  
      lcd.print("password        ");
      break;
      
   case 4:               
      lcd.print("Change mode?    ");
      lcd.setCursor (0,1);  
      lcd.print("say YES or NO   ");
      break;
      
    case 5:       
      lcd.print("I am listening..");
      lcd.setCursor (0,1);  
      lcd.print("                ");
      break;

    case 6:       
      lcd.print("Speak a command ");
      lcd.setCursor (0,1);  
      lcd.print("microphone is on");
      break;
    
    case 7:       
      lcd.print("Multiply score? ");
      lcd.setCursor (0,1);  
      lcd.print("say YES or NO   ");
      break;
    }
  lcd.on();
  // give user time to respond to request  
  delay(VOX_RESPONSE_DELAY);
  lcd.clear();                              
  lcd.home();  
  
  // setup responses to voice input
  switch(random(1,8))
  {
    case 1:
      lcd.print("I dont recognize");
      lcd.setCursor (0,1);  
      lcd.print("that command    ");
      break;
      
    case 2:
      lcd.print("Please try again");
      lcd.setCursor (0,1);  
      lcd.print("later           ");
      break;
      
    case 3:
      lcd.print("Score too low to");
      lcd.setCursor (0,1);  
      lcd.print("take that action");
      break;
      
    case 4:
      lcd.print("That function is");
      lcd.setCursor (0,1);  
      lcd.print("in lockout mode ");
      break;
      
    case 5:
      lcd.print("Unavailable at  ");
      lcd.setCursor (0,1);  
      lcd.print("this time       ");
      break;              
      
   case 6:
      lcd.print("Command not     ");
      lcd.setCursor (0,1);  
      lcd.print("understood      ");
      break;       
      
   case 7:

      if(random(1,100) < 2)
      {
        lcd.print("The answer is:  ");
        lcd.setCursor (0,1);  
        lcd.print("42              ");
      }
      else
      {
        lcd.print("Can not process ");
        lcd.setCursor (0,1);  
        lcd.print("that input      ");
      }
      break;          
  }
  // give user time to digest response 
  delay(VOX_RESPONSE_DELAY);
}


void iDisplayMagReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Orientation is: ");
  lcd.setCursor (0,1);  
  lcd.print(ireadMagSensor());  
  delay(4000);
}

void iDisplayDistanceReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Orientation is: ");
  lcd.setCursor (0,1);  
  lcd.print(getDistance());  
  delay(4000);
}

void iDisplayLightReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Light reading:  ");
  lcd.setCursor (0,1);  
  lcd.print(analogRead(LIGHT_SENSE_PIN));  
  delay(4000);
}

void iDisplayPerseverance(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Persevarance is:");
  lcd.setCursor (0,1);  
  lcd.print(iPerseverance);  
  delay(4000);
}

void flicker(int iflickerCount)
{
  i = 0;
  j = 0;
  
  lcd.on();                                             // turn on, may have been turned off in previous processing
  while(i++ < iflickerCount && iSwitchState == 1)
  {
    digitalWrite(LED_PIN,1);                            // turn LED off
    delay(random(23,871));
    iSwitchState = checkSwitchState();                                 // user may have changed switch setting, re-read and act accordingly 
    lcd.setCursor (8,1);                    
    lcd.print(inextMoveNumber++);
    digitalWrite(LED_PIN,0);                            // turn LED off
    delay(random(23,871));
    iSwitchState = checkSwitchState();                                 // user may have changed switch setting, re-read and act accordingly 
    lcd.setCursor (8,1);                    
    lcd.print(inextMoveNumber++);
  }
}

void turnSwitchOff(void)
{
    actuatorForward();
    actuatorHome();
}

void actuatorForward(void)
{
    lcd.off();
    actServo.write(ACTUATOR_FWD_POS);                 // Step 3b - move actuator forward to turn switch off
    delay(650);                                       // give the servo time to reach its position    
                                                      // read the switch so as to immediately change the LED indicator
    iSwitchState = digitalRead(SWITCH_PIN);           // get the switch reading
    digitalWrite(LED_PIN, iSwitchState);              // set LED according to switch reading (should be off at this point)
}

void actuatorHome(void)
// called as part of turnSwitchOff or may be called independently if switch is turned off by user
{
    lcd.off();
    actServo.write(ACTUATOR_HOME_POS);                // move the actuator to home position (back inside the box)
    delay(650);                                       // give the servo time to reach its position
}

void warble(int iduration)
{
        digitalWrite(ALERT_PIN, 1);         // sonic alert on
        delay(iduration);
        digitalWrite(ALERT_PIN, 0);         // sonic alert off
}

void displayScores()
{

  // calculate scores, of course they are random and the outcome is that the box always wins
  iMyScore += random(4,6);
  if(random(1,101) <= SCORE_BUMP_PROB)
  {                           
    iYourScore = iMyScore - 1;
  }
  else
  {
    iYourScore += random(0,3);
  }
  // maintain maximum of 3 digits for score
  if(iMyScore > 999)
  {
     iMyScore = 1;
     iYourScore = 0;
  }
  
  // display results
  if(random(1,101) <= YOU_WIN_PROB)
  {      
    // taunt option 1   
    lcd.off();             
    lcd.clear();                              
    lcd.home();  
    lcd.print("Congratulations ");
    lcd.setCursor (0,1);  
    lcd.print("You won!        ");
    lcd.on(); 
    warble(780);
    delay(2000);
    lcd.off();
    lcd.clear();                              
    lcd.home();  
    lcd.print("Just kidding... ");
    lcd.setCursor (0,1);  
    lcd.print("                ");
    lcd.on(); 
    delay(2000);
    lcd.off();
  }
  // taunt 2
  else if(random(1,101) <= I_AM_SORI_PROB)
  {      
    lcd.off();             
    lcd.clear();                              
    lcd.home();  
    lcd.print("I am Sori       ");
    lcd.on(); 
    delay(1500);
    lcd.setCursor (0,1);  
    lcd.print("..that you lost!");
    delay(3000);
    lcd.off();
   }
   
  if(random(1,101) <= GO_AWAY_PROB)
  {                    
    // annoying behavior     
    lcd.off();             
    lcd.clear();                              
    lcd.home();    
    lcd.print("I win again     ");
    lcd.setCursor (0,1);  
    lcd.print("please go away! ");
  }
  else
  {
    // normal behavior
    lcd.off();             
    lcd.clear();                              
    lcd.home();    
    lcd.print("I win ");
    lcd.print(iMyScore);
    lcd.print(" - ");  
    lcd.print(iYourScore);   
    lcd.setCursor (0,1);  
    lcd.print("your move...    ");    
  }
  lcd.on();                                           // turn display on to show results
   
}

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
    int iRetn = LOCK_STATUS_LOCKED;
    
    // Display lock statistics
    /*************************************
     * Perseverance Score
     * Note - option to make game more difficult
     * by resetting locks on each attempt.
     *************************************/
    iSwitchState = checkSwitchState();
    while(iSwitchState == 1)
    {
    lcd.clear();          
    lcd.home (); 
    lcd.print("Perseverance:   "); 
    lcd.setCursor (0,1);        
    if(iPerseverance > PERSEVERANCE_THRESHOLD)
    {
      bPerseverance = 1;           // once unlocked remain unlocked
      lcd.print(iPerseverance);       
      lcd.print(" = UNLOCKED"); 
      //warble(100);
    }
    else
    {
      lcd.print(iPerseverance);   
      lcd.print(" = LOCKED");   
    }
    delay(3000);
    if(checkSwitchState() == 0) break;

    /*************************************
     * Magnetic Sensor Reading
     * 
     *************************************/
    lcd.clear();          
    lcd.home (); 
    lcd.print("Orientation:    "); 
    lcd.setCursor (0,1);  
          
    if((iMagOrient = ireadMagSensor()) > 0)
    {
      bMagStatus = 1;            
      lcd.print(iMagOrient);    
      lcd.print(" = UNLOCKED"); 
      //warble(100);
    }
    else
    {
       bMagStatus = 0;            
       lcd.print(iMagOrient);       
       lcd.print(" = LOCKED"); 
    }
    delay(3000);
    if(checkSwitchState() == 0) break;
    
    /*************************************
     * Light Sensor Reading
     * 
     *************************************/
    lcd.clear();          
    lcd.home (); 
    lcd.print("Light level:   "); 
    lcd.setCursor (0,1);    
    iLightSense = analogRead(LIGHT_SENSE_PIN);
    if(iLightSense < LIGHT_THRESHOLD)
    {
      bLightStatus = 1;           
      lcd.print(iLightSense); 
      lcd.print(" = UNLOCKED"); 
      //warble(100);   
    }
    else
    {
      bLightStatus = 0; 
      lcd.print(iLightSense); 
      lcd.print(" = LOCKED"); 
    }
    delay(3000);
    if(checkSwitchState() == 0) break;

    /*************************************
     * Distance Sensor Reading
     * 
     *************************************/
    lcd.clear();          
    lcd.home (); 
    lcd.print("Distance:       "); 
    lcd.setCursor (0,1);   
    iDistance = getDistance();   
    if(iDistance < DISTANCE_THRESHOLD)
    {
      bDistanceStatus = 1;           
      lcd.print(iDistance); 
      lcd.print(" = UNLOCKED"); 
      //warble(100);   
    }
    else
    {
      bDistanceStatus = 0;   
      lcd.print(iDistance); 
      lcd.print(" = LOCKED"); 
    }
    delay(3000);
    if(checkSwitchState() == 0) break;
        
    /*************************************
     * If all conditions are met proceed with password attempt
     * 
     *************************************/
    if(bPerseverance && bMagStatus && bLightStatus && bDistanceStatus) 
      iRetn = LOCK_STATUS_UNLOCKED;
    } // end while
    
 
    return iRetn;
}

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

