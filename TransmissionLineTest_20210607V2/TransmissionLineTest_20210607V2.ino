// test bed based on cable tester.  Investigate capacitive coupling between lines which are otherwise unconnected.
// 6/7/21 - measured 40 feet of cable = 1000pf = 25pf/foot

 

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
#define SOUND_DBLCLICK          3

// setup LCD display
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 

const int alertPin = 10;          // sonic alert pin
const int iVersion = 3;

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
  delay(500);  // replace with "press button to continue"
}

void loop() {
  int iTestResult = 0;
  
  iTestResult = ConductTest(millis());

  // display results
  lcd.clear();          
  lcd.home (); 

  DisplayShortResultMic();
  delay(4000);
 
  // execute test again (remove with addition of user controls)
  swReset();    
  

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
int ConductTest(unsigned long ulStartTime)  
{
  int iRetnValue = 0;
  int iTestDuration = 2000;            
  unsigned long lPreviousDot = 0;


  
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Testing ... "));
  lcd.setCursor (0,1);        
   

  // setup test - all pins should remain in hi z state except the output
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

    // B.1 Check for short between all pins on MIC side connector
    // B.1.1 Check for short between MIC_GND and (MIC_HOT, MIC_NEUTRAL and MIC_SHELL)
    // B.1.1.0 setup test
    pinMode(MIC_GND, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin

    
    pinMode(MIC_HOT, INPUT_PULLUP);                                 // set subject being tested as input with pullup
    pinMode(MIC_NEUTRAL, INPUT_PULLUP);                             // set subject being tested as input with pullup
    pinMode(MIC_SHELL, INPUT_PULLUP);                               // set subject being tested as input with pullup


    digitalWrite(MIC_GND, LOW);                                     // set a low logic level on input side
                                                                    // observation:  If the digital write comes after the setting of pinmode pullups 
                                                                    // then the crosstalk effect observed presiously is significantly reduced.
                                                                    // both the driving of MIC_GND low as well as setting the other pins high via pullup
                                                                    // have an impact on crosstalk.  connecting a 1000 pf capacitor between pins 1 and 2 simulated
                                                                    // the crosstalk effect that was previously observed.

    delayMicroseconds(80);                                       // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MIC_HOT) == LOW) lMIC12ShortCount ++;           // For short circuit test the expected result is HIGH (no connection)
    if (digitalRead(MIC_NEUTRAL) == LOW) lMIC13ShortCount ++;       // For short circuit test the expected result is HIGH (no connection)
     pinMode(MIC_GND, INPUT);                                        // done testing with MIC_GND, return input side to high z state


// NOTE - commenting out B.1.2 significantly reduces the observed effect...


    // B.1.2 Check for short between MIC_HOT and (MIC_NEUTRAL and MIC_SHELL) (fewer tests required here because some conditions were covered above)
    // B.1.2.0 setup test
    pinMode(MIC_HOT, OUTPUT);                                       // setup digital output to serve as input to connector under test
                                                                    // this is set ONCE for all short circuit tests involving this pin
    digitalWrite(MIC_HOT, LOW);                                     // set a low logic level on input side for all tests in this section
    delayMicroseconds(80);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
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
    delayMicroseconds(80);                                         // allow system under test to stabilize prior to reading, No delay is required for very short cables
                                                                    // as cable length increases erratic results arise without a delay.  A 20 foot cable tested required
                                                                    // 11 microsecods delay before results were correct.  Two cables in series (40 feet) required 
                                                                    // 26 microseconds delay before results were correct.  Further testing required, for now 
                                                                    // use 100 microseconds. 
    if (digitalRead(MIC_SHELL) == LOW) lMIC3SShortCount ++;         // For short circuit test the expected result is HIGH (no connection)
    pinMode(MIC_NEUTRAL, INPUT);                                    // done testing with MIC_NEUTRAL, return input side to high z state

    pinMode(MIC_SHELL, INPUT);                                      // done testing with MIC_SHELL, return input side to high z state
                                                                    // at this point all 4 pins on MIC connector are back to hi z state



  }  // end of loop for specified time period

   return(iRetnValue);
} 


/*********************************************************************
 * display welcome message
 * 
 *********************************************************************/
void DisplayWelcomeMsg(void)
{
    //  Row 1
    lcd.setCursor (0,0);        
    lcd.print(F("cap coupling test "));
    SonalertControl(SOUND_CLICK);  
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
  lcd.print(F("Pin      Count"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F("1-2:")); 
  lcd.setCursor (11,1); 
  lcd.print(lMIC12ShortCount);

  // Display Row 3
  lcd.setCursor (0,2);        
  lcd.print(F("1-3:")); 
  lcd.setCursor (11,2);  
  lcd.print(lMIC13ShortCount);

    // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F("2-3:")); 
  lcd.setCursor (11,3);  
  lcd.print(lMIC23ShortCount);

  
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
      
    case SOUND_DBLCLICK:                       // brief click sound for positive feedback of keypress
      digitalWrite(alertPin, HIGH);
      delay(2);
      digitalWrite(alertPin, LOW);
      delay(40);
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



 
