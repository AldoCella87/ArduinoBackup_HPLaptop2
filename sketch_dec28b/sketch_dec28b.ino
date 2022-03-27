// adapted from http://revryl.com/2012/03/31/morse-code-gen/
// corrected index control and case statement structure 12/28/15 AE
// modified to handle canned message
//define freq 600
//define dotPeriod 50
#define tonePin 3
#define dahPeriod (dotPeriod *3)
#define relaxtime (dotPeriod)
#define letterSpace (dotPeriod *2)
#define wordSpace (dotPeriod *4)

int freq;
int dotPeriod;
int delayTime;
char sMessage[] = {'T','N','X',' ','F','R',' ','F','B',' ','Q','S','O',' ','B','E','S','T',' ',
                   '7','3',' ','E','T',' ','8','8',' ','T','O',' ','A','L','L',' ','D','E',' ',
                   'Z','I','P','P','E','R',' ','!'};

void setup() {

Serial.begin(9600);
pinMode(tonePin, OUTPUT);

}

void loop() {
  int i;
  // for audio mode use random values below
  //freq = random(500,1017);
  //dotPeriod = random(40,85);
  //delayTime = random(3321,1221);

  // for LED output mode use fixed values below
  freq = 900;
  dotPeriod = 160;
  delayTime = 5000;

  Serial.println();
  Serial.println();
  Serial.println(freq);
  Serial.println(dotPeriod);
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
        dah();dah();di();dit(); return;
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
  tone(tonePin, freq);
  delay(dotPeriod);
  noTone(tonePin);
  delay(relaxtime);
}

void dah()
{
  tone(tonePin, freq);
  delay(dahPeriod);
  noTone(tonePin);
  delay(relaxtime);
}
  
void FinalDah()
{
  tone(tonePin, freq);
  delay(dahPeriod * 3);
  noTone(tonePin);
  delay(relaxtime);
}

void di()
{
  dit();
}



