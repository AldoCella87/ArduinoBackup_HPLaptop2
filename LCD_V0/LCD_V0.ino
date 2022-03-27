
#include <Wire.h>               // required by LCD library to communicate with LCD
#include <LiquidCrystal_I2C.h>  // custom third party library containint functions for LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // setup for LCD
// based on information posted at https://arduino-info.wikispaces.com/LCD-Blue-I2C 

void setup()    
{

  lcd.begin(16,2);              // Identify the display type (16 char, 2 lines)
  lcd.backlight();              // turn backlight on
  
}


void loop()    
{

  lcd.setCursor(0,0);           // set cursor to character 0 of line 0 (left corner)
  lcd.write("Hello World");     // send message
  delay(2000);                  // wait 2 seconds
  lcd.clear();                  // clear screen
  delay(500);                   // wait 1/2 second

} 

