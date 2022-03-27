// write a range of analog values.  Display values to the serial monitor
// this example is intended to be as simple as possible in order to illustrate the fundamentals
// it may be refined as desired. 

int iAnalogOutputPin = 9;                       // define variable to identify which analog pin will be used
int iOutputValue = 0;                           // define variable to hold the 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);                           // setup serial output 

}

void loop() {
  // put your main code here, to run repeatedly:
  // Valid analog values range from 0 to 255.  A value of zero produces a digital signal which is in the off 
  // state 100% of the time.  A value of 255 produces a digital signal which is in the on state 100% of the time
  // values in between these two extremes produce a pulse with a pulse width proportionately between these extremes
  // 

  iOutputValue = 0;                             // establish value to be sent to output
  Serial.println(iOutputValue);                 // display the value to the serial monitor
  analogWrite(iAnalogOutputPin, iOutputValue);  // write the specified value to the specified output pin
  delay(4000);                                  // give the user time to view the output
  
  iOutputValue = 32;
  Serial.println(iOutputValue);
  analogWrite(iAnalogOutputPin, iOutputValue);
  delay(4000);
    
  iOutputValue = 64;
  Serial.println(iOutputValue);
  analogWrite(iAnalogOutputPin, iOutputValue);
  delay(4000);

  iOutputValue = 128;
  Serial.println(iOutputValue);
  analogWrite(iAnalogOutputPin, iOutputValue);
  delay(4000);
  
  iOutputValue = 255;
  Serial.println(iOutputValue);
  analogWrite(iAnalogOutputPin, iOutputValue);
  delay(4000);
   
}
