// 7/16/21 - corrected delay time (was int/ should be long.  added all letters and numbers, changed error handling, add amazon gift card 
// 1/17/17 - Louie - here is the code which is loaded on the Arduino which I sent you.  -Al
// created message matrix 12/28/15 AE
// 1/17/17 activated random behavior, added separate LED output, removed #defines AE

int tonePin = 3;
int LEDPin = 2;



char sMessage[] = {

                   // message from Albert A 
                   '_','V','V','V',' ',' ', 'H','E','L','L','O',' ',' ',
                   'T','N','X',' ','F','R',' ','F','B',' ','Q','S','O',' ','B','E','S','T',' ',
                   '7','3',' ','E','T',' ','8','8',' ','T','O',' ','A','L','L',' ','D','E',' ',
                   'W','3','>','I','P',' ','!',


                   // Message from Maria C
                   ' ',' ',' ','_',
                   'N','O','N','A',' ','S','A','Y','S',' ','D','O','N','T',' ','F','O','R','G','E','T',' ','T','O',' ','L','O','V','E',' ',
                   'E','A','C','H',' ',
                   'O','T','H','E','R',' ',' ','A','N','D',' ','T','H','A','T','S',' ','A','N',' ','O','R','D','E','R',' ',
                   

                   // message from Louis J
                   ' ',' ',' ','_',
                   'H','E','A','R','T','Y',' ','G','R','E','E','T','I','N','G','S',' ','F','R','O','M',' ','D','I','G','I','T','A','L',' ',
                   'L','O','U',' ',' ',
                   'P','A','P','A',' ','W','O','O','D','Y',' ','S','A','Y','S',' ','N','E','E','E','V','E','R',' ','B','E','T','T','E','R',
                   
                   // gift certificate number
                   ' ',' ',' ','_',
                   'U','S','E',' ','T','H','I','S',' ','C','O','D','E',' ',
                   'A','T',' ','J','E','F','F','S',' ','P','L','A','C','E',' ',' ',

                   'C','O','D','E',' ','I','S',' ','O','N','L','Y',' ','V','A','L','I','D',' ','F','O','R',' ',
                   'O','N','E',' ','U','S','E',' ','F','I','R','S','T',' ','C','O','M','E',' ',
                   'F','I','R','S','T',' ','S','E','R','V','E','D',' ',' ','G','O','O','D',' ','L','U','C','K',' ',' ',

                   'A','B','C','D','-',' ',
                   'E','F','G','H','-',' ',
                   'I','J','K','L','-',' ',
                   'M','N','O','P','-',' ',

                   'T','E','X','T',' ','T','O',' ','9','8','9',' ','3','9','7',' ','3','8','4','7',' ','W','H','E','N',' ','Y','O','U',' ','H','A','V','E',' ', 
                   'S','U','C','C','E','S','S','F','U','L','L','Y',' ','R','E','D','E','E','M','E','D',' ','T','H','I','S',' ','A','W','A','R','D',
                   };



                   
int freq;
int dotPeriod;
int dahPeriod;
int relaxtime;
int letterSpace;
int wordSpace;  
unsigned long delayTime;

void setup() {

pinMode(LEDPin, OUTPUT);
digitalWrite(LEDPin, LOW);

pinMode(tonePin, OUTPUT);
Serial.begin(9600);

}

void loop() {
  int i;
  // establish random behavior
  freq = random(600,1100);
  dotPeriod = random(70,100);
  dahPeriod = (dotPeriod *3);
  relaxtime = (dotPeriod);
  letterSpace = (dotPeriod *3);   // standard letter space is 3x dot length  
  wordSpace = (dotPeriod *7);     // standard word space is 7x dot length  
  delayTime = random(60000,300000);

  // display results of random variable assignments
  Serial.println();
  Serial.println("Digital Memorial for the Evangelista Family  8/15/21");
  Serial.println();
  Serial.print("frequency:  ");
  Serial.println(freq);
  Serial.print("dotPeriod:  ");
  Serial.println(dotPeriod);
  Serial.print("dahPeriod:  ");
  Serial.println(dahPeriod);
  Serial.print("delayTime:  "); 
  Serial.println(delayTime);

  // send message
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
    // change order to put frequently used characters at top of switch if desired.
    switch (x){
      case 'A':
        di();dah(); return;
      case 'B':
        dah();di();di();dit(); return;
      case 'C':
        dah();di();dah();dit();return;    
      case 'D':
        dah();di();dit(); return;      
      case 'E':
        dit(); return;
      case 'F':
        di();di();dah();dit(); return;
      case 'G':
        dah();dah();dit(); return;        
      case 'H':
        di();di();di();dit(); return;
      case 'I':
        di();dit(); return;
      case 'J':
        di();dah();dah();dah(); return;
      case 'K':
        dah();di();dah(); return;
      case 'L':
        di();dah();di();dit(); return;
      case 'M':
        dah();dah(); return;
      case 'N':
        dah();dit(); return;
      case 'O':
        dah();dah();dah(); return;
      case 'P':
        di();dah();dah();dit(); return;     
      case 'Q':
        dah();dah();di();dah(); return;
      case 'R':
        di();dah();dit(); return;
      case 'S':
        di();di();dit(); return;
      case 'T':
        dah(); return;
      case 'U':
        di();di();dah(); return;
      case 'V':
        di();di();di();dah(); return;                
      case 'W':
        di();dah();dah(); return;
      case 'X':
        dah();di();di();dah(); return;
      case 'Y':
        dah();di();dah();dah(); return;        
      case 'Z':
        dah();dah();di();dit(); return;
      case '-':
        dah();di();di();di();dah(); return; 
      case '_':
        dahhh(); return; 
      case '>':
        dah();dahh();di();dit(); return;  //custom Z for the Zipper        
      case '!': // stands in for SK (end of transmission / Silent Key)  with special emphasis on last dah
        di();di();dit();dah();di();FinalDah(); return;
      case '.':
        di();dah();di();dah();di();dah(); return;

      case '1':
        di();dah();dah();dah();dah(); return;
      case '2':
        di();di();dah();dah();dah(); return;
      case '3':
        di();di();di();dah();dah(); return; 
      case '4':
        di();di();di();di();dah(); return; 
      case '5':
        di();di();di();di();dit(); return; 
      case '6':
        dah();di();di();di();dit(); return;   
      case '7':
        dah();dah();di();di();dit(); return;
      case '8':
        dah();dah();dah();di();dit(); return;
      case '9':
        dah();dah();dah();dah();dit(); return;
      case '0':
        dah();dah();dah();dah();dah(); return;        
      case ' ':
        delay(wordSpace); return;
      default:
        // signify error
        di();di();di();di();di();di();di();dit(); return; 
        
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

void dahhh()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod*6);
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
