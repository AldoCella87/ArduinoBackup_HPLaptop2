
//  1/15/19  AE  revised parking indicator based on Dec 30 2018 version.
// changed LCD to use single jumbo RYG LED with PWM control

void print_parameters(void);
void DisplayRed(void);
void DisplayGreen(void);
void displayYellow(void);

// revised version installed 1/1/19 - Volvo XC90 (north garage)

const int iSWVersion = 2;         // Software version number

const int trigPin = 2;            // for ultrasonic sensor
const int echoPin = 4;            // for ultrasonic sensor

                                  // codes which represent current state of vehicle within the parking space
const int iYellowState = 2;       // vehicle has not yet reached the parking zone - proceed forward
const int iRedState = 1;          // vehicle has passed through the parking zone - back up
const int iGreenState = 0;        // vehicle is within the parking zone - OK to stop

int iDisplayMode = 0;             // variable holding user-specified display mode.  set default to zero = normal operation

int iFarEdgeGreen = 0;            // far boundary for acceptable parking distance
int iNearEdgeGreen = 0;           // near boundary for acceptable parking distance
                                  // range of 9 inches resulted in green zone of 33 inches due to angle of beam against windshield
                                  // factor of 3.66 green zone travel inches for each detected inch (12/17 experiment with Ford Explorer)  
int iPrevInches = 0;
int iGreenBandAddition = 0;       // used to dynamicaly control green band.  Once reading falls within green band then expand it to 1

int GreenDistanceInputPin = A0;   // select the input pin for the user-specified target "green point"
int TargetDistance = 0;           // This is the target "green point" as set by the user and read via analog input
long duration;                    // holds value of echo duration from distance sensor
int iInches;                      // distance to target in inches

// pins for RGB LED
int RedLED = 3;                   // the PWM pin the red LED is attached to
int GreenLED = 5;                 // the PWM pin the green LED is attached to
int BlueLED = 6;                  // the PWM pin the blue LED is attached to

void setup() {
  Serial.begin(9600);             // initialize serial communication:
  //pins for ultrasonic detector
  pinMode(trigPin, OUTPUT);       // setup ultrasonic sensor trigger pin as output
  pinMode(echoPin, INPUT);        // setup ultrasonic sensor echo pin as input

  // pins for RGB LED
  pinMode(RedLED, OUTPUT);  
  pinMode(GreenLED, OUTPUT);  
  pinMode(BlueLED, OUTPUT);
  
}

void loop()
{
  // read desired green distance point as specified by user, scale down by a factor of 1/10 to to give reading from 0 to 102
  TargetDistance = analogRead(GreenDistanceInputPin) / 10;


  // create green zone based on user specified green point
  iFarEdgeGreen = TargetDistance + 1; 
  iNearEdgeGreen = TargetDistance - 1;

  // get a distance reading
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
        DisplayRed();       
        iGreenBandAddition = 0;             // Not in green band,  Set to zero
      }
      if ((iInches <= (iFarEdgeGreen - iGreenBandAddition)) && (iInches >= (iNearEdgeGreen + iGreenBandAddition)))
      // Vehicle is within designated parking space - stop
      {
        DisplayGreen();     
        iGreenBandAddition = 1;           // use this value to dynamically control with width of the green band, when in the green band
                                          // expand green band by + or - 1
      }
      if(iInches > (iFarEdgeGreen + iGreenBandAddition))
      // Vehicle is not completely within parking space - proceed forward
      {
        DisplayYellow();
        iGreenBandAddition = 0;             // Not in green band, set to zero
      }
    }
  else
  {
      Serial.println("  ignoring value");
  }

  print_parameters();
  iPrevInches = iInches;
  delay(200);
}

void print_parameters(void)
{
  // display values on serial monitor
  Serial.print("  Target: ");
  Serial.print(TargetDistance);
  Serial.print("  Reading: ");
  Serial.print(iInches);
  Serial.print("  SW Version: ");  
  Serial.println(iSWVersion);   
}

void DisplayRed()
{
  // red
  analogWrite(RedLED, 255);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 0);
}

void DisplayYellow()
{
  // yellow
  analogWrite(RedLED, 255);
  analogWrite(GreenLED, 40);
  analogWrite(BlueLED, 0);
}

void DisplayGreen()
{
  // green
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 155);
  analogWrite(BlueLED, 0);
}
