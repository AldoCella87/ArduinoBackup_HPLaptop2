/**
 * Copyright (c) 2015 by http://www.electrominds.com
 * Simple Arduino Garage Parking Sensor Project
 * Project URL: http://www.electrominds.com/projects/arduino-garage-parking-sensor
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * Required library:
 * NewPing: https://code.google.com/p/arduino-new-ping/downloads/detail?name=NewPing_v1.5.zip
 */
 
#include <NewPing.h>

#define DEBUG         true // Set to true to enable Serial debug
#define TRIGGER_PIN   12  // Board pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN      11  // Board pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE  300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

// leds pins
int ledR = 3;
int ledY = 4;
int ledG = 5;

int potPin = A0;

int optDistance; 
int optDistanceGap = 5;
int optMin;
int optMax;
int yellowGap = 10;
int redGap = 20;
int maxDistance = MAX_DISTANCE;
int currentDistance;
int prevDistance = 0;
int deviationThreshold = 2;
int timerStartTime = 0;
int ledsTimout = 30; // seconds

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
    Serial.println("Garage sensor is starting");
  }
  
  pinMode(ledG, OUTPUT);
  pinMode(ledY, OUTPUT);
  pinMode(ledR, OUTPUT);
}

void loop() {
  
  // get pot reading for the optimal distance.
  optDistance = analogRead(potPin) / 5; // potentiometer readins are in between 0 and 1023. Deviding it by 5 makes our optimal distance range from 0 to about 200cm
  
  // recalculate optimal distance gaps
  optMin = optDistance - optDistanceGap;
  optMax = optDistance + optDistanceGap;

  if (DEBUG) {
    Serial.print("Optimal distance:"); 
    Serial.println(optDistance); 
  }
  
  delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.

  currentDistance = sonar.ping() / US_ROUNDTRIP_CM; // Send ping, get ping time in microseconds (uS) and translate it into cm
  
  if (abs(currentDistance - prevDistance) >= deviationThreshold) {
    prevDistance = currentDistance;
    timerStartTime = millis();
    
    if (DEBUG) {
      Serial.print("Current distance:"); 
      Serial.println(currentDistance); 
    }
    
    // if distance is in range of optimal distances, turn on green led
    if (inRange(currentDistance, optMin, optMax)) {
      setLeds(LOW, LOW, HIGH);
      if (DEBUG) Serial.println("Got in optimal distance range");
    }
    // if distance is close to the optimal distances, but not yet there, turn on yellow led
    else if (inRange(currentDistance, optMin - yellowGap, optMin) || inRange(currentDistance, optMax, optMax + yellowGap)) {
      setLeds(LOW, HIGH, LOW);
      if (DEBUG) Serial.println("Close to the optimal distance");
    }
    // if distance is close to zero or too far, turn on red led
    else if (inRange(currentDistance, 0, optMin - yellowGap) || inRange(currentDistance, optMax + yellowGap, optMax + yellowGap + redGap)) {
      setLeds(HIGH, LOW, LOW);
      if (DEBUG) Serial.println("Too far from the optimal distance");
    }
    // otherwise turn all leds off
    else {
      setLeds(LOW, LOW, LOW);
      if (DEBUG) Serial.println("Turn all leds off");
    }
  } else {
    int currentTime = millis();
    if ((currentTime - timerStartTime) / 1000 > ledsTimout) {
      if (DEBUG) Serial.println((currentTime - timerStartTime) / 1000);
      setLeds(LOW, LOW, LOW);
    }
  }
}

/**
 * Turn leds on and off
 */
void setLeds(int r, int y, int g) {
  digitalWrite(ledR, r);
  digitalWrite(ledY, y);
  digitalWrite(ledG, g);
}

/**
 * Check is value in range
 */
bool inRange(int d, int rangeMin, int rangeMax) {
  return d >= rangeMin && d < rangeMax;
}
