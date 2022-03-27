/*
 * Cable tester for XLR
 * Al Evangelista 2/2/20 - based on logic from 2019 cable tester
 */

/*
 * TODO:  
 * Add one touch on/automatic off functionality via FET
 * Add bt interface with MIT App Inventor
 * single button interface
 *  Push - ON and display welcome message
 *  Push - explanation (run continuously until next cycle (within timout parameter)
 *  Push - perform open circuit test
 *  Push - perform short circuit test
 *  Push - display results
 *  Push - off
 *  
 *  Note:  if continuity test is good for all 3 conductors do not distinguish between mic end and mixer end short test
 *  or state that short exists between pins X and Y on EITHER the mic or mixer end, else state which end (Mic or Mixer) on which the 
 *  short is found
 *  
 *  test for cross-wiring (e.g. pin 1 wired to either pin 2 or 3)
 *  
 *  detect cases where intermittent connection exists (some pass and some fail for a given conductor)
 *  
 *  add shield test (if shield is not connected to correct terminal then other terminals will have noise during analog read)
 *  if continuity test fails attempt to locate which end the break is located based on antenna effect using analog read
 *  
 *  test for high resisistance (over 10 ohms for example) during continuity test?
 *  
 *  create explanation of tests and display them (or display via serial port / bluetooth?)  "for more information connect via BT..."
 *  
 *  Show shield conductor status however due to variety of application of this pin the results should not participate 
 *  in pass/fail status
 *  
 *  or...  have a testing phase followed by a display results phase.  
 *  during test phast test BOTH connectors for shorts as well as continuity for each of 3 pins
*/

#include "Arduino.h"

// digital pins assigned to XLR connectors 
// connectors are uniquely identified as either microphone side (MIC) or mixer side (MXR)
#define MIC_GND       2     // green
#define MIC_HOT       3     // red
#define MIC_NEUTRAL   4     // white
#define MIC_SHELL     5     // black
#define MXR_GND       6     // green
#define MXR_HOT       7     // red
#define MXR_NEUTRAL   8     // white
#define MXR_SHELL     9     // black

// defines for sonic alert
#define SOUND_CLICK             0
#define SOUND_ERROR             1
#define SOUND_TICK              2

#define ALERT_PIN               10
#define ISOLATOR_ANODE          11
#define ISOLATOR_CATH           12

// setup LCD display
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 

const int iVersion = 3;

bool bContinuityError = false;
bool bShortCircuitError = false;

// continuity results
long lGndErrorCount = 0;
long lHotErrorCount = 0;
long lNeutralErrorCount = 0;
long lShellErrorCount = 0;

// short circuit results
long lMIC12ShortCount = 0;        // count of shorts detected between pins 1 and 2
long lMIC23ShortCount = 0;        // count of shorts detected between pins 2 and 3
long lMIC13ShortCount = 0;        // count of shorts detected between pins 1 and 3
long lMIC2SShortCount = 0;        // count of shorts detected between pin 2 and shell
long lMIC3SShortCount = 0;        // count of shorts detected between pin 3 and shell
long lMIC1SShortCount = 0;        // count of shorts detected between pin 1 and shell        
                                  // special note on this condition: pin 1 is ground and shell is also ground, 
                                  // a short between these pins is for informational purposes only and does not
                                  // necessarily represent a fault.  

long lMXR12ShortCount = 0;        // count of shorts detected between pins 1 and 2
long lMXR23ShortCount = 0;        // count of shorts detected between pins 2 and 3
long lMXR13ShortCount = 0;        // count of shorts detected between pins 1 and 3
long lMXR2SShortCount = 0;        // count of shorts detected between pin 2 and shell
long lMXR3SShortCount = 0;        // count of shorts detected between pin 3 and shell
long lMXR1SShortCount = 0;        // count of shorts detected between pin 1 and shell        
                                  // special note on this condition: pin 1 is ground and shell is also ground, 
                                  // a short between these pins is for informational purposes only and does not
                                  // necessarily represent a fault. 

// cross connect results.  These variables hold which pins on hte MXR side are mapped to MIC pins 1,2,3 and shield

