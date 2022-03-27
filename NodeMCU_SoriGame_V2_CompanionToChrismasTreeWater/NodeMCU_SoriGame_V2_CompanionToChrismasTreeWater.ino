// AE 12/14/19 - using NODEMCU to implement simple "Sori" game
// 
// some server concepts from: https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
//  

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// add for time support
#include <NTPClient.h>
#include <WiFiUdp.h>
// added DHT11 temp and humidity sensor
#include "DHT.h"       

DHT dht;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

char caPerseverance[][20]
{
  "Apprentice",
  "Assistant",
  "Beginner",
  "Learner",
  "Journeyman",
  "Associate",
  "Advanced",
  "Practitioner",
  "Master",
  "Mentor",
  "Expert",
  "Guru I",
  "Guru IA", 
  "Guru IB",
  "Guru IC", 
  "Guru IC Advanced", 
  "Guru II", 
  "Guru III"    
};


/*Put your SSID & Password*/
const char* ssid = "asdfgbvcxz";
const char* password = "2w3e4r5tdolly";
const int analogInPin = A0;   // ESP8266 Analog pin ADC0 = A0
const int digitalInPin = 0;  // ESP8266 digital pin 0 equates to D3 on nodemcu board
const int alarmPin = D2;       // audio alarm on connect
const int dhtPin = D1;         // pin for reading values for temperature/humidity sensor
int iInputValue = 0;          // value read from analog input
bool bTerminalState = false;     // Indicates when terminal state of game is reached
int iDigitalInValue = 0;
int iPerseveranceLevel = 0;   // controls display of perseverance level
int iMaxPerseverance = 18;    // number of elements in perseverance descriptor table

unsigned long ulMyScore = 1;
unsigned long ulYourScore = 0;

ESP8266WebServer server(80);

unsigned long ulAttemptCounter = 0;          // bogus "attempt" counter for Louie              

float Temperature;

 
void setup() {
  Serial.begin(9600);
  delay(100);

  pinMode(digitalInPin, INPUT);   // reads light state from light control board
  pinMode(alarmPin, OUTPUT);      // alarm pin
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0

  
  timeClient.setTimeOffset(0);

  dht.setup(dhtPin);    

}
void loop() {
  
  server.handleClient();
  
}

