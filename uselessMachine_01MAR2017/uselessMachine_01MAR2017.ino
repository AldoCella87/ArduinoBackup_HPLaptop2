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
 ***********************************************************************************************/
 
#include <Servo.h>              // this provides access to the arduino servo functions and is required in order for the code to work
#define LIDPIN  9               // digital pin 9 goes to the control line of the lid servo (white (sometimes orange) wire)
#define ACT_PIN 10              // digital pin 10 goes to the control line of the actuator servo (white (sometimes orange) wire)


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


/***********************************************************************************************
 * The setup function runs only one time when the Arduino is reset or powered on
 * This is where initial values are setup.   You shouldn't have to change anything here except
 * the servo motor positions below. 
 *
 ***********************************************************************************************/
void setup() {
  Serial.begin(9600);           // start the serial output stream, this enables displaying informational messages on the monitor

  pinMode(SWITCH_PIN, INPUT);   // configure switch pin as an input.  Connect a resistor between this pin and ground to force it to a low state when switch is open
  pinMode(LED_PIN, OUTPUT);     // configure LED pin as an output, this is for an external LED indicator to accompany the switch
  pinMode(13, OUTPUT);          // configure pin 13 which has an on-board LED, useful for diagnostics
 
//  lidServo.attach(LIDPIN);      // assign servos to control pins
  actServo.attach(ACT_PIN);
 
 // lidServo.write(LID_CLOSED);   // set servos to closed / home position
  actServo.write(ACTUATOR_HOME);

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
  iSwitchState = digitalRead(SWITCH_PIN);             // Step 1 - get the switch reading
  digitalWrite(LED_PIN, iSwitchState);                // Step 2 - set LED according to switch reading (switch on turns LED on)
  //digitalWrite(13, iSwitchState);                     // also set the user assignable LED which resides on the Arduino board
  if(iSwitchState == 1)                                 // Step 3 - if switch is on then turn it off
  {              
      delay(200);                
      // attach servos  
      actServo.attach(ACT_PIN);                           // assign servos to control pins
      // define behavior probabilities
      #define FLAP_PROBAB         3
      #define FLICKER_PROBAB      3
      #define PEEK_PROBAB         3
      #define MULTI_PEEK_PROBAB   3      
      #define LONG_WAIT_PROBAB    3
      #define SLOW_DEPLOY_PROBAB  3

     
      // other behaviors:  box tilt, front panel lift, box scoot, wav file ("I'm sorry Dave..."), LCD display "I win again"

      if(random(1,100) <= FLAP_PROBAB)
      {   
          flap(random(2,13));
      }
      
      if(random(1,100) <= PEEK_PROBAB) peekout(random(600,1200), random(900, 1700));
      
      if(random(1,100) <= LONG_WAIT_PROBAB)
      {   
          delay(random(600000,1200000));                      // delay 10 to 20 minutes
          iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
          digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
      }

      if(random(1,100) <= SLOW_DEPLOY_PROBAB)slowDeploy();
      
      if(random(1,100) <= FLICKER_PROBAB)flicker(random(3,27));
      
      if(random(1,100) <= MULTI_PEEK_PROBAB)
      {   
        delay(random(500, 1800));  
        iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
        digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
        i = 0;
        iloopMax = random(2,5);
        while(i++ < iloopMax && iSwitchState == 1)
        {
          peekout(random(90,1200), random(600, 2100));
          iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
          digitalWrite(LED_PIN, iSwitchState);                // set LED according to switch reading (switch on turns LED on)
        }
      }
                                                                 
      // turn switch off and detach servos
      if(iSwitchState == 1) turnSwitchOff();    // user may have turned off switch while processing, deploy actuator only if switch remains on
  } // end of if(iSwitchState == 1)  
} // end of loop()

void slowDeploy(void)
{
  int i = ACTUATOR_HOME;

  while(i-- > ACTUATOR_FORWARD+10 && iSwitchState == 1)
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
  int i = 0;
  int j = 0;
  while(i++ < iflickerCount && iSwitchState == 1)
  {
    digitalWrite(LED_PIN,1);                            // turn LED off
    delay(random(23,871));
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    digitalWrite(LED_PIN,0);                            // turn LED off
    delay(random(23,871));
    iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly

/*****
    while(j+=10 < 255 && iSwitchState == 1){
      analogWrite(LED_PIN, j);
      delay(80);
      iSwitchState = digitalRead(SWITCH_PIN);             // user may have changed switch setting, re-read and set LED accordingly
    }
*****/

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
    Serial.print("Calling Actuator Home... ");
    actServo.write(ACTUATOR_HOME);                    // move the actuator to home position (back inside the box)
    delay(650);                                       // give the servo time to reach its position
    actServo.detach();                                // detach servo
    Serial.println();
    Serial.println("Monitoring switch... ");
}




