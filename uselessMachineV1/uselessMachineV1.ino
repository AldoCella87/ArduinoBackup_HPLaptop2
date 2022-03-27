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
 * 2/20/17
 ***********************************************************************************************/
 
#include <Servo.h>              // this provides access to the arduino servo functions and is required in order for the code to work
#define LIDPIN  9               // digital pin 9 goes to the control line of the lid servo (white (sometimes orange) wire)
#define ACT_PIN 10              // digital pin 10 goes to the control line of the actuator servo (white (sometimes orange) wire)


// lid and actuator servo positions - adjust these as needed     
// A typical servo motor will only rotate 180 degrees.  When given a command it will rotate to that position and stay there.
// Valid servo commands are between 0 and 180.   
#define LID_CLOSED        10    // the servo starts at this position and returns to this position to close the lid
#define LID_OPEN          90    // the servo goes to this position to open the lid
#define ACTUATOR_HOME     10    // the servo starts at this position and returns to this position after turning the switch off
#define ACTUATOR_FORWARD  130   // the servo goes to this position to push the switch to the off position

                                // servo objects - no need to change these
Servo lidServo;                 // create the lid servo object
Servo actServo;                 // create the actuator servo object

                                // switch and LED pins - no need to change these
#define SWITCH_PIN      2       // assign switch input to digital pin 2.  Switch goes between digital pin 2 and 5V. 
                                // Add a 4700 ohm resistor (yellow, violet, black, brown brown) between this pin and ground to guarantee that the pin
                                // remains in a low state when the switch is opened.
#define LED_PIN         3       // assign LED to digital pin 3.  Anode of LED goes to pin 3. Must use a resistor in series with the LED
                                // Cathode of LED goes to through a 5100 ohm resistor (green, brown, black, brown, brown)to ground.
                                // use a lower value (minimum 200 ohms) for a brighter light

int iSwitchState = 0;           // variable which holds the current switch reading


/***********************************************************************************************
 * The setup function runs only one time when the Arduino is reset or powered on
 * This is where initial values are setup.   You shouldn't have to change anything here except
 * the servo motor positions below. 
 *
 ***********************************************************************************************/
void setup() {
  Serial.begin(9600);           // start the serial output stream, this enables displaying informational messages on the monitor

  pinMode(SWITCH_PIN, INPUT);   // configure switch pin as an input.  Connect a resistor between this pin and ground to force it to a low state when switch is open
  pinMode(LED_PIN, OUTPUT);     // configure LED pin as an output
 
  lidServo.attach(LIDPIN);      // assign servos to control pins
  actServo.attach(ACT_PIN);
 
  lidServo.write(LID_CLOSED);   // set servos to closed / home position
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

} // end of setup()


/***********************************************************************************************
 * The loop function runs after setup has been completed.  It will continue to run until the
 * Arduino is turned off or reset using the reset switch
 * This code simply checks to see if the switch has been turned on, sets the
 * LED according to switch state then, if switch is on, it attempts to turn it off.
 *
 ***********************************************************************************************/
void loop() {

  iSwitchState = digitalRead(SWITCH_PIN);             // Step 1 - get the switch reading
  digitalWrite(LED_PIN, iSwitchState);                // Step 2 - set LED according to switch reading (switch on turns LED on)

  if(iSwitchState == 1){                              // Step 3 - if switch is on then turn it off
    delay(700);                                       // provide time for human to move hand out of the way
    lidServo.write(LID_OPEN);                         // Step 3a - open the lid
    delay(400);                                       // give the servo time to reach its position
    actServo.write(ACTUATOR_FORWARD);                 // Step 3b - move actuator forward to turn switch off
    delay(800);                                       // give the servo time to reach its position
    actServo.write(ACTUATOR_HOME);                    // Step 3c - move the actuator to home position (back inside the box)
    delay(800);                                       // give the servo time to reach its position
    lidServo.write(LID_CLOSED);                       // Step 3d - close the lid
    delay(400);                                       // give the servo time to reach its position
  } // end of if statement

} // end of loop()

 

