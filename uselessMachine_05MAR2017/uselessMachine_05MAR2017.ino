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
 ***********************************************************************************************/
 
#include <Servo.h>              // this provides access to the arduino servo functions and is required in order for the code to work

//Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define LIDPIN  9               // digital pin 9 goes to the control line of the lid servo (white (sometimes orange) wire)
#define ACT_PIN 10              // digital pin 10 goes to the control line of the actuator servo (white (sometimes orange) wire)
#define ALERT_PIN  4           // sonic alert 

// lid and actuator servo positions - adjust these as needed     
// A typical servo motor will only rotate 180 degrees.  When given a command it will rotate to that position and stay there.
// Valid servo commands are between 0 and 180.   
#define LID_CLOSED        10    // the servo starts at this position and returns to this position to close the lid
#define LID_OPEN          90    // the servo goes to this position to open the lid
#define ACTUATOR_HOME     143    // the servo starts at this position and returns to this position after turning the switch off
#define ACTUATOR_FORWARD  40   // the servo goes to this position to push the switch to the off position
#define FLAP_POSITION     132   // the servo goes to this position to push the switch to the off position

                                // servo objects - no need to change these
//Servo lidServo;                 // create the lid servo object
Servo actServo;                 // create the actuator servo object

                                // switch and LED pins - no need to change these
#define SWITCH_PIN      2       // assign switch input to digital pin 2.  Switch goes between digital pin 2 and 5V. 
                                // Add a 4700 ohm resistor (yellow, violet, black, brown brown) between this pin and ground to guarantee that the pin
                                // remains in a low state when the switch is opened.
#define LED_PIN         3       // assign LED to digital pin 3.  Anode of LED goes to pin 3. Must use a resistor in series with the LED
                                // Cathode of LED goes to through a 5100 ohm resistor (green, brown, black, brown, brown)to ground.
                                // use a lower value (minimum 200 ohms) for a brighter light

int iSwitchState = 0;           // variable which holds the current switch reading
int i = 0;                      // generic loop control
int j = 0;
int imoveDetected = 0;
int iMyScore, iYourScore = 0;
int ibogusNumber = random(39,2723);   // number to display to user purporting to be calculation of next move
long liresponseTime = 0;        // determines response duration

// setup LCD display
LiquidCrystal_I2C  lcd(0x3F,2,1,0,4,5,6,7);  

// define behavior parameters
#define TEST_MODE

#ifdef TEST_MODE

  #define INACTIVITY_TIME   60                     // seconds to wait before issuing inactivity taunt    
  #define FLAP_PROB         100
  #define FLICKER_PROB      100
  #define PEEK_PROB         100
  #define MULTI_PEEK_PROB   100      
  #define LONG_WAIT_PROB    100
  #define SLOW_DEPLOY_PROB  100
  #define SCORE_BUMP_PROB   100
  #define YOU_WIN_PROB      100
  #define GO_AWAY_PROB      0
  #define SPEAK_TO_ME_PROB  100                     // Do you give up?  a bit more loudly please... just say Say yes or no... just kidding - I can't hear!
  #define I_AM_SORI_PROB    100                     // "I am Sori ... (pause) "that you are losing"
  #define SCORE_RESET_PROB  100                     // "User Response Error Your score has been reset to zero"

#else

  #define INACTIVITY_TIME   1200                    // seconds to wait before issuing inactivity taunt  
  #define FLAP_PROB         5
  #define FLICKER_PROB      5
  #define PEEK_PROB         5
  #define MULTI_PEEK_PROB   5      
  #define LONG_WAIT_PROB    5
  #define SLOW_DEPLOY_PROB  5
  #define SCORE_BUMP_PROB   5
  #define YOU_WIN_PROB      5
  #define GO_AWAY_PROB      5
  #define SPEAK_TO_ME_PROB  5
  #define I_AM_SORI_PROB    5    
  #define SCORE_RESET_PROB  0                                 

#endif




/***********************************************************************************************
 * The setup function runs only one time when the Arduino is reset or powered on
 * This is where initial values are setup.   You shouldn't have to change anything here except
 * the servo motor positions below. 
 *
 ***********************************************************************************************/
