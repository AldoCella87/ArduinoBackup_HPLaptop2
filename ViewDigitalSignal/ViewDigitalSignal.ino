// instrumentation exercise - observe digital signal

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);             // setup serial output 
  pinMode(9, OUTPUT);             // establish pin 9 as output
  Serial.println("instrumentation exercise - observe digital signal");

}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(9, LOW);           // set pin 9 to low logic state (near zero volts)
  Serial.println("Low");          // send message to serial monitor
  delay(2000);                    // wait 2 seconds 

  digitalWrite(9, HIGH);          // set pin 9 to high logic state (near five volts)
  Serial.println("High");         // send message to serial monitor
  delay(2000);                    // wait 2 seconds  
}
