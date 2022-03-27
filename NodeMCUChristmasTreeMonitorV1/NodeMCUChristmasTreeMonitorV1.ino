// AE 12/14/19 - using NODEMCU to display water level in Christmas tree stand
// server concepts from: https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
// 12/17 - added date and time

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
const int alarmPin = D2;       // audio alarm on connect
const int dhtPin = D1;         // pin for reading values for temperature/humidity sensor
int iInputValue = 0;          // value read from analog input
bool bLightState = false;     // toggle for holding result of light state reading
int iDigitalInValue = 0;

ESP8266WebServer server(80);

unsigned long i = 0;          // bogus "attempt" counter for Louie              

float Temperature;

 
void setup() {
  Serial.begin(9600);
  delay(100);

  pinMode(digitalInPin, INPUT);   // reads light state from light control board
  pinMode(alarmPin, OUTPUT);      // alarm pin
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
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

 // Temperature = 25;   // TODO:  assign a value here temporarily, replace it with water level reading
  iInputValue = analogRead(analogInPin);    // get water level reading
  iDigitalInValue = digitalRead(digitalInPin); // get light state
  Serial.print(iInputValue);
  Serial.print("\t");
  Serial.println(iDigitalInValue);
  
  server.send(200, "text/html", SendHTML(iInputValue,iDigitalInValue)); 
  digitalWrite(alarmPin,HIGH);
  delay(30);
  digitalWrite(alarmPin,LOW);

 
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
  if(millis() % 27 == 0)
  // enter fake backdoor mode
  {

    ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr +="<title>D77 Gateway (Restricted)</title>\n";
    ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
    ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
    ptr +="</style>\n";
    ptr +="</head>\n";
    ptr +="<body>\n";
    ptr +="<div id=\"webpage\">\n";
    ptr +="<h1>Level II Access</h1>\n";
   
    ptr +="<p>Please to enter ID user: ______________";
    ptr +="<p>Please to enter passwords: ______________";

    ptr +="<p> address:";
    ptr += millis() % 137;
    ptr +="</p>";
  
    ptr +="</div>\n";
    ptr +="</body>\n";
    ptr +="</html>\n";
  
      
    }
  else
  {
    ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr +="<title>Louie's Christmas Tree Condition Report</title>\n";
    ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
    ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
    ptr +="</style>\n";
    ptr +="</head>\n";
    ptr +="<body>\n";
    ptr +="<div id=\"webpage\">\n";
    ptr +="<h1>Christmas Tree Water Level</h1>\n";
    ptr +=dayStamp;
    ptr +=" ";
    ptr +=timeStamp;
    ptr +=" UTC";
    ptr +="<p>Water Level: ";
  
    ptr +=int(float(iValue)/10.23);
    ptr +="% full</p>";
    ptr +="<p>Lights: ";
    if(bLightState)
    {
      ptr +="ON";
    }
    else
    {
      ptr +="OFF";
    }
    ptr +="</p>";
  
    ptr +="<p>Humidity: ";
    ptr += h;
    ptr +=" %";
    ptr +="   Temperature: ";
    ptr +=f;
    ptr +=" F";
    ptr +="</p>";
  
  
  
  
    ptr +="<p>                   2 Factor Code: ";
    ptr += millis() % 13796;
    ptr +="</p>";
    
  
    if(i % 83 == 0)
    {
    ptr +="<p>                   Credential: 8DF7F3-____ ";
   //   ptr +="</p>";
    }
    else
    {
    ptr +="<p>                   Credential: ______-____ ";
   //   ptr +="</p>";
    }
  
  
  
    if(millis() % 25 == 0)
    {
      ptr +="<p>                   Status: Wait State 9F ";
      ptr +="</p>";
    }
    else
    {
      ptr +="<p>                   Status: Node-Locked ";
      ptr +="</p>";
    }
    
      ptr +="<p>Attempt: ";
    ptr += ++i;
  
  
  
    
    if(i % 117 == 0)
    {
    ptr +="<p>port scan initiated... ";
   //   ptr +="</p>";
    }
    else
    {
    ptr +="<p>                 ";
   //   ptr +="</p>";
    }
  
     
    if(i % 179 == 0)
    {
    ptr +="<p> This system is hack proof! ";
   //   ptr +="</p>";
    }
    else
    {
    ptr +="<p>                 ";
   //   ptr +="</p>";
    }

    if(i % 192 == 0)
    {
    ptr +="<p> Authorities have been notified... ";
   //   ptr +="</p>";
    }
    else
    {
    ptr +="<p>                 ";
   //   ptr +="</p>";
    }

    if(i % 62 == 0)
    {
    ptr +="<p> Lock-mode override code is:   2F ";
   //   ptr +="</p>";
    }
    else
    {
    ptr +="<p>                 ";
   //   ptr +="</p>";
    }
  
    ptr +="</div>\n";
    ptr +="</body>\n";
    ptr +="</html>\n";
  
      
    }
  
 
  return ptr;
}
