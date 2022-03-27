/*
 * https://github.com/chauhannaman98
 *
 * sketch.ino
 *
 * July 25, 2018 © GPL3+
 * Author : Naman Chauhan
 * 12/12/19 modified by Al Evangelista - included relay output control and debounce
 */

#include <IRremote.h>                 //including infrared remote header file
unsigned long currentMillis = 0;
unsigned long previousIRMillis = 0;   // save point in time when last IR command was detected
unsigned long previousUSMillis = 0;   // save point in time when last ultrasonic level reading was taken

const long IRinterval = 500;          // time to wait before acknowledging another IR command
const long USinterval = 10000;        // time to wait before taking another water level reading with US sensor
const int  relayPin = 7;              // pin which drives relay
const int ledPin =  LED_BUILTIN;      // the number of the LED pin

boolean relayState = false;           // toggle control for relay
int RECV_PIN = 2;                     // IR sensor input 

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn();
  relayState = false;         // set initial value to FALSE (off)

  pinMode(relayPin, OUTPUT);   
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(relayPin, relayState); // initialize relay and LED to 0 (off)
  digitalWrite(ledPin, relayState);  
}
 
void loop() 
{
  unsigned long currentMillis = millis();     // get current millisecond reading
  if (irrecv.decode(&results)) 
    {
       if (currentMillis - previousIRMillis >= IRinterval) 
       {
          previousIRMillis = currentMillis;
          relayState = !relayState;           // toggle the relay state
          Serial.print(" Toggle value is: "); // display value on serial monitor
          Serial.println(relayState);
          digitalWrite(relayPin, relayState); // write output to relay and onboard LED
          digitalWrite(ledPin, relayState);
       }
       irrecv.resume();                    // Resume receive function
    }




    
}