void setup() {
  actServo.attach(ACT_PIN);
  actServo.write(ACTUATOR_HOME);

  randomSeed(analogRead(0));

  // Start LCD display
  lcd.off(); 
  lcd.begin (16,2);                   // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); // set cursor to 0,0
  lcd.print("My name is Sori");
  lcd.setCursor (0,1);  
  lcd.print("Personality: ");
  lcd.print(random(42,121)); 
  lcd.on();  
  
  delay(5000);
  lcd.off();
  lcd.home (); // set cursor to 0,0
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

  // display settings on the serial monitor.  To open the serial monitor use the "Tools -> Serial Monitor" menu selection
  Serial.println("Initialization Complete, using these servo values:");
  Serial.print("Lid closed value: ");
  Serial.print(LID_CLOSED);
  Serial.print("  Lid open value: ");
  Serial.println(LID_OPEN);
  Serial.print("Actuator home value: ");
  Serial.print(ACTUATOR_HOME);
  Serial.print("  Actuator forward value: ");
  Serial.println(ACTUATOR_FORWARD);
  Serial.println("You may adjust these values in the source code to suit your needs");
  Serial.println();
  Serial.println("Monitoring switch... ");
  
  // detach servos - leaving servos detatched eliminates occasional chatter while in wait state
//  lidServo.detach();                                
  actServo.detach();

  liresponseTime = millis()/1000;
  Serial.println(liresponseTime);

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
  ibogusNumber = random(39,2723);
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
  
  // If a move has been made display current score and wait for next move
  if(imoveDetected == 1)
  {
    lcd.off();    
    lcd.clear();                              
    lcd.home(); 
    
    if(random(1,101) <= GO_AWAY_PROB)
    {                    
      // annoying behavior       
      lcd.print("I win again     ");
      lcd.setCursor (0,1);  
      lcd.print("please go away! ");
    }
    else
    {
      // normal behavior
      lcd.print("I win ");
      lcd.print(iMyScore);
      lcd.print(" - ");  
      lcd.print(iYourScore);   
      lcd.setCursor (0,1);  
      lcd.print("your move...    ");    
    }
    lcd.on();                                           // turn display on to show results
    imoveDetected = 0;                                  // reset move detection flag
  }

  if(iSwitchState == 1)                                 // Step 3 - if switch is on then turn it off
  {             
      imoveDetected = 1;
      liresponseTime = millis()/1000;                   // get current on time when switch was set
      lcd.off(); 
      lcd.clear();          
      lcd.home (); 
      lcd.print("calculating next"); 
      lcd.setCursor (0,1);        
      lcd.print("move..."); 
      lcd.print(ibogusNumber++);
      lcd.on();  
  
      delay(900);                
      // attach servos  
      actServo.attach(ACT_PIN);                           // assign servos to control pins



      // add light, temperature, humidity, background noise, bluetooth, wifi - some provide a more benevelont behavior
      // add "No Cheating" message when switch is turned off manually
      // add riddles
      // "Are you still there"?
      // auto shut off with insult
      // if switch is off unit should go back to "your move" without any other messages
      

     
      // other behaviors:  box tilt, front panel lift, box scoot, wav file ("I'm sorry Dave..."), LCD display "I win again"
      if(random(1,100) <= FLAP_PROB)
      {   
          flap(random(2,13));
      }
      
      if(random(1,100) <= PEEK_PROB) peekout(random(600,1200), random(900, 1700));
      
      if(random(1,100) <= LONG_WAIT_PROB)
      {   
        lcd.on();                                         // turn on, may have been turned off in previous processing
        //i = random(300,1200);                                 // 300ms * 1000ms = 5 minutes
        i = random(3,10);                                 // test mode
        while(j++ < i && iSwitchState == 1){
          delay(1000);                       
          iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
          digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
          lcd.setCursor (8,1);                    
          lcd.print(ibogusNumber++);
        }
      }

      if(random(1,100) <= SLOW_DEPLOY_PROB)slowDeploy();
      
      if(random(1,100) <= FLICKER_PROB)flicker(random(3,27));
      
      if(random(1,100) <= MULTI_PEEK_PROB)
      {   
        delay(random(500, 1800));  
        checkSwitchState()
        //iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
        //digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
        i = 0;
        iloopMax = random(2,5);
        while(i++ < iloopMax && iSwitchState == 1)
        {
          peekout(random(90,1200), random(600, 2100));
          checkSwitchState()
          //iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
          //digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
        }
      }
                                                                 
      // turn switch off and detach servos
      if(iSwitchState == 1) turnSwitchOff();    // user may have turned off switch while processing, deploy actuator only if switch remains on

      calculateScores();

  
  } // end of if(iSwitchState == 1)  
} // end of loop()


