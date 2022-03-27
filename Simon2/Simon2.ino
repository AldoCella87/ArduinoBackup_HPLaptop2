 
#define MAX_CHALLENGE_LENGTH  10      // maximum number of elements in sequence array
#define SOUND_CORRECT 0       // mode control to command audio sound signifying correct response
#define SOUND_INCORRECT 1     // mode control to command audio sound signifying correct response
#define SOUND_SUCCESSFUL_ROUND 2  // audio to signify successful round
#define END_OF_CHALLENGE_FLAG 99  // signifies end of challenge array
#define NUM_BUTTONS 5             // number of player buttons

  int iReading1 = 0;
  int iReading2 = 0;
  int iReading3 = 0;
  int iReading4 = 0;
  int iReading5 = 0;
  
  int iPrevReading1 = 0;
  int iPrevReading2 = 0;
  int iPrevReading3 = 0;
  int iPrevReading4 = 0;
  int iPrevReading5 = 0;
    
  int iResponseIndex = 0;

int buttonPins[NUM_BUTTONS] = {2, 3, 4, 5, 6};
const int ledPins[NUM_BUTTONS] = {9, 10, 11, 12, 13};


// constants won't change. They're used here to set pin numbers:
const int buttonPin1 = 2;     // the number of the pushbutton pin
const int buttonPin2 = 3;     // the number of the pushbutton pin
const int buttonPin3 = 4;     // the number of the pushbutton pin
const int buttonPin4 = 5;     // the number of the pushbutton pin
const int buttonPin5 = 6;     // the number of the pushbutton pin

const int alertPin = 7;       // sonalert
const int ledPin1 =  9;      // the number of the LED pin
const int ledPin2 =  10;      // the number of the LED pin
const int ledPin3 =  11;      // the number of the LED pin
const int ledPin4 =  12;      // the number of the LED pin
const int ledPin5 =  13;      // the number of the LED pin


const unsigned int debounceDelay = 150;  
int challengeSequence[MAX_CHALLENGE_LENGTH+1];           //light sequence array, reserve one space for end flag

int iResponses[MAX_CHALLENGE_LENGTH];         // array to hold sequence of responses


unsigned long PositiveTransitionTime1;      // point in time where transition was detected
unsigned long PositiveTransitionTime2;      // point in time where transition was detected
unsigned long PositiveTransitionTime3;      // point in time where transition was detected
unsigned long PositiveTransitionTime4;      // point in time where transition was detected
unsigned long PositiveTransitionTime5;      // point in time where transition was detected


unsigned long previousMillis1 = 0;   // save point in time 
unsigned long previousMillis2 = 0;   // save point in time 
unsigned long previousMillis3 = 0;   // save point in time 
unsigned long previousMillis4 = 0;   // save point in time 
unsigned long previousMillis5 = 0;   // save point in time 


// variables will change:
int button1State = 0;         // variable for reading the pushbutton status
int button2State = 0;         // variable for reading the pushbutton status
int button3State = 0;         // variable for reading the pushbutton status
int button4State = 0;         // variable for reading the pushbutton status
int button5State = 0;         // variable for reading the pushbutton status

bool bDisplaySeqn = true;     // displays light sequence when true, waits for response when false 


bool bButton1 = false;
bool bButton2 = false;
bool bButton3 = false;
bool bButton4 = false;
bool bButton5 = false;



struct ResponseStruct{
  int iReading;
  int iPrevReading;
  unsigned long PositiveTransitionTime;
  unsigned long PreviousMillis;
  bool bButtonState;
};

ResponseStruct Responses[MAX_CHALLENGE_LENGTH];



void setup() {
  // initialize the LED pins as an output:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
  
  pinMode(alertPin, OUTPUT);
          
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(buttonPin4, INPUT);
  pinMode(buttonPin5, INPUT);

  Serial.begin(9600);
}

