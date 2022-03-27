/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Fade
*/

int RedLED = 3;           // the PWM pin the LED is attached to
int GreenLED = 5;           // the PWM pin the LED is attached to
int BlueLED = 6;           // the PWM pin the LED is attached to

int RedBrightness = 255;    // how bright the LED is
int GreenBrightness =00 ;    // how bright the LED is
int BlueBrightness = 20;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(RedLED, OUTPUT);  
  pinMode(GreenLED, OUTPUT);  
  pinMode(BlueLED, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {

  // red
  RedBrightness = 255;    // how bright the LED is
  GreenBrightness =00 ;    // how bright the LED is
  BlueBrightness = 00;    // how bright the LED is
  analogWrite(RedLED, RedBrightness);
  analogWrite(GreenLED, GreenBrightness);
  analogWrite(BlueLED, BlueBrightness);
  delay(2000);

  // yellow
  RedBrightness = 255;    // how bright the LED is
  GreenBrightness =20 ;    // how bright the LED is
  BlueBrightness = 0;    // how bright the LED is
  analogWrite(RedLED, RedBrightness);
  analogWrite(GreenLED, GreenBrightness);
  analogWrite(BlueLED, BlueBrightness);
  delay(2000);

  // green
  RedBrightness = 0;    // how bright the LED is
  GreenBrightness =155;    // how bright the LED is
  BlueBrightness = 0;    // how bright the LED is
  analogWrite(RedLED, RedBrightness);
  analogWrite(GreenLED, GreenBrightness);
  analogWrite(BlueLED, BlueBrightness);
  delay(2000);
  
  // blue
  RedBrightness = 0;    // how bright the LED is
  GreenBrightness =0;    // how bright the LED is
  BlueBrightness = 200;    // how bright the LED is
  analogWrite(RedLED, RedBrightness);
  analogWrite(GreenLED, GreenBrightness);
  analogWrite(BlueLED, BlueBrightness);
  delay(2000);



/****
  // change the brightness for next time through the loop:
  RedBrightness = RedBrightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (RedBrightness <= 0 || RedBrightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
  **********/
}
