// 12/21/19
// accumulate run time, when switch is on accumulate time
// when switch is off do not accumulate time
// display result on LCD

#include <Wire.h>                     // support for I2C communication to LCD display
#include <LCD.h>                      // support for LCD display
#include <LiquidCrystal_I2C.h>        // support for LCD display


unsigned long ulCurrentMillis = 0;    // current time reading taken once each time through loop
unsigned long ulPrevMillis = 0;       // previous millis reading used to control display frequency
unsigned long ulStartMillis = 0;      // starting point for elapsed time
unsigned long ulEndMillis = 0;        // ending point in time for elapsed time
unsigned long ulElapsedMillis = 0;    // milliseconds of accumulated time
unsigned long ulLastDisplayedTime = 0;// controls display of accumulated time on LCD display
unsigned long ulSeconds = 0;
unsigned long ulMinutes = 0;
unsigned long ulHours = 0;

// inputs           
const int startStopPin = 2;          // toggle for start/stop timing


// store switch state
int iSwitchState = 0;
int iPrevSwitchState = 0;



LiquidCrystal_I2C  lcd(0x3f,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack
// NOTE - SDA of display connects to A4 pin and SCL of display connects to A5 pin

void setup()
{
  Serial.begin(9600);         // setup serial output
  
  // setup inputs
  pinMode(startStopPin, INPUT);
   
  // activate LCD module and initialize static portion of display
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                  // set cursor to 0,0
  lcd.clear();                  // clear previous values
  lcd.print("Project:  1");

  ulPrevMillis = millis();       // get initial millis reading
}
 
void loop() 
{
iSwitchState = digitalRead(startStopPin);         // get state of start-stop switch
ulCurrentMillis = millis();                       // get current millisecond reading

// handle switch transitions
if((iSwitchState == 1) && (iPrevSwitchState == 0))  // detect start condition
{
  ulStartMillis = ulCurrentMillis; 
  iPrevSwitchState = 1;
  Serial.println("Switch turned on");
}

if((iSwitchState == 0) && (iPrevSwitchState == 1))  // detect stop condition
{
  ulEndMillis = ulCurrentMillis;  
  iPrevSwitchState = 0;
  Serial.println("Switch turned off");
}



// handle elapsed time accumulation if called for by switch 
if(iSwitchState == 1)
{
  // calculate elapsed milliseconds for each loop
  ulElapsedMillis = ulCurrentMillis - ulPrevMillis;
  if(ulElapsedMillis > 999)   // has one second elapsed?
  {
    ulSeconds ++;
    ulElapsedMillis = 0;
    ulPrevMillis = ulCurrentMillis;
    if(ulSeconds > 59)
    {
      ulMinutes++;
      ulSeconds = 0;
    }
    if(ulMinutes > 59)
    {
      ulHours++;
      ulMinutes = 0;
    }
  }
}

// time to display value?
   if (millis() - ulLastDisplayedTime > 1000) 
   {
      ulLastDisplayedTime = millis();
      lcd.setCursor (0,1);            // go to start of 2nd line
      lcd.print("                ");
      lcd.setCursor (0,1);            // go to start of 2nd line
      lcd.print(ulHours);
      lcd.print(":");
      lcd.print(ulMinutes);
      lcd.print(":");      
      lcd.print(ulSeconds);
  
   }




   
}