void loop() {

  if (bDisplaySeqn){
    // setup sequence
    for(int i = 0; i < MAX_CHALLENGE_LENGTH + 1; i++) challengeSequence[i] = END_OF_CHALLENGE_FLAG; 
    challengeSequence[0] = ledPin1;
    challengeSequence[1] = ledPin2;
    challengeSequence[2] = ledPin3;
    challengeSequence[3] = ledPin4;
    challengeSequence[4] = ledPin5;
    challengeSequence[5] = ledPin5;
    challengeSequence[6] = ledPin4;
    challengeSequence[7] = ledPin3;
    challengeSequence[8] = ledPin2;
    challengeSequence[9] = ledPin1;
        
    // display sequence
    DisplaySequence(challengeSequence, 300);   
    bDisplaySeqn = false;

    // prepare to receive responses
    iResponseIndex = 0;     // initialize index which stores series of responses 
  
    // initialize button response booleans
    /**
    bButton1 = false;
    bButton2 = false;
    bButton3 = false;
    bButton4 = false;
    bButton5 = false;
    **/
    
    for(int i = 0; i<NUM_BUTTONS; i++) Responses[i].bButtonState = false;
  

    
  }
  else
  {
  // wait for response

  // read the state of the pushbuttons
  iReading1 = digitalRead(buttonPin1);
  iReading2 = digitalRead(buttonPin2);
  iReading3 = digitalRead(buttonPin3);
  iReading4 = digitalRead(buttonPin4);
  iReading5 = digitalRead(buttonPin5);








for(int j = 0; j<NUM_BUTTONS; j++)
{

  /*
   * Process Buttons 
   * 
   */
  Responses[j].iReading = digitalRead(buttonPins[j]);
  if ((Responses[j].iReading == HIGH) && (Responses[j].iPrevReading == LOW)) {
    // get time of newly detected high state
    Responses[j].PositiveTransitionTime = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((Responses[j].iReading == HIGH) && (Responses[j].iPrevReading == HIGH)){
    if (((millis() - Responses[j].PositiveTransitionTime) > debounceDelay && (Responses[j].bButtonState == false))) {
         Responses[j].bButtonState = true;
         Serial.print("Button state ON registered for button ");
         Serial.println(j);
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPins[j];             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPins[j]){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);                   // provide affirmative sound
            digitalWrite(ledPins[j], HIGH);                    // light LED
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bDisplaySeqn = true;                            // player fails this round, redisplay challenge
          }
          // we have a response
          iResponseIndex++;                                 // prepare to store next response
         }
         else
         {
            // guard against this...
            Serial.println("MAX_CHALLENGE_LENGTH exceeded");
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
         }
    }
  }
  else if ((Responses[j].iReading == LOW) && (Responses[j].iPrevReading == HIGH)){
    Responses[j].bButtonState = false;      // player has released the button, reset to allow it to be registered again
    digitalWrite(ledPins[j], LOW);                    // extinguish LED
    // check for end of sequence
    if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG) bDisplaySeqn = true;  // exit responses and display new challenge
  }

  Responses[j].iPrevReading = Responses[j].iReading;  // save current reading as previous
 }