int iMIC1 = 0;                     // pin on MXR cable which corresponds to MIC pin 1
int iMIC2 = 0;                     // pin on MXR cable which corresponds to MIC pin 2
int iMIC3 = 0;                     // pin on MXR cable which corresponds to MIC pin 3
int iMICS = 0;                     // pin on MXR cable which corresponds to MIC shield

long  lStartTime = 0;              // for pushbutton on / auto power off
long  lOnDuration = 20000;           // duration in miliseconds before power off

/*  Momentary push to power on
 *   using IRFZ44N n channel mosfet to implement power on with a single momentary pushbutton.  Power off after x time
 *   
 *   Mosfet:
 *      Gate:  N.O. Pushbutton through 10K resistor to Vcc to activate
 *      Gate:  330K resistor in parallel with 2.2uf capacitor to Vss
 *      Gate:  1.1K from emitter of H11 optoisolator
 *      Drain: to Arduino ground (to switch negative side of power to negative side of battery)
 *   Battery:  
 *      Source: to negative side of battery (Vss)
 *      Positive side of battery: to Arduino Vin pin and to one side of pushbutton switch
 *   H11 optoisolator:
 *      Both sides of LED are driven from Arduino, there is no electrical connection between the input side 
 *      of the isolator and the battery.
 *      Anode: to Arduino digital output ISOLATOR_ANODE
 *      Cathode: to Arduino digital output ISOLATOR_CATH through a 220 ohm resistor
 *      Collector: to Vdd
 *      Emitter: to gate of MOSFET via 1.1K resistor
 *      
 *      
 *      total system current draw is about 100 ma
 *   
 */

                                  
void setup() {


  // pinmode for sonalert pin
  pinMode(ALERT_PIN, OUTPUT);
  pinMode(ISOLATOR_ANODE, OUTPUT);                  // for MOSFET pushbutton power on control,  turns off after x minutes, drive through opto isolator
  pinMode(ISOLATOR_CATH, OUTPUT);                   // for MOSFET pushbutton power on control,  turns off after x minutes, drive through opto isolator

  digitalWrite(ISOLATOR_ANODE, HIGH);               // Anode of opto isolator to trigger MOSFET on
  digitalWrite(ISOLATOR_CATH, LOW);                 // Cathode of opto isolator.  Driving both sides of LED via Arduino prevents powering up the arduino
                                                    // from the digital pins.  Using 220 ohm resistor with H11 optoisolator gives 14ma of LED current
  SonalertControl(SOUND_CLICK);                     // signify power on (OK to release start button)


  
     
  lStartTime = millis();

  Serial.begin(9600);

  // LCD display setup
  //lcd.off(); 
  lcd.begin (20,4);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  
  DisplayWelcomeMsg();
  delay(3000);  // replace with "press button to continue"
}

void loop() {


  // initialize variables


  bContinuityError = false;
  bShortCircuitError = false;

  // Initialize continuity variables
  lGndErrorCount = 0;
  lHotErrorCount = 0;
  lNeutralErrorCount = 0;
  lShellErrorCount = 0;

  // Initialize short circuit variables
  lMIC12ShortCount = 0;        // count of shorts detected between pins 1 and 2
  lMIC23ShortCount = 0;        // count of shorts detected between pins 2 and 3
  lMIC13ShortCount = 0;        // count of shorts detected between pins 1 and 3
  lMIC2SShortCount = 0;        // count of shorts detected between pin 2 and shell
  lMIC3SShortCount = 0;        // count of shorts detected between pin 3 and shell
  lMIC1SShortCount = 0;        // count of shorts detected between pin 1 and shell        
                               // special note on this condition: pin 1 is ground and shell is also ground, 
                               // a short between these pins is for informational purposes only and does not
                               // necessarily represent a fault.  

  lMXR12ShortCount = 0;        // count of shorts detected between pins 1 and 2
  lMXR23ShortCount = 0;        // count of shorts detected between pins 2 and 3
  lMXR13ShortCount = 0;        // count of shorts detected between pins 1 and 3
  lMXR2SShortCount = 0;        // count of shorts detected between pin 2 and shell
  lMXR3SShortCount = 0;        // count of shorts detected between pin 3 and shell
  lMXR1SShortCount = 0;        // count of shorts detected between pin 1 and shell        
                               // special note on this condition: pin 1 is ground and shell is also ground, 
                               // a short between these pins is for informational purposes only and does not
                               // necessarily represent a fault. 

  // Initialize cross connect results.  These variables hold which pins on hte MXR side are mapped to MIC pins 1,2,3 and shield

  iMIC1 = 0;                     // pin on MXR cable which corresponds to MIC pin 1
  iMIC2 = 0;                     // pin on MXR cable which corresponds to MIC pin 2
  iMIC3 = 0;                     // pin on MXR cable which corresponds to MIC pin 3
  iMICS = 0;                     // pin on MXR cable which corresponds to MIC shield


  // conduct tests
  ConductTest(millis());

  // display results
  DisplaySummary();
  delay(4000);
  
  DisplayContinuityResult();
  delay(4000);  // replace with wait for button press...

  DisplayShortResultMic();
  delay(4000);
 
  DisplayShortResultMxr(); 
  delay(4000);

  DisplayShutdown();
  delay(4000);
  
  //if(millis() > lStartTime + lOnDuration) digitalWrite(ISOLATOR_ANODE, LOW);

  //shutdown
  digitalWrite(ISOLATOR_ANODE, LOW);
  delay(4000);


  // execute test again (remove with addition of user controls)
 // swReset();    
  
  /************** not implemented...
  DisplayCrossConnectResult();
  delay(4000);
  DisplayShellStatus(); 
  delay(4000);
  DisplayCapacitanceResult(167);
  delay(4000);
  *****************/
}


