
//  12/30/18  AE  revised parking indicator based on Jan 2017 version.
// changed R/Y/G LEDs to LCD display 
// added mode switch to simplify calibration
// added display of parameters to LCD when in setup mode

#include <Wire.h>               // required by LCD library to communicate with LCD
#include <LiquidCrystal_I2C.h>  // custom third party library containint functions for LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // setup for LCD
// based on information posted at https://arduino-info.wikispaces.com/LCD-Blue-I2C 

void display_parameters();
void print_parameters();

/* HC-SR04 Sensor
   https://www.dealextreme.com/p/hc-sr04-ultrasonic-sensor-distance-measuring-module-133696
  
   This sketch reads a HC-SR04 ultrasonic rangefinder and returns the
   distance to the closest object in range. To do this, it sends a pulse
   to the sensor to initiate a reading, then listens for a pulse 
   to return.  The length of the returning pulse is proportional to 
   the distance of the object from the sensor.
     
   The circuit:
  * VCC connection of the sensor attached to +5V
  * GND connection of the sensor attached to ground
  * TRIG connection of the sensor attached to digital pin 2
  * ECHO connection of the sensor attached to digital pin 4
   Original code for Ping))) example was created by David A. Mellis
   Adapted for HC-SR04 by Tautvidas Sipavicius
   This example code is in the public domain.
 */

// installed 1/14/17 - set point 51in aimed at Ford Explorer windshield (south garage)
// revised version installed 1/1/19 - Volvo XC90 (north garage)

const int iSWVersion = 2;         // Software version number

const int ModePin = 7;            // high sets calibration mode, low sets normal operate mode
const int trigPin = 2;            // for ultrasonic sensor
const int echoPin = 3;            // for ultrasonic sensor

                                  // codes which represent current state of vehicle within the parking space
const int iYellowState = 2;       // vehicle has not yet reached the parking zone - proceed forward
const int iRedState = 1;          // vehicle has passed through the parking zone - back up
const int iGreenState = 0;        // vehicle is within the parking zone - OK to stop

int iCurrentState = 0;            // controls whether or not to refresh LCD display
int iPrevState = -1;              // controls whether or not to refresh LCD display, set to -1 to ensure that it does not compare with 
                                  // any valid state during the first scan.  This ensures that LCD display will be painted the first time

int iDisplayMode = 0;             // variable holding user-specified display mode.  set default to zero = normal operation

int iFarEdgeGreen = 0;            // far boundary for acceptable parking distance
int iNearEdgeGreen = 0;           // near boundary for acceptable parking distance
                                  // range of 9 inches resulted in green zone of 33 inches due to angle of beam against windshield
                                  // factor of 3.66 green zone travel inches for each detected inch (12/17 experiment with Ford Explorer)  
int iPrevInches = 0;
int iGreenBandAddition = 0;       // used to dynamicaly control green band.  Once reading falls within green band then expand it to 1

int sensorPin = A0;               // select the input pin for the user-specified target "green zone"
int TargetDistance = 0;           // This is the target "green zone" as set by the user and read via analog input
long duration;                    // holds value of echo duration from distance sensor
int iInches;                      // distance to target in inches

void setup() {
  Serial.begin(9600);             // initialize serial communication:
  pinMode(ModePin, INPUT);        // prepare to read operating mode switch
  pinMode(trigPin, OUTPUT);       // setup ultrasonic sensor trigger pin as output
  pinMode(echoPin, INPUT);        // setup ultrasonic sensor echo pin as input
  lcd.begin(20,4);                // Identify the display type (char, lines)
  lcd.backlight();                // turn backlight on
}

