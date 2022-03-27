/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
*/ 

#include <Servo.h> 
 
Servo DoorServo;  // create servo object to control a servo 
Servo SwitchServo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int pos = 0;    // variable to store the servo position 
int DoorClosePos = 12;
int DoorOpenPos = 60;
int SwitchArmHome = 10;
int SwitchArmExtend = 93;
 
void setup() 
{ 
  DoorServo.attach(9);  // attaches the servo on pin 9 to the servo object 
  SwitchServo.attach(8);  // attaches the servo on pin 9 to the servo object 
  Serial.begin(9600);
  DoorServo.write(DoorClosePos);   
  SwitchServo.write(SwitchArmHome);             
  delay(15);            
  Serial.println("Starting... ");                
} 
 
void loop() 
{ 

//OpenLid();
DoorServo.write(DoorOpenPos); 

delay(1000);
SwitchServo.write(SwitchArmExtend); 
delay(1000);
SwitchServo.write(SwitchArmHome); 
delay(1000);

DoorServo.write(DoorClosePos); 
//CloseLid();
delay(1000);
Serial.println(pos++);     

} 

 