/*
 * Conduct Test
 * components:  
 *  A. continuity for conductors 1,2 and 3 - this is executed in a loop to test repeatedly over a specified period of time
 *  B. short circuit test for both MIC and MXR ends - this is executed in a loop to test repeatedly over a specified period of time
 *  C. shell status(connected to pin 1 or not, continuous or not...)
 *  D. overall capaticatance (perhaps relative capacitance) (not implemented as of 6/21)
 *  E. Cross connect test - this is executed only once
 */
void ConductTest(unsigned long ulStartTime)  
{
  int iTestDuration = 15000;            
  unsigned long lPreviousDot = 0;

  // initialize status flags
  bContinuityError = false;
  bShortCircuitError = false;
  
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Testing ... "));
  lcd.setCursor (0,1);        
  lcd.print(F("Flex the cable at"));
  lcd.setCursor (0,2);        
  lcd.print(F("at each connector"));
  lcd.setCursor (0,3);        

  // setup test - all pins should remain in hi z state except the output
  // pin which is providing input for testing and the input pin which is reading the result. 
  pinMode(MIC_GND, INPUT);
  pinMode(MIC_HOT, INPUT);
  pinMode(MIC_NEUTRAL, INPUT);
  pinMode(MIC_SHELL, INPUT);
  pinMode(MXR_GND, INPUT);
  pinMode(MXR_HOT, INPUT);
  pinMode(MXR_NEUTRAL, INPUT);
  pinMode(MXR_SHELL, INPUT);

  // monitor cable for specified time while user manipulates the cable and connectors attempting to reveal
  // an intermittent connection,  This loop covers the continuity and short circuit tests
  while (millis() < ulStartTime + iTestDuration)
  {
    if(millis() >lPreviousDot + (iTestDuration / 20))     // display a line of 20 dots along the bottom of screen to show progress
    {
       lcd.print(F("."));
       lPreviousDot = millis();
    }

    // Section A - continuty
    // test continuity on all three pins

    // A.1 Test GND connection for continuity
    // Setup digital inputs/outputs
    pinMode(MIC_GND, OUTPUT);                                       // only set pin on input side to an outupt
    pinMode(MXR_GND, INPUT_PULLUP);                                 // set corresponding output side as input with pullup
    
    // look for signal on the opposite connector and accumulate errors
    digitalWrite(MIC_GND, LOW);                                     // set a low logic level on input side
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds.
    if (digitalRead(MXR_GND) == HIGH) lGndErrorCount ++;            // Output side should be LOW, if HIGH then error
    
    // return digital pins to high z state
    pinMode(MIC_GND, INPUT);                                         // return input side to high z state
    pinMode(MXR_GND, INPUT);                                         // return output side to high z state

    
    // A.2 Test HOT connection for continuity
    // Setup digital inputs/outputs
    pinMode(MIC_HOT, OUTPUT);                                       // only set pin on input side to an outupt
    pinMode(MXR_HOT, INPUT_PULLUP);                                 // set corresponding output side as input with pullup
    
    // conduct tests
    digitalWrite(MIC_HOT, LOW);                                     // set a low logic level on input side
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds.
    if (digitalRead(MXR_HOT) == HIGH) lHotErrorCount ++;            // Output side should be LOW, if HIGH then error
    
    // return digital pins to high z state
    pinMode(MIC_HOT, INPUT);                                         // return input side to high z state
    pinMode(MXR_HOT, INPUT);                                         // return output side to high z state

    // A.3 Test NEUTRAL connection for continuity
    // Setup digital inputs/outputs
    pinMode(MIC_NEUTRAL, OUTPUT);                                    // only set pin on input side to an outupt
    pinMode(MXR_NEUTRAL, INPUT_PULLUP);                              // set corresponding output side as input with pullup
    
    // look for signal on the opposite connector and accumulate errors
    digitalWrite(MIC_NEUTRAL, LOW);                                  // set a low logic level on input side
    delayMicroseconds(100);                                          // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                     // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                     // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                     // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                     // use 100 microseconds.
    if (digitalRead(MXR_NEUTRAL) == HIGH) lNeutralErrorCount ++;     // Output side should be LOW, if HIGH then error
    
    // return digital pins to high z state
    pinMode(MIC_NEUTRAL, INPUT);                                      // return input side to high z state
    pinMode(MXR_NEUTRAL, INPUT);                                      // return output side to high z state

    // A.4 Test SHELL connection for continuity - note this is for informational purposes, not pass/fail
    // Setup digital inputs/outputs
    pinMode(MIC_SHELL, OUTPUT);                                       // only set pin on input side to an outupt
    pinMode(MXR_SHELL, INPUT_PULLUP);                                 // set corresponding output side as input with pullup
    
    // look for signal on the opposite connector and accumulate errors
    digitalWrite(MIC_SHELL, LOW);                                   // set a low logic level on input side
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds.
    
    if (digitalRead(MXR_SHELL) == HIGH) lShellErrorCount ++;          // Output side should be LOW, if HIGH then error
    
    // return digital pins to high z state
    pinMode(MIC_SHELL, INPUT);                                        // return input side to high z state
    pinMode(MXR_SHELL, INPUT);                                        // return output side to high z state


    // Section B - short circuit test
    // use caution when drawing conclusions about where a short circuit is physically located.  If continuity is good then the short may be physically on 
    // the remote end of the cable.  This test simply states where the short appears electrically.
    // set digital output ONCE for all tests for this pin then reset to high z after all tests have been completed

    // B.1 Check for short between all pins on MIC side connector
    // B.1.1 Check for short between MIC_GND and (MIC_HOT, MIC_NEUTRAL and MIC_SHELL)
    // B.1.1.0 setup test
    pinMode(MIC_GND, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MIC_GND, LOW);                                     // set a low logic level on input side
    pinMode(MIC_HOT, INPUT_PULLUP);                                 // set subject being tested as input with pullup
    pinMode(MIC_NEUTRAL, INPUT_PULLUP);                             // set subject being tested as input with pullup
    pinMode(MIC_SHELL, INPUT_PULLUP);                               // set subject being tested as input with pullup
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MIC_HOT) == LOW) lMIC12ShortCount ++;           // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MIC_NEUTRAL) == LOW) lMIC13ShortCount ++;       // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MIC_SHELL) == LOW) lMIC1SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MIC_GND, INPUT);                                        // done testing with MIC_GND, return input side to high z state
    
    // B.1.2 Check for short between MIC_HOT and (MIC_NEUTRAL and MIC_SHELL) (fewer tests required here because some conditions were covered above)
    // B.1.2.0 setup test
    pinMode(MIC_HOT, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MIC_HOT, LOW);                                     // set a low logic level on input side for all tests in this section
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MIC_NEUTRAL) == LOW) lMIC23ShortCount ++;       // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MIC_SHELL) == LOW) lMIC2SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MIC_HOT, INPUT);                                        // done testing with MIC_HOT, return input side to high z state

    // B.1.3 Check for short between MIC_NEUTRAL and MIC_SHELL (fewer tests required here because some conditions were covered above)
    // B.1.3.0 setup test
    pinMode(MIC_NEUTRAL, OUTPUT);                                   // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MIC_NEUTRAL, LOW);                                 // set a low logic level on input side for all tests in this section
    pinMode(MIC_SHELL, INPUT_PULLUP);                               // set subject being tested as input with pullup
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MIC_SHELL) == LOW) lMIC3SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MIC_NEUTRAL, INPUT);                                    // done testing with MIC_NEUTRAL, return input side to high z state

    pinMode(MIC_SHELL, INPUT);                                      // done testing with MIC_SHELL, return input side to high z state
                                                                    // at this point all 4 pins on MIC connector are back to hi z state


    // B.2 Check for short between all pins on MXR side connector
    // B.2.1 Check for short between MXR_GND and (MXR_HOT, MXR_NEUTRAL and MXR_SHELL)
    // B.2.1.0 setup test
    pinMode(MXR_GND, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MXR_GND, LOW);                                     // set a low logic level on input side
    pinMode(MXR_HOT, INPUT_PULLUP);                                 // set subject being tested as input with pullup
    pinMode(MXR_NEUTRAL, INPUT_PULLUP);                             // set subject being tested as input with pullup
    pinMode(MXR_SHELL, INPUT_PULLUP);                               // set subject being tested as input with pullup
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MXR_HOT) == LOW) lMXR12ShortCount ++;           // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MXR_NEUTRAL) == LOW) lMXR13ShortCount ++;       // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MXR_SHELL) == LOW) lMXR1SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MXR_GND, INPUT);                                        // done testing with MXR_GND, return input side to high z state
    
    // B.2.2 Check for short between MXR_HOT and (MXR_NEUTRAL and MXR_SHELL) (fewer tests required here because some conditions were covered above)
    // B.2.2.0 setup test
    pinMode(MXR_HOT, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MXR_HOT, LOW);                                     // set a low logic level on input side for all tests in this section
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds.    
    if (digitalRead(MXR_NEUTRAL) == LOW) lMXR23ShortCount ++;       // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MXR_SHELL) == LOW) lMXR2SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MXR_HOT, INPUT);                                        // done testing with MXR_HOT, return input side to high z state

    // B.2.3 Check for short between MXR_NEUTRAL and MXR_SHELL (fewer tests required here because some conditions were covered above)
    // B.2.3.0 setup test
    pinMode(MXR_NEUTRAL, OUTPUT);                                   // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MXR_NEUTRAL, LOW);                                 // set a low logic level on input side for all tests in this section
    pinMode(MXR_SHELL, INPUT_PULLUP);                               // set subject being tested as input with pullup
    delayMicroseconds(100);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MXR_SHELL) == LOW) lMXR3SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MXR_NEUTRAL, INPUT);                                    // done testing with MXR_NEUTRAL, return input side to high z state

    pinMode(MXR_SHELL, INPUT);                                      // done testing with MXR_SHELL, return input side to high z state
                                                                    // at this point all 4 pins on MXR connector are back to hi z state    

    // Section C - assess status of shell connection
    // use of shell connection varies, results will be presented as informational rather than pass/fail
    // shell may be connected to ground (pin 1) or it may be continuous from MIC to MXR ends without being connected to pin 1
    // use information gathered above to provide information to user, no further tests required here
    // A positive value in lMXR1SShortCount indicates that the shell is connected to the ground pin on either the mixer side or microphone side 
    // A positive value in lMIC1SShortCount indicates that the shell is connected to the ground pin on either the mixer side or microphone side 
    // lShellErrorCount value of zero indicates continuity between shell connections on both ends of cable

  }  // end of loop for specified time period

  // set status flags once here rather than multiple times in loop above
  // note we do not test lShellErrorCount as this count is informational and does not
  // necessarily signify an error
  if( lGndErrorCount > 0 ||
      lHotErrorCount > 0 ||
      lNeutralErrorCount > 0){
        bContinuityError = true; 
      }
  
  if( lMIC12ShortCount > 0 ||
      lMIC23ShortCount > 0 ||
      lMIC13ShortCount > 0 ||
      lMIC2SShortCount > 0 ||
      lMIC3SShortCount > 0 ||
      lMIC1SShortCount > 0 ||
      lMXR12ShortCount > 0 ||
      lMXR23ShortCount > 0 ||
      lMXR13ShortCount > 0 ||
      lMXR2SShortCount > 0 ||
      lMXR3SShortCount > 0 ||
      lMXR1SShortCount > 0){
        bShortCircuitError = true; 
      }

} 


