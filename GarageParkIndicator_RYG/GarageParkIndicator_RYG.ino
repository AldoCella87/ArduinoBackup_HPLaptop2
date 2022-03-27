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
const int echoPin = 3;
const int GreenPin = 4;
const int YellowPin = 5;
const int RedPin = 6;
int prev_inches = -1;
int deadband = 0;
void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  pinMode(GreenPin, OUTPUT);
  digitalWrite(GreenPin, LOW);
  pinMode(YellowPin, OUTPUT);
  // initialize yellow light (proceed with caution) to ON
  digitalWrite(YellowPin, HIGH);
  pinMode(RedPin, OUTPUT);
  digitalWrite(RedPin, LOW);

}

void loop()
{


  if (prev_inches == -1)
  {
    deadband = 0;
  }
  else
  {
    deadband = 1;
  }
  
    
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

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

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  
  Serial.print(inches);
  Serial.print("in, ");
  //Serial.print(cm);
  //Serial.print("cm");

// discard outlying high values
if (inches - prev_inches < 10)
  {
    if(inches <= 30 - deadband){
      digitalWrite(GreenPin, LOW);
      digitalWrite(YellowPin, LOW);
      digitalWrite(RedPin, HIGH);
      Serial.print("  Red");
    }
    if ((inches <= 39 - deadband) && (inches > 30 + deadband)){
      digitalWrite(GreenPin, HIGH);
      digitalWrite(YellowPin, LOW);
      digitalWrite(RedPin, LOW);
      Serial.print("  Green");
    }
    if(inches > 39 + deadband)
    {
    digitalWrite(GreenPin, LOW);
    digitalWrite(YellowPin, HIGH);
    digitalWrite(RedPin, LOW);
    Serial.print("  Yellow");
    }
  }
else
  {
      Serial.print("  ignoring value");
  }


  Serial.println();
  prev_inches = inches;

  delay(500);

}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