void loop()
{
  iDisplayMode = digitalRead(ModePin);    // read the user selectable operation mode:
  // read desired distance, scale down by a factor of 1/10 to to give reading from 0 to 102
  TargetDistance = analogRead(sensorPin) / 10;
  iFarEdgeGreen = TargetDistance + 1; 
  iNearEdgeGreen = TargetDistance - 1;

  // The ultrasonic distance sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(echoPin, HIGH);

  // convert the pulse round trip time into a distance in inches
  iInches = (duration / 74 / 2);

  // discard outlying high values
  if ((iInches - iPrevInches) < 10)
    {
      if(iInches < (iNearEdgeGreen - iGreenBandAddition))
      // Vehicle is beyond forward limit of parking space - back up
      {
        iCurrentState = iRedState;      // save current state
        if(iDisplayMode == 0)
        {
          if (iCurrentState != iPrevState)  // refresh display only if necessary
                                            // enhancement:  move this check to outside of loop and check once for all states
          {
            lcd.clear();                    // clear screen
            lcd.setCursor(0,0);             // set cursor to (char,row)
            lcd.write("*****               ");      
            lcd.setCursor(0,1);             // set cursor to (char,row)
            lcd.write("********            ");      
            lcd.setCursor(0,2);             // set cursor to (char,row)
            lcd.write("********            ");      
            lcd.setCursor(0,3);             // set cursor to (char,row)
            lcd.write("*****               "); 

            iPrevState = iCurrentState;     // capture previous state             
          }        
        }
        else 
        {
          display_parameters();
        }
        iGreenBandAddition = 0;             // Not in green band,  Set to zero
      }
      if ((iInches <= (iFarEdgeGreen - iGreenBandAddition)) && (iInches >= (iNearEdgeGreen + iGreenBandAddition)))
      // Vehicle is within designated parking space - stop
      {
        iCurrentState = iGreenState;        // save current state
        if(iDisplayMode == 0)
        {
          if (iCurrentState != iPrevState)  // refresh display only if necessary
          {
            lcd.clear();                    // clear screen
            lcd.setCursor(0,0);             // set cursor to (char,row)
            lcd.write("        ***         ");      
            lcd.setCursor(0,1);             // set cursor to (char,row)
            lcd.write("        ***         ");      
            lcd.setCursor(0,2);             // set cursor to (char,row)
            lcd.write("        ***         ");      
            lcd.setCursor(0,3);             // set cursor to (char,row)
            lcd.write("        ***         "); 
            
            iPrevState = iCurrentState;     // capture previous state
          }     
        }
        else
        {
          display_parameters();
        }
        iGreenBandAddition = 1;           // use this value to dynamically control with width of the green band, when in the green band
                                          // expand green band by + or - 1
      }
      if(iInches > (iFarEdgeGreen + iGreenBandAddition))
      // Vehicle is not completely within parking space - proceed forward
      {
        iCurrentState = iYellowState;       // save current state
        if(iDisplayMode == 0)
        {
          if (iCurrentState != iPrevState)  // refresh display only if necessary
          {
            lcd.clear();                    // clear screen
            lcd.setCursor(0,0);             // set cursor to (char,row)
            lcd.write("               *****");      
            lcd.setCursor(0,1);             // set cursor to (char,row)
            lcd.write("            ********");      
            lcd.setCursor(0,2);             // set cursor to (char,row)
            lcd.write("            ********");      
            lcd.setCursor(0,3);             // set cursor to (char,row)
            lcd.write("               *****");

            iPrevState = iCurrentState;     // capture previous state
          } 
        }
        else
        {
          display_parameters();
        }
        iGreenBandAddition = 0;             // Not in green band, set to zero
      }
    }
  else
  {
      Serial.println("  ignoring value");
  }

  iPrevInches = iInches;
  delay(200);
}

void display_parameters(void)
{
  // display values on LCD screen
  lcd.clear();                              // clear screen
  lcd.setCursor(0,0);                       // set cursor to (char,row)
  lcd.write("Target:       ");  
  lcd.print(TargetDistance);      
  lcd.setCursor(0,1);                       // set cursor to (char,row)
  lcd.print("Reading:      ");  
  lcd.print(iInches);      
  lcd.setCursor(0,2);                       // set cursor to (char,row)
  lcd.write("Current Zone: "); 
  lcd.print(iCurrentState);         
  lcd.setCursor(0,3);                       // set cursor to (char,row)
  lcd.write("Software:     ");  
  lcd.print(iSWVersion);
   
  print_parameters();                       // dispaly values on serial monitor

  iPrevState = -1;                          // reset iPrevState so LCD display will redraw after exiting this routine
}

void print_parameters(void)
{
  // display values on serial monitor
  Serial.print("  Target: ");
  Serial.print(TargetDistance);
  Serial.print("  Reading: ");
  Serial.print(iInches);
  Serial.print("  Zone: ");  
  Serial.print(iCurrentState); 
  Serial.print("  SW Version: ");  
  Serial.println(iSWVersion);   
}
