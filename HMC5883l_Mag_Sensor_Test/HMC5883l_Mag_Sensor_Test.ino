// from:  https://forum.arduino.cc/index.php?topic=388339.0
#include <Wire.h>

const int HMC5883L_address = 0x1E;

void setup()
{
  Serial.begin(9600);
  Serial.println("HMC5883L test.");

  Wire.begin();
  Wire.setClock( 100000L);   // set to 50000L for lower speed

  // Write zero to register 2 of the HMC5883L for continuous mode.
  Wire.beginTransmission( HMC5883L_address);
  Wire.write( 2);
  Wire.write( 0x00);
  int error = Wire.endTransmission();
  if( error != 0)
    Serial.println( "Error 1");

  Serial.println( "The ID of the HMC5883L is H43");
}

void loop()
{
  int error, n;
 
  // Read the ID registers (ASCII value: 'H', '4', '3')
  Wire.beginTransmission( HMC5883L_address);
  Wire.write( 10);        // set register to 10 for the start of the ID registers.
  error = Wire.endTransmission( false);    // parameter "false" to test with repeated start
  if( error != 0)
    Serial.println( "Error 2");

  n = Wire.requestFrom( HMC5883L_address, 3);
  if( n != 3)
    Serial.println( "Error 3");

  Serial.print( "ID = ");
  Serial.write( Wire.read());  // write the ASCII value.
  Serial.write( Wire.read());  // write the ASCII value.
  Serial.write( Wire.read());  // write the ASCII value.
 
 
  Wire.beginTransmission( HMC5883L_address);
  Wire.write( 3);        // set register to 3 for the start of the x,z,y values.
  error = Wire.endTransmission( false);     // parameter "false" to test with repeated start
  if( error != 0)
    Serial.println( "Error 4");

  byte buf[6];
  n = Wire.requestFrom( HMC5883L_address, 6);
  if( n != 6)
    Serial.println( "Error 5");
  Wire.readBytes( buf, 6);

  // The order of the registers is x-z-y
  int x = word( buf[0], buf[1]);  // highbyte first
  int z = word( buf[2], buf[3]);
  int y = word( buf[4], buf[5]);

  Serial.print( "   x,y,z = ");
  Serial.print( x);
  Serial.print( ", ");
  Serial.print( y);
  Serial.print( ", ");
  Serial.println( z);

  delay(700);
}
