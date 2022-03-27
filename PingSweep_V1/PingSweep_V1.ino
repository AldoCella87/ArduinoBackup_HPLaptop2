// Based on Ping and Sweep examples - AE 12/2017

/* Ping))) Sensor

   This sketch reads a PING))) ultrasonic rangefinder and returns the
   distance to the closest object in range. To do this, it sends a pulse
   to the sensor to initiate a reading, then listens for a pulse
   to return.  The length of the returning pulse is proportional to
   the distance of the object from the sensor.

   The circuit:
	* +V connection of the PING))) attached to +5V
	* GND connection of the PING))) attached to ground
	* SIG connection of the PING))) attached to digital pin 7

   http://www.arduino.cc/en/Tutorial/Ping

   created 3 Nov 2008
   by David A. Mellis
   modified 30 Aug 2011
   by Tom Igoe

   This example code is in the public domain.

 */
#include <Servo.h>

// this constant won't change.  It's the pin number
// of the sensor's output:
const int trigPin = 10;
const int echoPin = 11;
int iaReading[9];           // holds one of 9 readings in order to establish the median value
int iaDistance[180];        // array to hold distances, one per degree of rotation


Servo myservo;  // create servo object to control a servo

int iPos = 5;    // variable to store the servo position

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(iPos);   // start scan at zero degrees
  delay(1000);
}

void loop() {
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, inches, cm;



  // gather 9 readings for servo position x
  for(int i = 0; i<9; i++)
  {
    
      // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
      // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
      pinMode(trigPin, OUTPUT);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(trigPin, LOW);
    
      // The same pin is used to read the signal from the PING))): a HIGH
      // pulse whose duration is the time (in microseconds) from the sending
      // of the ping to the reception of its echo off of an object.
      pinMode(trigPin, INPUT);
      duration = pulseIn(echoPin, HIGH);
    
      // convert the time into a distance
      cm = microsecondsToCentimeters(duration);
      iaReading[i] = cm;        // store distance  
     // Serial.println(iaReading[i]);
  }


 int k;
  // sort readings for servo position x
  // outside loop controls sort
  for(int i = 8; i>0; i--)
  {
    // inside loop - move smallest to bottom
    for(int j = 0; j<i; j++)
    {  
      if(iaReading[j] < iaReading[j+1])
      {
        k = iaReading[j];
        iaReading[j] = iaReading[j+1];
        iaReading[j+1] = k;        
      }
    }
  }


  iaDistance[iPos] = iaReading[4];      // use median value
  Serial.println(iaReading[4]);
  delay(100);


if(iPos < 175) 
{
    // step to next position
    iPos ++;
    myservo.write(iPos);              // tell servo to go to position in variable 'pos'
    delay(10);                        // waits for the servo to reach the position
}
else
{
    // return home
    iPos = 5;
    myservo.write(iPos);               // tell servo to go to position in variable 'pos'
    delay(1000);                       // waits for the servo to reach the position
}

  
}



long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