/*********************************************************************
 * display welcome message
 * 
 *********************************************************************/
void DisplayWelcomeMsg(void)
{
    //  Row 1
    lcd.setCursor (0,0);        
    lcd.print(F("Cable Tester V. "));
    lcd.print(iVersion);
    SonalertControl(SOUND_CLICK);  
}

/*
 * Display Results for continuity test
 */
void DisplayContinuityResult() 
{
  bool bErrorDetected = false;
  
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Header
  lcd.print(F("Continuity Test"));

  // Display data for pin 1 (GND)
  lcd.setCursor (0,1);        
  lcd.print(F(" Pin 1")); 
  lcd.setCursor (8,1); 
  if ( lGndErrorCount > 0){
     lcd.print(F("FAIL "));
     lcd.print(lGndErrorCount);
     bErrorDetected = true;
  }
  else{
    lcd.print(F("Pass"));  
  }

  // Display data for pin 2 (HOT)
  lcd.setCursor (0,2);        
  lcd.print(F(" Pin 2")); 
  lcd.setCursor (8,2); 
  if ( lHotErrorCount > 0){
     lcd.print(F("FAIL "));
     lcd.print(lHotErrorCount);
     bErrorDetected = true;
  }
  else{
    lcd.print(F("Pass"));  
  }

    // Display data for pin 3 (NEUTRAL)
  lcd.setCursor (0,3);        
  lcd.print(F(" Pin 3")); 
  lcd.setCursor (8,3); 
  if ( lNeutralErrorCount > 0){
     lcd.print(F("FAIL "));
     lcd.print(lNeutralErrorCount);
     bErrorDetected = true;
  }
  else{
    lcd.print(F("Pass"));  
  }
  //disable for testing...  if(bErrorDetected) SonalertControl(SOUND_ERROR);
} 


