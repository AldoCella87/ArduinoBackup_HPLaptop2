// AE 12/14/19 - using NODEMCU to display water level in Christmas tree stand
// server concepts from: https://lastminuteengineers.com/esp8266-dht11-dht22-web-server-tutorial/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


/*Put your SSID & Password*/
const char* ssid = "asdfgbvcxz";
const char* password = "2w3e4r5tdolly";
const int analogInPin = A0;   // ESP8266 Analog pin ADC0 = A0
const int digitalInPin = 0;  // ESP8266 digital pin 0 equates to D3 on nodemcu board
int iInputValue = 0;          // value read from analog input
bool bLightState = false;     // toggle for holding result of light state reading
int iDigitalInValue = 0;

ESP8266WebServer server(80);

              

float Temperature;

 
void setup() {
  Serial.begin(9600);
  delay(100);

  pinMode(digitalInPin, INPUT);   // reads light state from light control board
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
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(int iValue, bool bLightState){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Water Level Report</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Christmas Tree Water Level</h1>\n";
  
  ptr +="<p>Water Level: ";

  ptr +=float(iValue)/10.23;
  ptr +=" % full</p>";
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
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
