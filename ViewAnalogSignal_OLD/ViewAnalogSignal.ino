
int iAnalogOutputPin = 9;         //define variable to identify which analog pin will be used
int iOutputValue = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);             // setup serial output 

}

void loop() {
  // put your main code here, to run repeatedly:

    /**************

  iOutputValue = 0;
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
  
  ************/
  
for(int i = 0;i<256;i++)
{
  Serial.println(i);
  analogWrite(iAnalogOutputPin, i);
  delay(100);
}

}
