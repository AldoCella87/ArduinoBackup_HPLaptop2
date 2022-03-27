// Exterior Door Monitor
// based on requirement from Dah H. 3/8/2020
// thought starters for behavior...
// if door open more than x minutes sound periodic alarm
// if door open more than y minutes after z time of day sound alarm
// add array to handle multiple inputs

// libraries to support various functions.  These will have to be located and downloaded to the "libraries" folder in 
// your Arduino folder.  
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>       

DHT dht;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

const char* ssid = "asdfgbvcxz";                // wireless connection setup - use your values here of course
const char* password = "2w3e4r5tdolly";

// inputs and outputs
const int Door1Pin = 16;                        // Door 1 status GPIO16 equates to D0 pin on Nodemcu board
                                                // connect one side of a normally open switch to this pin.  The other side 
                                                // goes to ground.  Switch is open when the door is open and closed when the door is closed.
                                                // use normally open reed switch and magnet or similar setup.
                                                
const int alarmPin = 4;                         // audio alarm, GPIO 4 equates to D2 pin on Nodemcu board. Attach the positive side of a sonic
                                                // alert unit to this pin.  The other side of the alert goes to ground.
                                                
const int dhtPin = 5;                           // connect signal pin from DHT11 temperature/humidity sensor, GPIO 5 equates to D1 pin
                                                // on the Nodemcu board.

const int RedLEDPin = 9;                        // GPIO9 = S2 pin on Nodemcu board.  Connect anode of red LED to this pin through a 200 - 300 ohm resistor.  
                                                // Cathode of LED goes to gnd.                                                
const int GreenLEDPin = 10;                     // GPIO10 = S3 pin on Nodemcu board.  connect anode of green LED to this pin through a 200 - 300 ohm resistor.  
                                                // Cathode of LED goes to gnd. 

int iDoor1Input = 0;                            // state of door 
int iPrevDoor1Input = 0;                        // door state from previous scan, used to determine transition from closed to open

// 
// server code adapted from from: https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/
//  
ESP8266WebServer server(80);         

float Temperature;
unsigned long ulDoor1OpenTime = 0;              // record opening time of door 1

unsigned long ulDoor1AlarmTime = 60000;         // ms after which alarm sounds
                                                // 60000 = 1 minute
                                                // 600000 = 10 minutes, etc.

void setup() {
  Serial.begin(9600);
  delay(100);
  pinMode(Door1Pin, INPUT_PULLUP);              // holds state of door switch
  pinMode(alarmPin, OUTPUT);                    // alarm pin
  pinMode(RedLEDPin, OUTPUT);                   // LED indicator for door open  
  pinMode(GreenLEDPin, OUTPUT);                 // LED indicator for door closed
  
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

  timeClient.begin();                                   // initialize NTP client
  
  timeClient.setTimeOffset(0);                          // Set offset time in seconds to adjust for your timezone, for example:
                                                        // GMT +1 = 3600
                                                        // GMT +8 = 28800
                                                        // GMT -1 = -3600
                                                        // GMT 0 = 0
  
  dht.setup(dhtPin);                                    // temp and humidity sensor setup
}

void loop() {

  iDoor1Input = digitalRead(Door1Pin);                  // get door status, use normally open switch with input in pullup mode.
                                                        // switch is connected from input to ground.  Switch is closed when door 
                                                        // is closed pulling input low.  0 represents closed, 1 represents open

  if(iDoor1Input == 1 && iPrevDoor1Input == 0){         // detect transition from door closed to door open and get timestamp
    ulDoor1OpenTime = millis();
    Serial.println("transition from closed to open");
  }

  if(iDoor1Input == 1){                                 // if door is currently open
    // handle LEDs
    digitalWrite(RedLEDPin,HIGH);
    digitalWrite(GreenLEDPin, LOW);
    Serial.println("Door Open");
    
    // check if time to sound alarm
     if( millis() > ( ulDoor1OpenTime + ulDoor1AlarmTime)){
        digitalWrite(alarmPin,HIGH);
        Serial.println("Alarm condition");
     } 
  }
  else{
        // handle LEDs
        digitalWrite(RedLEDPin,LOW);
        digitalWrite(GreenLEDPin, HIGH);
        
        digitalWrite(alarmPin,LOW);
        Serial.println("Door Closed");    
  }

  server.handleClient();                                // handle requests for web page

  iPrevDoor1Input = iDoor1Input;                        // save current door reading for future comparison
}

void handle_OnConnect() {

  iDoor1Input = digitalRead(Door1Pin);                  // get door status, use normally open switch with input in pullup mode.
                                                        // switch is connected from input to ground.  Switch is closed when door 
                                                        // is closed pulling input low.
  Serial.print("Door 1 input is: ");
  Serial.println(iDoor1Input);

  server.send(200, "text/html", SendHTML());     
  
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
  ptr +=" UTC";               // adjust this accordingly bif you change timezone offset

  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  
  return ptr;
}
