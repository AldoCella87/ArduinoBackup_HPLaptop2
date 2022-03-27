//AE - test old NodeMCU code on ESP8266 board...  9/25/21

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



/*Put your SSID & Password*/
const char* ssid = "asdfgbvcxz";
const char* password = "2w3e4r5tdolly";
const int analogInPin = A0;   // ESP8266 Analog pin ADC0 = A0
const int digitalInPin = 0;  // ESP8266 digital pin 0 equates to D3 on nodemcu board
//const int alarmPin = D2;       // audio alarm on connect
//const int dhtPin = D1;         // pin for reading values for temperature/humidity sensor
int iInputValue = 0;          // value read from analog input
bool bLightState = false;     // toggle for holding result of light state reading
int iDigitalInValue = 0;

unsigned long ulMyScore = 1;
unsigned long ulYourScore = 0;

ESP8266WebServer server(80);

unsigned long i = 0;          // bogus "attempt" counter for Louie              

float Temperature;

 
void setup() {
  Serial.begin(9600);
  delay(100);

  pinMode(digitalInPin, INPUT);   // reads light state from light control board
 // pinMode(alarmPin, OUTPUT);      // alarm pin
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

//  dht.setup(dhtPin);    

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
  
  server.send(200, "text/html", SendHTML(iInputValue,iDigitalInValue)); 
  //digitalWrite(alarmPin,HIGH);
  delay(30);
  //digitalWrite(alarmPin,LOW);

 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(int iValue, bool bLightState){

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
        default:
              ptr +="<p> I win this round ...try again";  
              ulMyScore ++;
              ulYourScore ++;      
      }
    }

    //  Display Scores
    ptr +="<p>My Score: ";
    ptr +=ulMyScore;
    ptr +="<p>Your Score: ";
    ptr +=ulYourScore;
    ptr +="</p>";

    ptr +="<p>Enter your MoveCode:  ___ ";

   
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
    ptr += ++i;
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
