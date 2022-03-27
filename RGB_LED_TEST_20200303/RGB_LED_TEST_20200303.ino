/* Simple application to send three PWM signals to a tri-color LED.
 * PWM output values are set according to three analog inputs
 * which are controlled by the user  
 *
 * Note: Analog inputs range from 0 to 1024.  
 * Analog outputs range from 0 to 255 so we must scale the output values down by a factor of 4.  
 * 11/1/19 AE
 */

// INCLUDES
#include <Wire.h>                                              
#include <LiquidCrystal_I2C.h>
// setup for LCD - note my display uses address 0x3F,     
// another typically used address is 0x27                              
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);   
                                                                
// CONSTANTS
// input and output pin constants.  
// Note "A0" etc. are integer aliases for analog input pins
const int analogInRed = A0;   
const int analogInGreen = A1;   
const int analogInBlue = A2; 
  
const int analogOutRed = 3;  
const int analogOutGreen = 5;  
const int analogOutBlue = 6;  

// VARIABLES
// variables to hold input values.  Range is 0 to 1024
int i_sensorValueRed = 0;         
int i_sensorValueGreen = 0;         
int i_sensorValueBlue = 0; 

// variables to hold output values.  Range is 0 to 255
int i_RedValue = 0;
int i_GreenValue = 0;
int i_BlueValue = 0;

// variables to hold previous values, this controls whether or not to write to the LCD display
// no need to write if there is no change
int i_PrevRedValue = 999;
int i_PrevGreenValue = 999;
int i_PrevBlueValue = 999; 

// SETUP - runs one time
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  // set output pin mode
  pinMode(analogOutRed, OUTPUT);
  pinMode(analogOutGreen, OUTPUT);
  pinMode(analogOutBlue, OUTPUT);

  // Setup LCD display
  lcd.begin(16,2);               // Identify the display type (16 char, 2 lines)
  lcd.backlight();               // turn backlight on  

  // display header on LCD screen.  This never changes so we do it only once here in setup()
  lcd.setCursor(0,0);            // set cursor to character 0 of line 0 
  lcd.write("Red   Grn   Blu");  // send message
  
}


// Loop - runs continuously
void loop() {
  // read the analog in value:
  i_sensorValueRed = analogRead(analogInRed);
  i_sensorValueGreen = analogRead(analogInGreen);
  i_sensorValueBlue = analogRead(analogInBlue);

  // scale the input range of 0 to 1024 to the output range of 0 to 255.  
  // (simply divide input value by 4)
  i_RedValue = i_sensorValueRed / 4;  
  i_GreenValue = i_sensorValueGreen / 4;
  i_BlueValue = i_sensorValueBlue / 4;
  
  // set the analog out value:
  analogWrite(analogOutRed, i_RedValue);
  analogWrite(analogOutGreen, i_GreenValue);
  analogWrite(analogOutBlue, i_BlueValue);

  // display results on serial monitor
  Serial.print("Red Value: ");
  Serial.print(i_RedValue);
  Serial.print("     Green Value: ");
  Serial.print(i_GreenValue);
  Serial.print("     Blue Value: ");
  Serial.print(i_BlueValue);
  Serial.println();

  // display results on LCD screen
  // new red value?
  if(i_RedValue != i_PrevRedValue){
      lcd.setCursor(0,1);                // set cursor to character 0 of line 1 
      lcd.print("   ");                  // clear previous value
      lcd.setCursor(0,1);                // set cursor to character 0 of line 1       
      lcd.print(i_RedValue);             // write current value
  }

  // new green value?
  if(i_GreenValue != i_PrevGreenValue){
      lcd.setCursor(6,1);                // set cursor to character 0 of line 1 
      lcd.print("   ");                  // clear previous value
      lcd.setCursor(6,1);                // set cursor to character 0 of line 1       
      lcd.print(i_GreenValue);           // write current value
  }

  // new blue value?
  if(i_BlueValue != i_PrevBlueValue){
      lcd.setCursor(12,1);               // set cursor to character 0 of line 1 
      lcd.print("   ");                  // clear previous value
      lcd.setCursor(12,1);               // set cursor to character 0 of line 1       
      lcd.print(i_BlueValue);             // write current value
  }

  
  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2);
}
