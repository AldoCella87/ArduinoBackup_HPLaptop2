
// 11/17/19 AE - count pulses per second using reflective IR sensor
// sensor TCRT5000 (top view)    phototransistor (PT) is black, IR LED is blue
//      
//       coll   anode
//      |--------\
//      | PT   LED
//      |--------/
//       emit   cathode
//
// use 200 ohm resistor between anode and +5V
// use 5K ohm resistor between coll(ector) and +5V
// emit(ter) and cathode go to ground
// collector goes to input



int PulsePin = 2; 

int iAnalogValue = 0;
volatile unsigned int pulse; 
 
void setup() 
{ 
Serial.begin(9600); 
 
pinMode(PulsePin, INPUT); 
attachInterrupt(0, count_pulse, RISING); 
} 
 
void loop() 
{ 
iAnalogValue = analogRead(A0);

pulse=0; 
interrupts(); 
delay(1000); 
noInterrupts(); 
 
Serial.print("Pulses per second: "); 
Serial.print("   ");
Serial.println(pulse); 
} 
 
void count_pulse() 
{ 
pulse++; 
} 