void checkSwitchState()
{
  
  iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
  digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
  if(iSwitchState == 0) lcd.off();



  
}

void slowDeploy(void)
{
  int i = ACTUATOR_HOME;
  lcd.off();

  while(i-- > ACTUATOR_FORWARD+13 && iSwitchState == 1)
  { 
    actServo.write(i);
    delay(100);
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)

    if(iSwitchState == 0)                               // switch may have been turned off by user
    {
      actuatorHome();
    }
  }
  delay(random(900,1800));
}


void flap(int iflapCount)
{
  int i = 0;
  lcd.off();
  while(i++ < iflapCount && iSwitchState == 1)
  {
    actServo.write(FLAP_POSITION);
    delay(80);
    actServo.write(ACTUATOR_HOME);
    delay(80);
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)

    if(iSwitchState == 0)                               // switch may have been turned off by user
    {
      actuatorHome();
    }
  }
}

void peekout(int ipeekTime, int ipeekDelay)
{
  if(iSwitchState == 1)
  {
    lcd.off();
    actServo.write(FLAP_POSITION);
    delay(ipeekTime);
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)    
    actServo.write(ACTUATOR_HOME);
    delay(ipeekDelay);
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
    
    if(iSwitchState == 0)                               // switch may have been turned off by user
    {
      actuatorHome();
    }
  }
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
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    lcd.setCursor (8,1);                    
    lcd.print(ibogusNumber++);
    digitalWrite(LED_PIN,0);                            // turn LED off
    delay(random(23,871));
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    lcd.setCursor (8,1);                    
    lcd.print(ibogusNumber++);

    if(iSwitchState == 0)                               // switch may have been turned off by user
    {
      digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
      actuatorHome();
    }
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
    Serial.print("Calling Actuator forward... ");
    actServo.write(ACTUATOR_FORWARD);                 // Step 3b - move actuator forward to turn switch off
    delay(650);                                       // give the servo time to reach its position    
                                                      // read the switch so as to immediately change the LED indicator
    iSwitchState = digitalRead(SWITCH_PIN);           // get the switch reading
    digitalWrite(LED_PIN, iSwitchState);              // set LED according to switch reading (should be off at this point)
}

void actuatorHome(void)
// called as part of turnSwitchOff or may be called independently if switch is turned off by user
{
    lcd.off();
    Serial.print("Calling Actuator Home... ");
    actServo.write(ACTUATOR_HOME);                    // move the actuator to home position (back inside the box)
    delay(650);                                       // give the servo time to reach its position
    actServo.detach();                                // detach servo
    Serial.println();
    Serial.println("Monitoring switch... ");
}

void warble(int iduration)
{
        digitalWrite(ALERT_PIN, 1);         // sonic alert on
        delay(iduration);
        digitalWrite(ALERT_PIN, 0);         // sonic alert off
}

void calculateScores()
{

  // calculate scores, of course they are random and the outcome is that the box always wins
  iMyScore += random(4,23);
  if(random(1,101) <= SCORE_BUMP_PROB)
  {                           
    iYourScore = iMyScore - 1;
  }
  else
  {
    iYourScore += random(1,3);
  }
  // maintain maximum of 3 digits for score
  if(iMyScore > 999)
  {
     iMyScore = 1;
     iYourScore = 0;
  }
  
  // display some annoying behavior
  if(random(1,101) <= YOU_WIN_PROB)
  {      
    // taunt option 1                
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
    lcd.clear();                              
    lcd.home();  
    lcd.print("I am Sori       ");
    lcd.on(); 
    delay(1500);
    lcd.setCursor (0,1);  
    lcd.print("..that you lost!");
    delay(3000);
    lcd.off();
    lcd.clear();                              
    lcd.home();  
   }

}

