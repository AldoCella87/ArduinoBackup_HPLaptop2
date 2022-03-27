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

//installed 1/14/17 - set point 51in aimed at Ford Explorer windshield

const int trigPin = 2;
const int echoPin = 3;
const int GreenPin = 4;
const int YellowPin = 5;
const int RedPin = 6;
int FarEdgeGreen = 59;   // original value 1/14/17   (far edge puts car outside garage)
int NearEdgeGreen = 50;  // original value 1/14/17   (correct placement of near edge)
                               // range of 9 inches resulted in green zone of 33 inches due to angle of beam against windshield
                               // factor of 3.66 green zone travel inches for each detected inch            
int prev_inches = 0;
int greenbandAddition = 1;

// get desired distance from A0
int sensorPin = A0;    // select the input pin for the potentiometer
int TargetDistance = 0;  // variable to store the value coming from the sensor



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

  // read desired distance, scale down by a factor of to to give reading from 0 to 102
  TargetDistance = analogRead(sensorPin) / 10;
  FarEdgeGreen = TargetDistance + 2; 
  NearEdgeGreen = TargetDistance - 2;
  Serial.print("Target Distance: ");
  Serial.print(TargetDistance);
  Serial.print("    ");
    
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
  
  Serial.print(inches);
  Serial.print(" in");

// discard outlying high values
if (inches - prev_inches < 10)
  {
    if(inches < NearEdgeGreen - greenbandAddition){
      digitalWrite(GreenPin, LOW);
      digitalWrite(YellowPin, LOW);
      digitalWrite(RedPin, HIGH);
      Serial.print("  Red");
      greenbandAddition = 0;
    }
    if ((inches <= FarEdgeGreen - greenbandAddition) && (inches >= NearEdgeGreen + greenbandAddition)){
      digitalWrite(GreenPin, HIGH);
      digitalWrite(YellowPin, LOW);
      digitalWrite(RedPin, LOW);
      Serial.print("  Green");
      greenbandAddition = 1;
    }
    if(inches > FarEdgeGreen + greenbandAddition)
    {
    digitalWrite(GreenPin, LOW);
    digitalWrite(YellowPin, HIGH);
    digitalWrite(RedPin, LOW);
    Serial.print("  Yellow");
    greenbandAddition = 0;
    }
  }
else
  {
      Serial.print("  ignoring value");
  }

  Serial.println();
  prev_inches = inches;
  delay(200);
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



