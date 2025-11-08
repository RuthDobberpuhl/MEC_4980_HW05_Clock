#include <Arduino.h>
#include <WiFi.h>
#include <AccelStepper.h>
#include <time.h>

/*
const int limitSwitchPin = 5;

void setup() {
  Serial.begin(115200);
  pinMode(limitSwitchPin, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(limitSwitchPin) == HIGH){
    Serial.println("Limit Switch is HIGH");
  } else {
    Serial.println("Limit Switch is LOW");
  }
}*/


const int stepsPerRevolution = 200;
AccelStepper Stepper1(AccelStepper::DRIVER, 11, 12); // second hand
const int limitSwitchPin = 5;

const char* ssid = "NSA Security Van HQ";
const char* password = "windowstothehallway";
const char* ntpServer = "time.google.com";
const long gmtOffset_sec = -21600;   // CST
const int daylightOffset_sec = 0;

long stepsPerSecond = stepsPerRevolution / 60; // steps per second

// --- Move stepper to current NTP time ---
void moveToCurrentTime() {
  struct tm timeinfo;
  Serial.println("Waiting for NTP time...");
  unsigned long start = millis();
  const unsigned long timeoutMs = 30000; // 30 second timeout
  // Wait until SNTP provides a time or timeout
  while (!getLocalTime(&timeinfo)) {
    if (millis() - start >= timeoutMs) {
      Serial.println("Timeout while waiting for NTP time");
      return;
    }
    Serial.print('.');
    delay(500);
  }
  Serial.println();

  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  int second = timeinfo.tm_sec;

  if (hour >= 12) {
    hour -= 12; // Convert to 12-hour format
  }

  long totalSeconds = hour*3600 + minute*60 + second;
  long targetStep = totalSeconds * stepsPerSecond;

  Stepper1.moveTo(targetStep);
  Stepper1.setSpeed(800);
  while(Stepper1.distanceToGo() != 0){
    Stepper1.run();
  }

  Serial.printf("Moved to current time: %02d:%02d:%02d\n", hour, minute, second);
}



void setup() {
  Serial.begin(115200);
  pinMode(limitSwitchPin, INPUT_PULLUP);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" connected!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "pool.ntp.org");

  Stepper1.setMaxSpeed(800);
  Stepper1.setAcceleration(300);

  // --- Home the second hand ---
  Serial.println("Homing...");
  Stepper1.setSpeed(800); 
  while(digitalRead(limitSwitchPin) == LOW){
    Stepper1.runSpeed();
  }
  Stepper1.setCurrentPosition(0);
  Serial.println("Homed!");

  // --- Move to current time ---
  moveToCurrentTime();
  //correctDrift();
  moveToCurrentTime();
}

void loop() {
  // Run continuously like a second hand
  Stepper1.setSpeed(stepsPerSecond); // 1 rev per 60 seconds
  Stepper1.runSpeed();


}

