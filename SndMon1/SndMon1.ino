
// modified version of analog read example for use with sound monitor card.  AE 12/17
/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

int iSndLvl = 0;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  if (sensorValue > 1024) sensorValue = 1024;

iSndLvl = sqrt((sensorValue * sensorValue));
  
  // print out the value you read:
  //Serial.print(sensorValue);  
  //Serial.print("   ");
  Serial.println(iSndLvl);
  delay(1);        // delay in between reads for stability
}
