// display level of water in watering pan of Christmas tree.  Display result on bar graph
// This code contains a mashup of a couple of the ultrasonic sensor and LCD display programs plus a 
// few additional lines of code and comments.  This is easily adapted to a simple 
// Christmas tree water level monitor.  Mount the sensor on a clothespin or similar clamp 
// above the water.  

// 12/9/19 - add bluetooth level indicator or web server?

/* Demonstration sketch for PCF8574T I2C LCD display 
Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>



// establish variables for duration of the ping, 
// and the distance result in inches and centimeters:
long duration;
float fcm;

char cData = '0';                // single byte to serve as location for serial input

int iDepthOfTank = 8;         // depth of water tank
int iSensorOffset = 5;        // distance of the sensor above the surface of the tank
int iNumberOfSteps = 16;      // number of degrees used to represent range between empty and full
float fRawLevel = 0.0;        // level of tank as reported by sensor (pre-scaled)
int iScaledLevel = 0;         // scaled tank level as expressed in one of iNumberOfSteps divisions
int iPrevScaledLevel = 0;     // save previous level so as to prevent unnecessary display refresh
int iAlarmThreshold = 3;      // sound audio alarm when tank level is less than this value


unsigned long liCurrentMillis = 0;  //used to control how often to sound low water alarm
unsigned long liPrevMillis = 0;
unsigned long iAlarmDelay = 90000;   // controls how long to wait before resounding low water alarm  
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
const int relayPin  = 13; // pin to control light relay

LiquidCrystal_I2C  lcd(0x3f,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack
// NOTE - SDA of display connects to A4 pin and SCL of display connects to A5 pin


void setup()
// setup is run one time only each time the board is powered on or reset
{
  // setup alarm pin
  pinMode(7,OUTPUT);            // low water alarm
  // set alarm state off
  digitalWrite(7, LOW);
  
  // Step 1 - activate LCD module and initialize static portion of display
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                  // set cursor to 0,0
  lcd.clear();                  // clear previous values
  lcd.print("Empty       Full");


  // Step 2 - initialize serial communication
  Serial.begin(9600);

  // Step 3 - initialize relay pin as an output
  pinMode(relayPin, OUTPUT);        //Sets digital pin 13 as output pin
  digitalWrite(relayPin, LOW);      // set initial state to "Off"
 
}

void loop()
// loop runs continually after setup has been executed
{
  // check to see if data is sent via bluetooth link
  if(Serial.available() > 0)  // Send data only when you receive data:
  {
    cData = Serial.read();      //Read the incoming data and store it into variable data
    if(cData == '1')            //Checks whether value of data is equal to 1 
      digitalWrite(relayPin, HIGH);  //If value is 1 then command relay ON
    else if(cData == '0')       //Checks whether value of data is equal to 0
      digitalWrite(relayPin, LOW);   //If value is 0 then command relay OFF
  }    

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
  // speed of sound is 340 meters per second = 29 microseconds per cm
  fcm = float(duration) / float(29) / float(2);
  // sanity check: There are about 2.5 cm per inch so we expect it to take about 2.5 times as long 
  // for sound to cover an inch than it does to cover a centimeter.  
  
  
  // limit range of reading and remove offset
  if(fcm > (iDepthOfTank + iSensorOffset))
    {
      fRawLevel = 0.0;
    }
  else if(fcm <= iSensorOffset) 
    {
      fRawLevel = iDepthOfTank;
    }
  else
    {
      fRawLevel = iDepthOfTank - (fcm - iSensorOffset);
    }
    
  // calculate scaled level based on raw level. zero raw = zero scaled,
  // iDepthOfTank = iNumberOfSteps
  iScaledLevel = fRawLevel / iDepthOfTank * iNumberOfSteps;


  // Write the results to the serial monitor
  Serial.print("actual reading in cm:  ");
  Serial.print(fcm);
  Serial.print("  fRawLevel: ");
  Serial.print(fRawLevel);  
  Serial.print("  iScaledLevel is: ");
  Serial.print(iScaledLevel);
  Serial.print("  Relay command: ");
  Serial.print(cData);
  Serial.println();


  // write result to display only if level has changed
  if (iScaledLevel != iPrevScaledLevel)
  {
    lcd.setCursor (0,1);            // go to start of 2nd line
    lcd.print("                ");  // remove display value from previous cycle
    lcd.setCursor (0,1);            // go to start of 2nd line
    for (int i = 0; i<iScaledLevel; i++)
    {
      lcd.print("*");
      Serial.print("*");
    }
    iPrevScaledLevel = iScaledLevel;  // save the new reading as the previous reading for next comparison
  }

  Serial.println();

  // sound alarm if low water level
  if(iScaledLevel < iAlarmThreshold)
  {
    liCurrentMillis = millis();

    Serial.print( liCurrentMillis);Serial.print("    >> Prev>> ");
    Serial.print(liPrevMillis);
    Serial.println();
    if (liCurrentMillis > (liPrevMillis + iAlarmDelay))
    {
      digitalWrite(7,HIGH);
      delay(20);
      digitalWrite(7,LOW);
      delay(100);
      digitalWrite(7,HIGH);
      delay(20);
      digitalWrite(7,LOW);

      liPrevMillis = millis();
    }
  }

  // delay for a moment and do it all again
  delay(1000);                  // sleep for one second then execution begins again at top of loop()
}