void handle_OnConnect() {

  iInputValue = analogRead(analogInPin);    // get water level reading
  iDigitalInValue = digitalRead(digitalInPin); // get light state
  Serial.print(iInputValue);
  Serial.print("\t");
  Serial.println(iDigitalInValue);

  if(bTerminalState)
  {
    server.send(200, "text/html", SendHTML_FinishGame(iInputValue));     
  }
  else
  {
    server.send(200, "text/html", SendHTML_1(iInputValue));     
  }

  
  
  
  digitalWrite(alarmPin,HIGH);
  delay(30);
  digitalWrite(alarmPin,LOW);

 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML_1(int iValue){

  /********* NTP stuff from:
    Rui Santos
    Complete project details at https://randomnerdtutorials.com
    Based on the NTP Client library example
  *********/

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  float f = dht.toFahrenheit(dht.getTemperature());
  float h = dht.getHumidity();

  String ptr = "<!DOCTYPE html> <html>\n";

    ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr +="<title>Sori Interface V2.3</title>\n";
    ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
    ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
    ptr +="</style>\n";
    ptr +="</head>\n";
    ptr +="<body>\n";
    ptr +="<div id=\"webpage\">\n";
    ptr +="<h1>Game score summary:</h1>\n";


    // limit score
    if(ulMyScore > 1000000000)
    {
      ptr +="<p> You lose - game reset"; 
      ulMyScore = 1;
      ulYourScore = 0;
    }
    else
    {
      // express behavior and adjust score
      switch(millis() % 217)
      {
        case(1):
              ptr +="<p> Incorrect response time - try again"; 
              break;
        case(2):
              ptr +="<p> Are you trying to annoy me?"; 
              ulMyScore +=2;
              break;
        case(3):
              ptr +="<p> Too much light for that operation";    
              break;
        case(4):
              ptr +="<p> Score reduced due to improper response";    
              ulYourScore = ulYourScore / 2;
              break;
        case(5):
        case(6):
              ptr +="<p> Incorrect lValue - bad move...";    
              ulMyScore *=2;
              break;
        case(7):
              ptr +="<p> Condition 4 score boost"; 
              ulYourScore = ulMyScore -1;   
              break;   
  
        case(8):
              ptr +="<p> D10 sensor value lock";    
              break;
        case(9):
              ptr +="<p> Move Code password is:  RuNID10T";    
              break;
        case(10):
              ptr +="<p> Unlock not available at this time";   
              break;
        case(11):
              ptr +="<p> Condition 3 penalty"; 
              ulYourScore = 1;   
              break;                                 
        case(12):
              ptr +="<p> I am Sori.....    that you lost";   
              break;      
        case(13):
              ptr +="<p> You are not doing very well today";   
              break;      
        case(14):
              ptr +="<p> I am getting mad...";   
              break;                                  
        case(15):
              ptr +="<p> Perseverance boost"; 
              if (iPerseveranceLevel < (iMaxPerseverance - 1)) iPerseveranceLevel++;  
              break;             
        case(35):
              ptr +="<p> Temperature too low for tBonus...";   
              break;    
        case(36):
              ptr +="<p> iBonus added to score";
              ulYourScore +=9;   
              break;   
        case(37):
        case(38):        
              ptr +="<p> Well Played!";
              ulYourScore = ulMyScore + 1;   
              break;                   
        
        default:
              // no response in this case 
              // adjust score              
              ulMyScore ++;
              ulYourScore ++;
              
              // adjust perseverance and assign win
              if((ulAttemptCounter % 8) == 0)
              {
                if (iPerseveranceLevel < (iMaxPerseverance - 1)) iPerseveranceLevel++;
                if (iPerseveranceLevel == (iMaxPerseverance - 1)) bTerminalState = true;
              } 
      }
    }

    //  Display Scores
    ptr +="<p>My Score: ";
    ptr +=ulMyScore;
    ptr +="<p>Your Score: ";
    ptr +=ulYourScore;
    ptr +="</p>";


    if(ulMyScore > ulYourScore)
    {
      ptr +="<p> I win this round ...try again";                 
    }
    else if (ulMyScore <  ulYourScore)
    {
      ptr +="<p> You win this round"; 
    }
    else
    {
      ptr +="<p> Tie Score";                 
    }


      Serial.println(iPerseveranceLevel);
              
    ptr +="<p> Perseverance Level: ";
    ptr +=caPerseverance[iPerseveranceLevel];
   
    // display other information
    ptr +="<p>wValue: ";
    ptr +=int(float(iValue)/10.23); // water level from sensor
    ptr +="%</p>";
  
    ptr +="<p>hValue: ";
    ptr += h;                   // humidity from sensor
    ptr +="   tValue: ";
    ptr +=f;                    // temperature from sensor
    ptr +="</p>";
    ptr +="<p>lValue: ";
    ptr += millis() % 13796;    // random-like number from millis
    ptr +="</p>";

    ptr +="<p>iValue: ";
    ptr += ++ulAttemptCounter;
    ptr +="</p>";

    ptr +="<p>";          
    ptr +=dayStamp;
    ptr +=" ";
    ptr +=timeStamp;
    ptr +=" UTC";
  
    ptr +="</div>\n";
    ptr +="</body>\n";
    ptr +="</html>\n";
  
  return ptr;
}

String SendHTML_FinishGame(int iValue){

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  String ptr = "<!DOCTYPE html> <html>\n";

    ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr +="<title>Sori Interface V2.3</title>\n";
    ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
    ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
    ptr +="</style>\n";
    ptr +="</head>\n";
    ptr +="<body>\n";
    ptr +="<div id=\"webpage\">\n";
    ptr +="<h1>Congratulations Player 217</h1>\n";

    ptr +="<p> Amazon Gift Certificate Number:  AQS2-7AFUFK-KFJAY"; 
    
    ptr +="<p>Game Reset in process...";
    // display date and time
    ptr +="<p>";          
    ptr +=dayStamp;
    ptr +=" ";
    ptr +=timeStamp;
    ptr +=" UTC";
  
    ptr +="</div>\n";
    ptr +="</body>\n";
    ptr +="</html>\n";

    // reset game
    //bTerminalState = false;   // set to false to reset game, leave true to remain in terminal state
    ulMyScore = 1;
    ulYourScore = 0;
    ulAttemptCounter = 1;
    iPerseveranceLevel = 0;
  
  return ptr;
}
