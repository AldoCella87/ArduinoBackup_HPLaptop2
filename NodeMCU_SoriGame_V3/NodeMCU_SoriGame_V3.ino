#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "789quarmel";
const char *password = "quarmel";

ESP8266WebServer server(80);

void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
    delay(1000);
    Serial.begin(4800);
    Serial.println();
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);



 


/* Put IP Address details */
//IPAddress local_ip(192,168,1,1);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);


    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}
void loop() {
    server.handleClient();
}
