#include <EEPROM.h>


char data = 0;                //Variable for storing received data
#define LED_ADDRESS 0         // using EEPROM address 0 as storage destination
int iLedState = 2;            // set to a value which will never be used to signify
                              // update required during first pass
void setup() 
{
  Serial.begin(9600);         //Sets the data rate in bits per second (baud) for serial data transmission
  pinMode(13, OUTPUT);        //Sets digital pin 13 as output pin
  iLedState = EEPROM.read(LED_ADDRESS);  // get previously stored LED state and set LED accordingly
  if(iLedState == 1)             
    digitalWrite(13, HIGH);   
  else if(iLedState == 0)        
    digitalWrite(13, LOW);   
  
}
void loop()
{
  if(Serial.available() > 0)  // Send data only when you receive data:
  {
    data = Serial.read();      //Read the incoming data and store it into variable data
    Serial.print(data);        //Print Value inside data in Serial monitor
    Serial.print("\n");        //New line 
    
    if(data == '1')            //Checks whether value of data is equal to 1 
    {
      digitalWrite(13, HIGH);  //If value is 1 then LED turns ON
      EEPROM.update(LED_ADDRESS, 1);
    }
    else if(data == '0')       //Checks whether value of data is equal to 0
    {
      digitalWrite(13, LOW);   //If value is 0 then LED turns OFF
      EEPROM.update(LED_ADDRESS, 0);
    }
    
  }                            
 
}
