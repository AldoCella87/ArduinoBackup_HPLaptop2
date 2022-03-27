
#define OUT_PIN 13

long liStartTime = 0;
void setup() {
  // put your setup code here, to run once:
 liStartTime = millis();

pinMode(OUT_PIN, OUTPUT);
digitalWrite(OUT_PIN, HIGH);

}

void loop() {
  // put your main code here, to run repeatedly:

if (millis() > liStartTime + 5000)  digitalWrite(OUT_PIN, LOW);



}
