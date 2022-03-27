// Exterior Door Monitor
// based on requirement from Dah H. 3/8/2020

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

// wireless connection
const char* ssid = "asdfgbvcxz";
const char* password = "2w3e4r5tdolly";

// inputs and outputs
const int digitalInPin = 1;      // ESP8266 digital pin 1 equates to GPIO1 on nodemcu board
const int alarmPin = 4;          // audio alarm, GPIO 4 equates to D2 pin
const int dhtPin = 5;            // pin for reading values for temperature/humidity sensor, equates to GPIO 5 (D1) pin

int iDoor1Input = 0;

ESP8266WebServer server(80);         

float Temperature;

 
void setup() {
  Serial.begin(9600);
  delay(100);

  pinMode(digitalInPin, INPUT_PULLUP);   // reads light state from light control board
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

  iDoor1Input = digitalRead(digitalInPin);              // get door status, use normally open switch with input in pullup mode.
                                                        // switch is connected from input to ground.  Switch is closed when door 
                                                        // is closed pulling input low.
  Serial.print("Door 1 input is: ");
  Serial.println(iDoor1Input);

  server.send(200, "text/html", SendHTML());     
  
  digitalWrite(alarmPin,HIGH);
  delay(30);
  digitalWrite(alarmPin,LOW);

 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(void){

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

  // get temp and humidity from DHT module
  float f = dht.toFahrenheit(dht.getTemperature());
  float h = dht.getHumidity();

  String ptr = "<!DOCTYPE html> <html>\n";

  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Door Monitor</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Door Status:</h1>\n";

  // thought starters for behavior...
  // if door open more than 15 minutes sound periodic alarm
  // if door open more than 5 minutes after dark sound alarm
  // If door is open after 11PM sound alarm
  // add multiple inputs
  

  ptr +="<p>Door1 Status: ";


  // display status for multiple inputs here (can be analog or digital)
  if(iDoor1Input == 0){
    ptr +="closed";    
  }
  else{
    ptr +="open";    
  } 
  ptr +="</p>";


  // done displaying statuses, show other info...
  ptr +="<p>Humidity: ";
  ptr += h;                   // humidity from sensor
  ptr +="   Temperature: ";
  ptr +=f;                    // temperature from sensor
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
