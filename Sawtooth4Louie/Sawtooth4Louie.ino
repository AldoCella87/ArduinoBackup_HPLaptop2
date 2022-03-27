int i_counter = 0;
const int i_max = 300;
bool b_go_up = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(b_go_up == true){
    i_counter ++;
  }
  else{
    i_counter = i_counter - 8;
  }
  
  Serial.println(i_counter);
  
  if(i_counter > i_max){
    b_go_up = false;
    digitalWrite(LED_BUILTIN, LOW);
  }
  else if(i_counter < 2){
    b_go_up = true;
    digitalWrite(LED_BUILTIN, HIGH);
  }

  delay(1);
}
