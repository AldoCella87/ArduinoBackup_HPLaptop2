// 12/12/19
// display level of water in Christmas tree stand.  Display result on bar graph
// This code contains a mashup of an ultrasonic sensor, LCD display and IR control program plus a 
// few additional lines of code and comments. Mount the sensor on a clothespin or similar clamp 
// above the water. 
//
// added NodeMCU web server to show water level, temperature and humidity.  Also operates as a bogus
// honeypot.  Given enough interaction with web server display an Amazon gift card.  
// (removed honeypot and gift card for 2020)
//
//  NodeMCU requires separate code
// 

#include <Wire.h>                     // support for I2C communication to LCD display
#include <LCD.h>                      // support for LCD display
#include <LiquidCrystal_I2C.h>        // support for LCD display


unsigned long currentMillis = 0;      // current time reading taken once each time through loop
unsigned long previousIRMillis = 0;   // save point in time when last IR command was detected
unsigned long previousUSMillis = 0;   // save point in time when last ultrasonic level reading was taken
unsigned long previousAlarmMillis = 0;// save point in time when last ultrasonic level reading was taken
unsigned long IRDuration = 0;         // duration of IR pluse 
const long IRinterval = 800;          // time to wait before acknowledging another IR command
const long USinterval = 10000;       // time to wait before taking another water level reading with US sensor

// inputs           
const int trigPin = 2;                // for ultrasonic distance sensor
const int echoPin = 4;                // for ultrasonic distance sensor
const int IRPin = 5;                  // for IR remote control sensor
const int alarmPin = 7;               // for audible alarm

// outputs
const int relayPin = 8;               // pin which drives relay
const int RelayStatusPin = 9;             // signals relay state to NodeMCU
//const int analogOutPin = 3;           // pin used to signal water level.  using 39uf / 100K total parallel
const int analogOutPin = 6;           // pin used to signal water level.  using 39uf / 100K total parallel
                                      // resistor with 3.9K input resistor to smooth pulses
                                      // 100K is made up of 2 resistors as voltage divider to keep 
                                      // the max voltage output below 3.3v.(approx 60K and 20K)
const int ledPin =  LED_BUILTIN;      // pin which drives internal LED 

// booleans
boolean relayState = false;     // toggle control for relay

long liDuration;
float fDistanceToSensor;        // distance between ultrasonic sensor and top of water
const int iDepthOfTank = 8;     // depth of water tank
const int iSensorOffset = 5;    // distance of the sensor above the surface of the tank
const int iNumberOfSteps = 16;  // number of degrees used to represent range between empty and full
float fWaterDepth = 0.0;        // calculated depth of water in tank
int iSixteenthsToFull = 0;      // proportion of full tank as expressed in one of iNumberOfSteps divisions
int iPrevSixteenthsToFull = 0;  // save previous level to prevent unnecessary display refresh
int iAlarmThreshold = 3;        // sound audio alarm when tank level is less than this value
int iAnalogOut = 0;             // analog output to NodeMCU to indicate water level (range 0 - 255)

