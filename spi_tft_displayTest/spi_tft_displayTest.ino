//https://randomnerdtutorials.com/guide-to-1-8-tft-display-with-arduino/
/*
 * Rui Santos 
 * Complete Project Details http://randomnerdtutorials.com
 */

// include TFT and SPI libraries
#include <TFT.h>  
#include <SPI.h>

// pin definition for Arduino UNO
#define cs   10
#define dc   9
#define rst  8


// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);

void setup() {

  //initialize the library
  TFTscreen.begin();

  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);
  //set the text size
  TFTscreen.setTextSize(2);

    TFTscreen.background(0, 0, 255);
  delay(2000);
  TFTscreen.background(0, 255, 0);
  delay(2000);
  TFTscreen.background(255, 255, 0);
  delay(2000);
 
}

void loop() {


  
  TFTscreen.background(0, 0, 255);
  delay(2000);
  TFTscreen.background(0, 255, 0);
  delay(2000);
  TFTscreen.background(255, 255, 0);
  delay(2000);
  
  }
