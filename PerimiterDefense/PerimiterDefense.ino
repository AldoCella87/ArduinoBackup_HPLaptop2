/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

#define MIN_RADIUS  60          //  consider anything closer than this distance to be a target

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards



const int SweepPin = 3;   // controls sweep (on, off)
const int LaserPin = 7;   //laser control

const int trigPin = 2;
const int echoPin = 4;

long duration = 0;

long cm = 0;                   // distance to target in cm


int pos = 0;    // variable to store the servo position
int iTargetPosition = 0;        // location of target
long liMinDistance = 0;           // the minimum distance found during the sweep

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(LaserPin, OUTPUT);
  pinMode(SweepPin, INPUT);

}

void loop() {

  while(digitalRead(SweepPin) == 1){

      // approach 2 - illuminate anything within target radius while scanning
      digitalWrite(LaserPin, LOW);      // laser off
      for (pos = 0; pos < 180; pos += 2) {  
        myservo.write(pos);              // tell servo to go to position in variable 'pos'
        if(pos == 0) delay(600);        // provide time for servo to return to home position
        // get distance
        digitalWrite(trigPin, LOW);
        delayMicroseconds(5);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(trigPin, LOW);
        duration = pulseIn(echoPin, HIGH);
        cm = (duration / 2) / 29;      // convert to cm
        delay(30);
        
        if(cm < MIN_RADIUS){
            digitalWrite(LaserPin, HIGH);
        }
        else{
            digitalWrite(LaserPin, LOW);
        }

      }// end for (pos = 0; ...
     
  }// while digitalRead
}