/*
 * Display Results for cross connection test
 * using global variables iMIC1, iMIC2 and iMIC3 to signify which pins on MXR side pins 1,2 and 3 are mapped to from the MIC side.
 * normal condition is 1,2,3 however cable may be incorrectly wired.   this test will reveal that condution
 */
void DisplayCrossConnectResult(void)
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  

  // Display Row 1
  lcd.print(F("Cross-connect Test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" MIC1 -> MXR")); 
  lcd.setCursor (12,1);
  lcd.print(iMIC1);
  if(iMIC1 == 1){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }


  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" MIC2 -> MXR")); 
  lcd.setCursor (12,2);
  lcd.print(iMIC2);
  if(iMIC2 == 2){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" MIC3 -> MXR")); 
  lcd.setCursor (12,3);
  lcd.print(iMIC3);
  if(iMIC3 == 3){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }
  
  //lcd.on();

} 

/*
 * Display Results for short circuit test for microphone end of cable
 */
void DisplayShortResultMic(void)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Mic end short test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" P1-2:")); 
  lcd.setCursor (9,1); 
  if(lMIC12ShortCount >0)
  {
    lcd.print(F("FAIL "));
    lcd.print(lMIC12ShortCount);
  }
  else{
     lcd.print(F("Pass"));   
  }

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" P2-3:")); 
  lcd.setCursor (9,2);  
  if(lMIC23ShortCount >0)
  {
    lcd.print(F("FAIL "));
    lcd.print(lMIC23ShortCount);
  }
  else{
     lcd.print(F("Pass")); 
  }

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" P1-3:")); 
  lcd.setCursor (9,3);  
  if(lMIC13ShortCount >0)
  {
    lcd.print(F("FAIL "));
    lcd.print(lMIC13ShortCount);
  }
  else{
     lcd.print(F("Pass")); 
  }
  
  //lcd.on();
} 


