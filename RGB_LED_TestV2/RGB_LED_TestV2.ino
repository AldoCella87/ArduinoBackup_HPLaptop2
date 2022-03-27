
//  1/15/19  AE  test RGB LED


void DisplayRed(void);
void DisplayGreen(void);
void DisplayYellow(void);
void DisplayBlue(void);


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
 DisplayRed();
 delay(2000);
 DisplayGreen();
 delay(2000);
 DisplayYellow();
 delay(2000);
 DisplayBlue();
 delay(2000);
  delay(200);
}



void DisplayRed()
{
  // red
  analogWrite(RedLED, 255);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 0);
}

void DisplayBlue()
{
  // blue
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 255);
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
