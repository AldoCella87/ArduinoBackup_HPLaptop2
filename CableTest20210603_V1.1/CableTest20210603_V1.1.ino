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
 *  
 *  or...  have a testing phase followed by a display results phase.  
 *  during test phast test BOTH connectors for shorts as well as continuity for each of 3 pins
*/

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




#include "Arduino.h"


// setup LCD display
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS2,2,1,0,4,5,6,7); 

#define TEST_MODE
 
// non test mode parameters


#define PIEZO_CLICK             0           // make click as audio feedback for key press
#define PIEZO_ERROR             1           // make click as audio feedback for key press
#define PIEZO_TICK              2

const int alertPin = 10;       // sonalert for audible 'click' when button is pressed and other optional uses
const int iVersion = 2;
long liTestStartTime;


long lGndErrorCount = 0;
long lHotErrorCount = 0;
long lNeutralErrorCount = 0;

void setup() {
  int i;
  Serial.begin(9600);

// don't need these here - move to test function so they may be dynamically assigned
  pinMode(MIC_GND, INPUT);
  pinMode(MIC_HOT, INPUT);
  pinMode(MIC_NEUTRAL, INPUT);
  pinMode(MIC_SHELL, INPUT);
  pinMode(MXR_GND, INPUT);
  pinMode(MXR_HOT, INPUT);
  pinMode(MXR_NEUTRAL, INPUT);
  pinMode(MXR_SHELL, INPUT);
  
  // LCD display setup
  //lcd.off(); 
  lcd.begin (20,4);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  // end LCD setup

  

  // pinmode for sonalert pin
  pinMode(alertPin, OUTPUT);


  // TESTING...
  DisplayWelcomeMsg();
  delay(4000);


  liTestStartTime = millis();                 // get starting time 
}

void loop() {
  int iTestResult = 0;
  iTestResult = ConductTest(millis());


  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(lGndErrorCount);

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(lHotErrorCount); 

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(lNeutralErrorCount);    

  delay(5000);

  swReset();
  




  
  /**************
  DisplayContinuityResult();
  delay(4000);
  DisplayCrossConnectResult(3,2,1);
  delay(4000);
  DisplayIntermittenceResult(0,0,1);
  delay(4000);    
  DisplayShortResultMic();
  delay(4000);
  DisplayShortResultMxr();
  delay(4000);
  DisplayCapacitanceResult(167);
  delay(4000);
  DisplaySummary(iTestResult);
  delay(4000);
  *****************/

  
}


/*
 * Conduct Test
 * Returns 8 bit integer signifying test results:
 * bit position     condition                             value
 * 0                pin 1 continuity failure              1
 * 1                pin 2 continuity failure              2
 * 2                pin 3 continuity failure              4
 * 3                Connector A pin 1 to pin 2 short      8
 * 4                Connector A pin 2 to pin 3 short      16
 * 5                Connector A pin 3 to pin 1 short      32
 * 6                Connector B pin 1 to pin 2 short      64
 * 7                Connector B pin 2 to pin 3 short      128
 * 8                Connector B pin 3 to pin 1 short      256
 */
 
int ConductTest(unsigned long ulStartTime)  
{
  int iRetnValue = 0;
  unsigned long lPreviousDot = 0;


  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Testing "));

  // test continuity on all three pins

  // setup the test, establish pin modes and write a LOW for each pin
  pinMode(MIC_GND, OUTPUT);
  pinMode(MXR_GND, INPUT_PULLUP);
  digitalWrite(MIC_GND, LOW);

  pinMode(MIC_HOT, OUTPUT);
  pinMode(MXR_HOT, INPUT_PULLUP);
  digitalWrite(MIC_HOT, LOW);
  
  pinMode(MIC_NEUTRAL, OUTPUT);
  pinMode(MXR_NEUTRAL, INPUT_PULLUP);
  digitalWrite(MIC_NEUTRAL, LOW);

  // monitor cable for specified time while user manipulates the cable and connectors attempting to reveal
  // an intermittent connection
  while (millis() < ulStartTime + 20000)
  {
    if(millis() >lPreviousDot + 5000)
    {
       lcd.print(F("."));
       lPreviousDot = millis();
    }

    
    if (digitalRead(MXR_GND) != LOW) lGndErrorCount ++;
    if (digitalRead(MXR_HOT) != LOW) lHotErrorCount ++;
    if (digitalRead(MXR_NEUTRAL) != LOW) lNeutralErrorCount ++;  
  }

   return(iRetnValue);
} 

/*
 * creates audible click when button is pushed
 * may be used for other signaling
 */