/*
 * Display Results for short circuit test for mixer end of cable
 */
void DisplayShortResultMxr(void)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Mixer end short test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" P1-2:")); 
  lcd.setCursor (9,1); 
  if(lMXR12ShortCount >0)
  {
    lcd.print(F("FAIL "));
    lcd.print(lMXR12ShortCount);
  }
  else{
     lcd.print(F("Pass"));   
  }

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" P2-3:")); 
  lcd.setCursor (9,2);  
  if(lMXR23ShortCount >0)
  {
    lcd.print(F("FAIL "));
    lcd.print(lMXR23ShortCount);
  }
  else{
     lcd.print(F("Pass")); 
  }

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" P1-3:")); 
  lcd.setCursor (9,3);  
  if(lMXR13ShortCount >0)
  {
    lcd.print(F("FAIL "));
    lcd.print(lMXR13ShortCount);
  }
  else{
     lcd.print(F("Pass")); 
  }
  
  //lcd.on();

} 



/*
 * Display shutdown message
 */
void DisplayShutdown(void)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Test Complete"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" ... goodbye...")); 

}


/*
 * Display findings for shell connectivity, note
 * this is presented as informational and does not
 * participate in pass/fail results
 */
void DisplayShellStatus(void)  
{
  // A positive value in lMXR1SShortCount indicates that the shell is connected to the ground pin on either the mixer side or microphone side 
  // A positive value in lMIC1SShortCount indicates that the shell is connected to the ground pin on either the mixer side or microphone side 
  // lShellErrorCount value of zero indicates continuity between shell connections on both ends of cable
  
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Shell Status (info)"));

  // Display Row 2
  lcd.setCursor (0,1); 
  if(lShellErrorCount == 0){
    lcd.print(F(" Shell is connected")); 
  }
  else{
     lcd.print(F(" Shell not connected"));    
  }

  // Display Row 3
  lcd.setCursor (0,2); 
  if((lMXR1SShortCount+lMIC1SShortCount) > 0){
    lcd.print(F(" Shell tied to P1")); 
  }
  else{
     lcd.print(F("Shell not tied to P1"));    
  }

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F("(Informational only)")); 

  
  //lcd.on();

} 



