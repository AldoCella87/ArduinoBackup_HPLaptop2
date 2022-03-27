
#define Heart 3                            //Attach the Grove Ear-clip sensor to digital pin 2.
#define LED 13                              //Attach an LED to digital pin 4

boolean beat = false;                      /* This "beat" variable is used to control the timing of the Serial communication
                                           so that data is only sent when there is a "change" in digital readings. */

//==SETUP==========================================================================================
void setup() {
  Serial.begin(9600);                     //Initialise serial communication
  pinMode(Heart, INPUT);                  //Set digital pin 2 (heart rate sensor pin) as an INPUT
  pinMode(LED, OUTPUT);                   //Set digital pin 4 (LED) to an OUTPUT
}


//==LOOP============================================================================================
void loop() {
  if(digitalRead(Heart)>0){               //The heart rate sensor will trigger HIGH when there is a heart beat
    if(!beat){                            //Only send data when it first discovers a heart beat - otherwise it will send a high value multiple times
      beat=true;                          //By changing the beat variable to true, it stops further transmissions of the high signal
      digitalWrite(LED, HIGH);            //Turn the LED on 
      Serial.println(1023);               //Send the high value to the computer via Serial communication.
    }
  } else {                                //If the reading is LOW, 
    if(beat){                             //and if this has just changed from HIGH to LOW (first low reading)
      beat=false;                         //change the beat variable to false (to stop multiple transmissions)
      digitalWrite(LED, LOW);             //Turn the LED off.
      Serial.println(0);                  //then send a low value to the computer via Serial communication.
    }
  }
}