void PiezoControl(int iMode)
{
  switch(iMode)
  {
    case PIEZO_CLICK:                       // brief click sound for positive feedback of keypress
      digitalWrite(alertPin, HIGH);
      delay(2);
      digitalWrite(alertPin, LOW);
      break;

      case PIEZO_TICK:                    
      digitalWrite(alertPin, HIGH);
      delay(6);
      digitalWrite(alertPin, LOW);
      break;
      
    case PIEZO_ERROR:                      // signify an error has been detected
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
    PiezoControl(PIEZO_CLICK);  
}



/*
 * Display Results for open circuit test
 */
void DisplayContinuityResult(void) // Restarts program from beginning but does not reset the peripherals and registers
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Continuity Test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F("  Pin 1")); 
  lcd.setCursor (12,1);  
  lcd.print(F("Pass"));

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F("  Pin 2"));  
  lcd.setCursor (12,2);  
  lcd.print(F("FAIL"));

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F("  Pin 3"));  
  lcd.setCursor (12,3);  
  lcd.print(F("Pass"));
  
  //lcd.on();

  //PiezoControl(PIEZO_ERROR);
} 


/*
 * Display Results for cross connection test
 */
void DisplayCrossConnectResult(int iResult1, int iResult2, int iResult3)
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
  lcd.print(iResult1);
  if(iResult1 == 1){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }


  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" MIC2 -> MXR")); 
  lcd.setCursor (12,2);
  lcd.print(iResult2);
  if(iResult2 == 2){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" MIC3 -> MXR")); 
  lcd.setCursor (12,3);
  lcd.print(iResult3);
  if(iResult3 == 3){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }
  
  //lcd.on();

} 

/*
 * Display Results for intermittent connection test
 */
void DisplayIntermittenceResult(int iResult1, int iResult2, int iResult3)
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Intermittence Test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" Pin 1: ")); 
  lcd.setCursor (12,1);
  if(iResult1 == 0){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }


  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" Pin 2: ")); 
  lcd.setCursor (12,2);
  if(iResult2 == 0){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" Pin 3: ")); 
  lcd.setCursor (12,3);
  if(iResult3 == 0){
     lcd.print(F(" Pass")); 
  }
  else{
     lcd.print(F(" FAIL"));
  }
  
  //lcd.on();

} 




/*
 * Display Results for short circuit test
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
  lcd.print(F(" Pins 1 to 2:")); 
  lcd.setCursor (14,1);  
  lcd.print(F("FAIL"));

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" Pins 2 to 3:")); 
  lcd.setCursor (14,2);  
  lcd.print(F("Pass"));

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" Pins 3 to 4:")); 
  lcd.setCursor (14,3);  
  lcd.print(F("Pass"));
  
  //lcd.on();

} 



/*
 * Display Results for short circuit test
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
  lcd.print(F(" Values < 200pf best")); 
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


void DisplayShortResultMxr(void)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Mixer end short test"));

  // Display Row 2
  lcd.setCursor (0,1);        
  lcd.print(F(" Pins 1 to 2:")); 
  lcd.setCursor (14,1);  
  lcd.print(F("Pass"));

  // Display Row 3            
  lcd.setCursor (0,2);        
  lcd.print(F(" Pins 2 to 3:")); 
  lcd.setCursor (14,2);  
  lcd.print(F("Pass"));

  // Display Row 4
  lcd.setCursor (0,3);        
  lcd.print(F(" Pins 3 to 4:")); 
  lcd.setCursor (14,3);  
  lcd.print(F("FAIL"));
  
  //lcd.on();

} 

void DisplaySummary(int iErrorCode)  
{
  //lcd.off(); 
  lcd.clear();          
  lcd.home (); 

  // Display Row 1
  lcd.print(F("Summary"));

switch(iErrorCode)
{
  case 0:
    // Display Row 2
    lcd.setCursor (0,1);        
    lcd.print(F("Cable is OK")); 
    break;
  
  case 1:
    // Display Row 2
    lcd.setCursor (0,1);        
    lcd.print(F("Cable needs service")); 
    lcd.setCursor (0,2);        
    lcd.print(F("Continuity error"));
    break;

  case 2:
    // Display Row 2
    lcd.setCursor (0,1);        
    lcd.print(F("Cable needs service")); 
    lcd.setCursor (0,2);        
    lcd.print(F("Continuity error")); 
    break;

  case 9:
    // Display Row 2
    lcd.setCursor (0,1);        
    lcd.print(F("Cable needs service")); 
    lcd.setCursor (0,2);        
    lcd.print(F("Error 9 encountered"));    
    lcd.setCursor (0,3);        
    lcd.print(F("Error code:")); 
    lcd.setCursor (12,3);        
    lcd.print(iErrorCode); 
    break;
    
  default:
    // Display Row 2
    lcd.setCursor (0,1);        
    lcd.print(F("others here...")); 
    break;
  }
  //lcd.on();

}


void swReset(void) // Restarts program from beginning but does not reset the peripherals and registers
{
  lcd.off(); 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Resetting...        ")); 
  lcd.on();
  delay(2000); // or wait for button press to reset
  lcd.clear();
  asm volatile ("  jmp 0");         // boom boom... out go the lights
} 



 