/******************************************************** 

  // process button 1
  if ((iReading1 == HIGH) && (iPrevReading1 == LOW)) {
    // get time of newly detected high state
    PositiveTransitionTime1 = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((iReading1 == HIGH) && (iPrevReading1 == HIGH)){
    if (((millis() - PositiveTransitionTime1) > debounceDelay && (bButton1 == false))) {
         bButton1 = true;
         Serial.println("Button 1 ON state registered");
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPin1;             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPin1){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);                   // provide affirmative sound
            digitalWrite(ledPin1, HIGH);                    // light LED
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bDisplaySeqn = true;                            // player fails this round, redisplay challenge
          }
          // we have a response
          iResponseIndex++;                                 // prepare to store next response
         }
         else
         {
            // guard against this...
            Serial.println("MAX_CHALLENGE_LENGTH exceeded");
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
         }
    }
  }
  else if ((iReading1 == LOW) && (iPrevReading1 == HIGH)){
    bButton1 = false;      // player has released the button, reset to allow it to be registered again
    digitalWrite(ledPin1, LOW);                    // extinguish LED
    // check for end of sequence
    if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG) bDisplaySeqn = true;  // exit responses and display new challenge
  }
  
 

  // process button 2
  if ((iReading2 == HIGH) && (iPrevReading2 == LOW)) {
    // get time of newly detected high state
    PositiveTransitionTime2 = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((iReading2 == HIGH) && (iPrevReading2 == HIGH)){
    if (((millis() - PositiveTransitionTime2) > debounceDelay && (bButton2 == false))) {
         bButton2 = true;
         Serial.println("Button 2 ON state registered");
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPin2;             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPin2){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);
            digitalWrite(ledPin2, HIGH);                    // light LED
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bDisplaySeqn = true;                            // player fails this round, redisplay challenge            
          }
          iResponseIndex++;                                 // prepare to store next response
         }
         else
         {
            // guard against this...
            Serial.println("MAX_CHALLENGE_LENGTH exceeded");
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
         }
    }
  }
  else if ((iReading2 == LOW) && (iPrevReading2 == HIGH)){
    bButton2 = false;      // player has released the button, reset to allow it to be registered again
    digitalWrite(ledPin2, LOW);                    // extinguish LED
    // check for end of sequence
    if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG) bDisplaySeqn = true;  // exit responses and display new challenge
                 
  }
 

  // process button 3
  if ((iReading3 == HIGH) && (iPrevReading3 == LOW)) {
    // get time of newly detected high state
    PositiveTransitionTime3 = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((iReading3 == HIGH) && (iPrevReading3 == HIGH)){
    if (((millis() - PositiveTransitionTime3) > debounceDelay && (bButton3 == false))) {
         bButton3 = true;
         Serial.println("Button 3 ON state registered");
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPin3;             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPin3){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);
            digitalWrite(ledPin3, HIGH);                    // light LED
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bDisplaySeqn = true;                            // player fails this round, redisplay challenge            
          }
          iResponseIndex++;                                 // prepare to store next response
         }
         else
         {
            // guard against this...
            Serial.println("MAX_CHALLENGE_LENGTH exceeded");
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
         }
    }
  }
  else if ((iReading3 == LOW) && (iPrevReading3 == HIGH)){
    bButton3 = false;                 // player has released the button, reset to allow it to be registered again
    digitalWrite(ledPin3, LOW);       // extinguish LED     
    // check for end of sequence
    if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG) bDisplaySeqn = true;  // exit responses and display new challenge
  }
 

  // process button 4
  if ((iReading4 == HIGH) && (iPrevReading4 == LOW)) {
    // get time of newly detected high state
    PositiveTransitionTime4 = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((iReading4 == HIGH) && (iPrevReading4 == HIGH)){
    if (((millis() - PositiveTransitionTime4) > debounceDelay && (bButton4 == false))) {
         bButton4 = true;
         Serial.println("Button 4 ON state registered");
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPin4;             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPin4){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);
            digitalWrite(ledPin4, HIGH);                    // light LED            
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bDisplaySeqn = true;                            // player fails this round, redisplay challenge            
          }
          iResponseIndex++;                                 // prepare to store next response
         }
         else
         {
            // guard against this...
            Serial.println("MAX_CHALLENGE_LENGTH exceeded");
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
         }
    }
  }
  else if ((iReading4 == LOW) && (iPrevReading4 == HIGH)){
    bButton4 = false;             // player has released the button, reset to allow it to be registered again
    digitalWrite(ledPin4, LOW);                    // extinguish LED 
    // check for end of sequence
    if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG) bDisplaySeqn = true;  // exit responses and display new challenge
  }
 

  // process button 5
  if ((iReading5 == HIGH) && (iPrevReading5 == LOW)) {
    // get time of newly detected high state
    PositiveTransitionTime5 = millis();
  }
  // button has been pushed for at least one cycle,  accumulate time until we reach threshold
  else if ((iReading5 == HIGH) && (iPrevReading5 == HIGH)){
    if (((millis() - PositiveTransitionTime5) > debounceDelay && (bButton5 == false))) {
         bButton5 = true;
         Serial.println("Button 5 ON state registered");
         if(iResponseIndex < MAX_CHALLENGE_LENGTH){
          iResponses[iResponseIndex] = ledPin5;             // record activation of button 
          if(challengeSequence[iResponseIndex] == ledPin5){  // Requested button matches button played
            SoundFeedback(SOUND_CORRECT);
            digitalWrite(ledPin5, HIGH);                    // light LED            
          }
          else{
            SoundFeedback(SOUND_INCORRECT);
            bDisplaySeqn = true;                            // player fails this round, redisplay challenge            
          }
          iResponseIndex++;                                 // prepare to store next response
         }
         else
         {
            // guard against this...
            Serial.println("MAX_CHALLENGE_LENGTH exceeded");
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
            SoundFeedback(SOUND_INCORRECT);
         }
    }
  }
  else if ((iReading5 == LOW) && (iPrevReading5 == HIGH)){
    bButton5 = false;             // player has released the button, reset to allow it to be registered again
    digitalWrite(ledPin5, LOW);                    // extinguish LED 
    // check for end of sequence
    if(challengeSequence[iResponseIndex] == END_OF_CHALLENGE_FLAG) bDisplaySeqn = true;  // exit responses and display new challenge        
    }
 
  // save current pushbutton state as previous state for comparison during next cycle
  iPrevReading1 = iReading1;
  iPrevReading2 = iReading2;
  iPrevReading3 = iReading3;
  iPrevReading4 = iReading4;
  iPrevReading5 = iReading5;



  *********************/
  }


  
}

void DisplaySequence(int challengeElements[], int iDuration)
{
  int i = 0;
  while(challengeElements[i] != END_OF_CHALLENGE_FLAG) 
  {
    digitalWrite(challengeElements[i], HIGH);
    delay(iDuration);
    digitalWrite(challengeElements[i], LOW);
    delay(iDuration * 1.5); 
    i++;   
  }
}

void SoundFeedback(int iMode)
{
  switch(iMode)
  {
    case SOUND_CORRECT:
      digitalWrite(alertPin, HIGH);
      delay(10);
      digitalWrite(alertPin, LOW);
      break;   
    
    case SOUND_INCORRECT:
      for(int i = 0;i<9;i++)
      {
      digitalWrite(alertPin, HIGH);
      delay(6);
      digitalWrite(alertPin, LOW);
      delay(15);
      }     
      break;   

    case SOUND_SUCCESSFUL_ROUND:
      for(int i = 0;i<3;i++)
      {
      digitalWrite(alertPin, HIGH);
      delay(10);
      digitalWrite(alertPin, LOW);
      delay(15);
      }     
      break;   
   
    default:
      break;
  }

}
