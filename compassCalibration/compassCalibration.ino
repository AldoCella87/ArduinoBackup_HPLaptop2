#include <Wire.h>

#define ADDRESS 0x21

void setup(){
  Wire.begin();
  Serial.begin(9600);
  while(!Serial);
  calibrate();
}

void loop(){
}

void calibrate(){
  Serial.println("Calibration Mode");
  delay(1000);  //1 second before starting
  Serial.println("Start");

  Wire.beginTransmission(ADDRESS);
  Wire.write(0x43);
  Wire.endTransmission();
  for(int i=0;i<15;i++){        //15 seconds
   Serial.println(i);
   delay(1000);
  }
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x45);
  Wire.endTransmission();
  Serial.println("done");

}
