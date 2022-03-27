
// pins for RGB LED
int RedLED = 3;                   // the PWM pin the red LED is attached to
int GreenLED = 5;                 // the PWM pin the green LED is attached to
int BlueLED = 6;                  // the PWM pin the blue LED is attached to

void setup() {

  // pins for RGB LED
  pinMode(RedLED, OUTPUT);  
  pinMode(GreenLED, OUTPUT);  
  pinMode(BlueLED, OUTPUT);
  
}

void loop()
{


   // display 0000red
  analogWrite(RedLED, 255);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 0);
  delay(2000);
  // display green
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 255);
  analogWrite(BlueLED, 0);
  delay(2000);
  // display blue
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 255);
  delay(2000);
  // display my custom color
  analogWrite(RedLED, 200);
  analogWrite(GreenLED, 60);
  analogWrite(BlueLED, 40);
  delay(2000);
  // turn off all LEDs
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 0);
  delay(1000);
  
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