LiquidCrystal_I2C  lcd(0x3f,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack
// NOTE - SDA of display connects to A4 pin and SCL of display connects to A5 pin

void setup()
{
  Serial.begin(9600);         // setup serial output
  relayState = false;         // set initial value of relay toggle to FALSE (off)

  // setup inputs
  pinMode(echoPin, INPUT);
  pinMode(IRPin, INPUT);
  
  // setup outputs
  pinMode(relayPin, OUTPUT);          
  pinMode(RelayStatusPin, OUTPUT);  
  pinMode(ledPin, OUTPUT);  
  pinMode(alarmPin, OUTPUT);
  pinMode(analogOutPin, OUTPUT);
  pinMode(trigPin, OUTPUT);

  // initialize outputs
  digitalWrite(relayPin, relayState); 
  digitalWrite(RelayStatusPin, relayState); 
  digitalWrite(ledPin, relayState);  
  digitalWrite(alarmPin, LOW);       

  // activate LCD module and initialize static portion of display
  lcd.begin (16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                  // set cursor to 0,0
  lcd.clear();                  // clear previous values
  lcd.print("Empty       Full");
 
}
 
void loop() 
{


  unsigned long currentMillis = millis();     // get current millisecond reading for subsequent control

  // look for signal from IR Remove  (just looking for receipt of any IR signal, not 
  // a specific code

      // if ((currentMillis - previousIRMillis) >= IRinterval) 
     //  {
          IRDuration = pulseIn(5, LOW);
          if(IRDuration > 0)
          {
            if ((currentMillis - previousIRMillis) >= IRinterval) 
            {        
            previousIRMillis = currentMillis;
            relayState = !relayState;             // toggle the relay state
            digitalWrite(relayPin, relayState);   // write relayState to relay  
            digitalWrite(RelayStatusPin, relayState); // write relayState to NodeMCU 
            digitalWrite(ledPin, relayState);     // set onboard LED to display relayState
            Serial.print("Relay State: ");       
            Serial.println(relayState);
            }
        }

   // Is it time to take another water level reading?
   // if so take reading, scale it for display and display it
   // then sound alarm if water level is low
   if ((currentMillis - previousUSMillis) >= USinterval) 
   {
      previousUSMillis = currentMillis;
      // get distance to the object from the ultrasonic sensor and calculate distance
      // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
      // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      liDuration = pulseIn(echoPin, HIGH);
      // calculate distance based on time for echo to travel to and from water surface
      // 29 microseconds per cm
      fDistanceToSensor = float(liDuration) / float(29) / float(2);
 
      // limit range of reading and remove offset
      if(fDistanceToSensor > (iDepthOfTank + iSensorOffset))
        {
          fWaterDepth = 0.0;
        }
      else if(fDistanceToSensor <= iSensorOffset) 
        {
          fWaterDepth = iDepthOfTank;
        }
      else
        {
          fWaterDepth = iDepthOfTank - (fDistanceToSensor - iSensorOffset);
        }
        
      // calculate scaled level based on raw level. zero raw = zero scaled,
      // iDepthOfTank = iNumberOfSteps
      iSixteenthsToFull = fWaterDepth / iDepthOfTank * iNumberOfSteps;

      // write results to analog output to be read by NodeMCU and reported via web server
      iAnalogOut = iSixteenthsToFull * 15;    // scale sixteenths up to 0 - 255
      analogWrite(analogOutPin, iAnalogOut);    // write scaled value to analog output 



      // Write the results to the serial monitor
      Serial.print("Duration:  ");
      Serial.print(liDuration);
      Serial.print("    Distance to sensor (cm):  ");
      Serial.print(fDistanceToSensor);
      Serial.print("  Water Depth (cm): ");
      Serial.print(fWaterDepth);  
      Serial.print("  sixteenths to full: ");
      Serial.print(iSixteenthsToFull);
      Serial.print("   analog value to NodeMCU: ");
      Serial.print(iAnalogOut);
      Serial.println();

      // display result on LCD screen only if level has changed
      if (iSixteenthsToFull != iPrevSixteenthsToFull)
      {
        lcd.setCursor (0,1);            // go to start of 2nd line
        lcd.print("                ");  // remove display value from previous cycle
        lcd.setCursor (0,1);            // go to start of 2nd line
        for (int i = 0; i<iSixteenthsToFull; i++)
        {
          lcd.print("*");
        }
        iPrevSixteenthsToFull = iSixteenthsToFull;  // save the new reading as the previous reading for next comparison
      }
      // sound alarm if low water level
      if(iSixteenthsToFull < iAlarmThreshold)
      {
        for(int i = 1; i<2; i++)
        {
          digitalWrite(7,HIGH);
          delay(18);
          digitalWrite(7,LOW);
          delay(100);
        }
      }
   } // end if time to take another reading
   
}
