// 1/17/17 - Louie - here is the code which is loaded on the Arduino which I sent you.  -Al

// adapted from http://revryl.com/2012/03/31/morse-code-gen/
// created message matrix 12/28/15 AE
// 1/17/17 activated random behavior, added separate LED output, removed #defines AE

int tonePin = 13;
int LEDPin = 2;
char sMessage[] = {'T','N','X',' ','F','R',' ','F','B',' ','Q','S','O',' ','B','E','S','T',' ',
                   '7','3',' ','E','T',' ','8','8',' ','T','O',' ','A','L','L',' ','D','E',' ',
                   'W','3','Z','I','P',' ','!'};
int freq;
int dotPeriod;
int dahPeriod;
int relaxtime;
int letterSpace;
int wordSpace;  
int delayTime;

void setup() {

pinMode(LEDPin, OUTPUT);
digitalWrite(LEDPin, LOW);

pinMode(tonePin, OUTPUT);
Serial.begin(9600);

}

void loop() {
  int i;
  // establish random behavior
  freq = random(400,1000);
  dotPeriod = random(40,90);
  dahPeriod = (dotPeriod *3);
  relaxtime = (dotPeriod);
  letterSpace = (dotPeriod *2);
  wordSpace = (dotPeriod *4);  
  delayTime = random(10000,50000);

  // display results of random variable assignments
  Serial.println();
  Serial.println("W3ZIP 'The Zipper' Digital Memorial");
  Serial.println();
  Serial.print("frequency:  ");
  Serial.println(freq);
  Serial.print("dotPeriod:  ");
  Serial.println(dotPeriod);
  Serial.print("dahPeriod:  ");
  Serial.println(dahPeriod);
  Serial.print("delayTime:  "); 
  Serial.println(delayTime);

  // send canned message
  for(i=0;i<sizeof(sMessage);i++)
  {
    playLetter(sMessage[i]);
    Serial.write(sMessage[i]);
    delay(letterSpace);
  }
  delay(delayTime);
}

void playLetter(char x)
  {
    switch (x){
      case 'E':
        dit(); return;
      case 'T':
        dah(); return;
      case 'A':
        di();dah(); return;
      case 'I':
        di();dit(); return;
      case 'O':
        dah();dah();dah(); return;
      case 'D':
        dah();di();dit(); return;
      case 'R':
        di();dah();dit(); return;
      case 'W':
        di();dah();dah(); return;
      case 'C':
        dah();di();dah();dit();return;
      case 'Q':
        dah();dah();di();dah(); return;
      case 'L':
        di();dah();di();dit(); return;
      case '7':
        dah();dah();di();di();dit(); return;
      case '8':
        dah();dah();dah();di();dit(); return;
      case '3':
        di();di();dit();dah();dah(); return;
      case 'B':
        dah();di();di();dit(); return;
      case 'H':
        di();di();di();dit(); return;
      case 'S':
        di();di();dit(); return;
      case 'F':
        di();di();dah();dit(); return;
      case 'N':
        dah();dit(); return;
      case 'M':
        dah();dah(); return;
      case 'X':
        dah();di();di();dah(); return;
      case 'P':
        di();dah();dah();dit(); return;
      case 'Z':
        dah();dahh();di();dit(); return;  //custom Z for the Zipper
      case '!':
        di();di();dit();dah();di();FinalDah(); return;
      case '.':
        di();dah();di();dah();di();dah(); return;
 
      case ' ':
        delay(wordSpace); return;
      default:
        ErrInd(); return;
    }
  }
   
void ErrInd()
{
  tone(tonePin, freq / 10);
  delay(dahPeriod);
  noTone(tonePin);
  delay(relaxtime);
}

void dit()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dotPeriod);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void dah()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void dahh()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod);
  delay(dahPeriod);  //extra long dah for call sign, characteristic of the Zipper
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}
  
void FinalDah()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod * 3);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void di()
{
  dit();
}



