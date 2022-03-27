// a fork of cable tester intended to investigate this behavior:
//  when testing an XLR connector for short circuit between conductors I bring one pin at a time LOW then look for a corresponding LOW state on each of the 
// remaining pins in the same connector.  
// when bringing pin 1 low and testing for a short on pin 2 even though there is no electrical connection between pins 1 and 2 pin 2 will be low very briefly 
// even though the pin is not directly connected to pin 1.  
// This situation manifests itself to a greater degree with longer cables (the time pin 2 remains low is longer with longer cable).  I theorize that this effect is due
// to the capacitave coupling between conductors (about 15pf per foot I believe).  with no cable connected the effect is not evident.  With 20 foot cable it takes
// about 10 microseconds for the low logic level to be considered a high level on the unconnected pin.  With 40 foot cable it takes about 25 microseconds.  
//  the purpose of this code is to make a test bed to further investigate this phenomenon.  Currently the lines are not terminated in any resistance so perhaps
//  a terminating resistor at each end will help mitigate this situation.  
//  Example - see comment around delayMicroseconds(100) below

/*********     
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

 * 
 ********/

 

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


// defines for sonic alert
#define SOUND_CLICK             0
#define SOUND_ERROR             1
#define SOUND_TICK              2

// setup LCD display
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 

const int alertPin = 10;          // sonic alert pin

// short circuit results
long lMIC12ShortCount = 0;        // count of shorts detected between pins 1 and 2
long lMIC13ShortCount = 0;        // count of shorts detected between pins 1 and 3

                                  
void setup() {
  Serial.begin(9600);

  // LCD display setup
  //lcd.off(); 
  lcd.begin (20,4);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);

  // pinmode for sonalert pin
  pinMode(alertPin, OUTPUT);

  DisplayWelcomeMsg();
  delay(1000);  // replace with "press button to continue"
}

void loop() {
  int iTestResult = 0;

  iTestResult = ConductTest(millis());

  // display results
  lcd.clear();          
  lcd.home (); 

  DisplayShortResultMic();
  delay(5000);

  // execute test again (remove with addition of user controls)
  swReset();    
}

/*
 * Conduct Test
 */
ConductTest(unsigned long ulStartTime)  
{
  int iTestDuration = 2000;            
  unsigned long lPreviousDot = 0;
  
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Testing ... "));
  lcd.setCursor (0,2);              
    
  // Section A - continuty
  // test continuity on all three pins

  // A.0 setup test - all pins should remain in hi z state except the output
  // pin which is providing input for testing and the input pin which is reading the result. 
  pinMode(MIC_GND, INPUT);
  pinMode(MIC_HOT, INPUT);
  pinMode(MIC_NEUTRAL, INPUT);
  pinMode(MIC_SHELL, INPUT);


  // monitor cable for specified time  
  while (millis() < ulStartTime + iTestDuration)
  {
    if(millis() >lPreviousDot + (iTestDuration / 20))     // display a line of 20 dots along the bottom of screen to show progress
    {
       lcd.print(F("."));
       lPreviousDot = millis();
    }

 
    // demonstrate and measure capacitave coupling between conductors of XLR cable
 
    // B.1 Check for short between all pins on MIC side connector
    // B.1.1 Check for short between MIC_GND and (MIC_HOT, MIC_NEUTRAL and MIC_SHELL)
    // B.1.1.0 setup test
    pinMode(MIC_GND, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MIC_GND, LOW);                                     // set a low logic level on input side
    pinMode(MIC_HOT, INPUT_PULLUP);                                 // set subject being tested as input with pullup
    pinMode(MIC_NEUTRAL, INPUT_PULLUP);                             // set subject being tested as input with pullup
    pinMode(MIC_SHELL, INPUT_PULLUP);                               // set subject being tested as input with pullup
   // delayMicroseconds(1);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MIC_HOT) == LOW) lMIC12ShortCount ++;           // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MIC_NEUTRAL) == LOW) lMIC13ShortCount ++;       // For short circuit test the expected result is HIGH (no connection)
    pinMode(MIC_GND, INPUT);                                        // done testing with MIC_GND, return input side to high z state
    


  }  // end of loop for specified time period

} 


/*********************************************************************
 * display welcome message
 * 
 *********************************************************************/
void DisplayWelcomeMsg(void)
{
    //  Row 1
    lcd.setCursor (0,0);        
    lcd.print(F("Test capacitave coupling between transmission lines"));
  
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
  lcd.print(F("pin   cnt of samples"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" 2:")); 
  lcd.setCursor (11,1); 
  lcd.print(lMIC12ShortCount);

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" 3:")); 
  lcd.setCursor (11,2);  
  lcd.print(lMIC13ShortCount);

  //lcd.on();
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
      digitalWrite(alertPin, HIGH);
      delay(2);
      digitalWrite(alertPin, LOW);
      break;

      case SOUND_TICK:                    
      digitalWrite(alertPin, HIGH);
      delay(6);
      digitalWrite(alertPin, LOW);
      break;
      
    case SOUND_ERROR:                      // signify an error has been detected
      digitalWrite(alertPin, HIGH);
      delay(200);
      digitalWrite(alertPin, LOW);
      delay(50);
      digitalWrite(alertPin, HIGH);
      delay(200);
      digitalWrite(alertPin, LOW);
      delay(50);
      digitalWrite(alertPin, HIGH);
      delay(200);
      digitalWrite(alertPin, LOW);      
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



 
