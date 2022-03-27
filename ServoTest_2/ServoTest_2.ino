#include <Servo.h>
 
int servoPin = 10;
int pos = 0;
int servoDelay = 25;
Servo myServo;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
myServo.attach(servoPin);

}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println("Enter Value: ");
while(Serial.available() == 0){};
pos = Serial.parseInt();

myServo.write(pos);
delay(servoDelay);

Serial.println(myServo.read());


}
