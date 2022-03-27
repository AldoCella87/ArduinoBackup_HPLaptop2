// read analog an analog value and display it to the serial monitor
// this example is intended to be as simple as possible in order to illustrate the fundamentals
// it may be refined as desired. 

int iAnalogValue = 0;             // variable to hold the analog value 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);             // setup serial output 
}

void loop() {
  // put your main code here, to run repeatedly:
  iAnalogValue = analogRead(0);   // read analog value from analog input zero
  Serial.println(iAnalogValue);   // display analog value to serial monitor
  delay(100);                     // introduce a brief delay to provide time for serial output

}
