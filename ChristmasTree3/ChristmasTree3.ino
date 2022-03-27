// Holly and family,   
// This code contains a mashup of a couple of the ultrasonic sensor and LCD display programs plus a 
// few additional lines of code and comments.  This is easily adapted to a simple 
// Christmas tree water level monitor.  Mount the sensor on a clothespin or similar clamp 
// above the water.  Determine what values are required to represent high, medium, low and display 
// results on LCD display using if or case structure.  Alternatively, create a bar-graph style display by filling in boxes or asterisks
// from left to right.  Be creative and have fun.  There are lots of good resources on the web.
// Merry Christmas!  -A

/* Demonstration sketch for PCF8574T I2C LCD display 
Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// establish variables for duration of the ping, 
// and the distance result in inches and centimeters:
long duration, inches, cm;
  
/* HC-SR04 Sensor
   https://www.dealextreme.com/p/hc-sr04-ultrasonic-sensor-distance-measuring-module-133696
  
   This sketch reads a HC-SR04 ultrasonic rangefinder and returns the
   distance to the closest object in range. To do this, it sends a pulse
   to the sensor to initiate a reading, then listens for a pulse 
   to return.  The length of the returning pulse is proportional to 
   the distance of the object from the sensor.
     
   The circuit:
  * VCC connection of the sensor attached to +5V
  * GND connection of the sensor attached to ground
  * TRIG connection of the sensor attached to digital pin 2
  * ECHO connection of the sensor attached to digital pin 4
   Original code for Ping))) example was created by David A. Mellis
   Adapted for HC-SR04 by Tautvidas Sipavicius
   This example code is in the public domain.
 */
const int trigPin = 2;
const int echoPin = 4;

LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack

void setup()
// setup is run one time only each time the board is powered on or reset
{
  // Step 1 - activate LCD module
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);

  // Step 2 - initialize serial communication
  Serial.begin(9600);
  
}

void loop()
// loop runs continually after setup has been executed
{

  // Step 1 - get distance to the object from the ultrasonic sensor and calculate distance

  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert time it took to receive echo in microseconds into corresponding distance in inches and cm.  
  // Divide by 2 because we only want to measure the distance from sensor to object, 
  // not from sensor to object and back to sensor. 
  // speed of sound is 1130 feet per second = 74 microseconds per inch
  inches = duration / 74 / 2;
  // speed of sound is 340 meters per second = 29 microseconds per cm
  cm = duration / 29 / 2;
  // sanity check: There are about 2.5 cm per inch so we expect it to take about 2.5 times as long 
  // for sound to cover an inch than it does to cover a centimeter.  
  
  // Step 2 - Write the results to the serial monitor
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  
  // Step 3 - Write results to the LCD display
  lcd.home ();                  // set cursor to 0,0
  lcd.clear();                  // clear previous values
  lcd.print(inches);            // display distance in inches
  lcd.print(" inches");
  lcd.setCursor (0,1);          // go to start of 2nd line
  lcd.print(cm);                // display distance in centimeters
  lcd.print(" centimeters");

  // Step 4 - delay for a moment and do it all again
  delay(1000);                  // sleep for one second then execution begins again at top of loop()
}
