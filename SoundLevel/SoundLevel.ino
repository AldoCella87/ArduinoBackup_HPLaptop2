/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/
// from: https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels


int iMeasureSound();

//const int sampleWindow = 100; // Sample window width in mS (50 mS = 20Hz)
#define SOUND_SAMPLE_WINDOW 100
unsigned int sample;
void setup() 
{
   Serial.begin(9600);
}
 
 
void loop() 
{

 
   Serial.println(iMeasureSound());
}

// adapted from: 
// https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels
int iMeasureSound()
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;

   while (millis() - startMillis < SOUND_SAMPLE_WINDOW)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   return(peakToPeak);
}