/*
 * Display Results for cable capacitance test
 */
void DisplayCapacitanceResult(int iResult)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Capacitance Test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" Values < 500pf best")); 
  lcd.setCursor (14,1);  


  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F("Test Value: ")); 
  lcd.setCursor (14,2);  
  lcd.print(iResult);
  lcd.print(F("pf"));

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F("")); 
  lcd.setCursor (14,3);  
  lcd.print(F(""));
  
  //lcd.on();

} 


void DisplaySummary(void)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  if (bContinuityError == true ||
      bShortCircuitError == true){
        lcd.print(F("Cable needs service"));
        SonalertControl(SOUND_ERROR);             
      }
      else{
        lcd.print(F("Cable is OK"));         
      }

   lcd.setCursor (0,2);
   lcd.print(F("details follow..")); 
}

/*
 * controls sonic alert
 * input parameter is integer representing the type of sound desired
 */
void SonalertControl(int iMode)
{
  switch(iMode)
  {
    case SOUND_CLICK:                       // brief click sound for positive feedback of keypress
      digitalWrite(ALERT_PIN, HIGH);
      delay(2);
      digitalWrite(ALERT_PIN, LOW);
      break;

      case SOUND_TICK:                    
      digitalWrite(ALERT_PIN, HIGH);
      delay(6);
      digitalWrite(ALERT_PIN, LOW);
      break;
      
    case SOUND_ERROR:                      // signify an error has been detected
      digitalWrite(ALERT_PIN, HIGH);
      delay(15);
      digitalWrite(ALERT_PIN, LOW);
      delay(5);
      digitalWrite(ALERT_PIN, HIGH);
      delay(15);
      digitalWrite(ALERT_PIN, LOW);
      delay(5);
      digitalWrite(ALERT_PIN, HIGH);
      delay(15);
      digitalWrite(ALERT_PIN, LOW);      
      break;  
         
    default:
      break;
  }
}

void swReset(void) // Restarts program from beginning but does not reset the peripherals and registers
{
  lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Resetting...        ")); 
  lcd.on();
  delay(1000); // or wait for button press to reset
  lcd.clear();
  asm volatile ("  jmp 0");         // boom boom... out go the lights
} 



 
