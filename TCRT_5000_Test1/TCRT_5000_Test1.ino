/*
test the TCRT_5000 IR proximity sensor.  Code started with Arduino analog read example...
IR LED resistor using 220 ohms giving about 17ma of LED current
phototransistor collector resistor is 10K
potential problem with picking up optical noise from flourescent lights which appears as ripple on the signal.

An analog input threshold of approximatley 830 seems to be workable, detecting a finger at one inch distance.
*/

int sensorPin = A0;    
int iThresholdPin = A1; // use potentiometer to establish threshold

int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
int iThresholdValue = 0;  // variable holding the object detection threshold

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(9600);
}

void loop() {
  // read the value from the sensor - low value indicates presence of object
  sensorValue = analogRead(sensorPin);
  iThresholdValue = analogRead(iThresholdPin);
  if(sensorValue > iThresholdValue) 
  {
  // turn the ledPin off
  digitalWrite(ledPin, LOW);
  }
  else
  {
  // turn the ledPin on indicating that an object has been detected
  digitalWrite(ledPin, HIGH);
  }
  
 Serial.println(iThresholdValue);



}
