volatile int IRQcount;
int pin = 5;
int pin_irq = 0; //IRQ that matches to pin 2

void setup() {
  // Put your setup code here, to run once:
  Serial.begin (9600);
}

void IRQcounter() {
   IRQcount++;
}

void loop() {
  // Put your main code here, to run repeatedly:

  attachInterrupt(pin_irq, IRQcounter, RISING);
  delay(25);
  detachInterrupt(pin);
  Serial.print(F("Counted = "));
  Serial.println(IRQcount);
}
