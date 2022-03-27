//NodeMCU test2
// source: http://internetofthinking.blogspot.com/2015/12/control-led-from-webserver-using.html


#include <ESP8266WiFi.h>
 
const char* ssid = "asdfgbvcxz";
const char* password = "2w3e4r5tdolly";

// defines pins numbers for distance sensor
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3

// defines variables for distance sensor
long duration;
int distance;
 
int ledPin = 13; // GPIO13
WiFiServer server(80);
 
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200);
  delay(10);
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
 
}
 
void loop() {

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  /***Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
***/
  // get distance:
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(200);

  // Read the first line of the request
  /***String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
***/
 
// Set ledPin according to the request
//digitalWrite(ledPin, value);
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Hello Louie: ");
  //client.print(distance);
  client.print(millis()); 
  /***if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
    
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</html>");

  ***/ 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 
}
