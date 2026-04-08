#include <HX711_ADC.h>
#include <EEPROM.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Pins and Constants for Load Cell (ESP8266)
const int HX711_dout = D7; // ESP8266 GPIO pin connected to HX711 DOUT
const int HX711_sck = D8;  // ESP8266 GPIO pin connected to HX711 SCK
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAddress = 0;
unsigned long t = 0;

// Constants for MPU6050
MPU6050 mpu;
const int stepsPerMeter = 9; // 9 steps per meter
volatile int stepCount = 0;
float distance = 0.0;
const int accelerationThreshold = 1500; // Lower threshold for more sensitivity
const unsigned long stepInterval = 100; // Shorter interval for faster step detection

// Constants for GPS and WiFi
static const int RXPin = D2, TXPin = D1; // Use GPIO4 (D2) for RX, and GPIO5 (D1) for TX
static const uint32_t GPSBaud = 9600;
const char* ssid = "mosa";
const char* password = "Slp@tec.2003";
IPAddress local_IP(192, 168, 1, 11);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(163,121,128,134);
IPAddress secondaryDNS(163,121,128,135);

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
ESP8266WebServer server(80);
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 2000; // Print every 2 seconds

void setup() {
  Serial.begin(115200); // Initialize hardware serial for debugging
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  // Setup Load Cell
  float calibrationValue; // calibration value
  EEPROM.begin(512); // Initialize EEPROM with 512 bytes
  EEPROM.get(calVal_eepromAddress, calibrationValue);
  LoadCell.begin();
  unsigned long stabilizingTime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingTime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(-55); // Set calibration factor (float)
    Serial.println("Startup is complete");
  }

  // Setup MPU6050
  Wire.begin(12, 14); // Initialize I2C with SDA = GPIO 12 (D6), SCL = GPIO 14 (D5)
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  } else {
    Serial.println("MPU6050 connected.");
  }
  mpu.setDLPFMode(MPU6050_DLPF_BW_5);
  mpu.setRate(4); // 1kHz / (1 + rate) => 200Hz

  // Setup GPS and WiFi
  ss.begin(GPSBaud);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println(F("STA Failed to configure"));
  }
  WiFi.begin(ssid, password);
  Serial.print(F("Connecting to Wi-Fi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("Connected!"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/gps", handleGPS);
  server.begin();
  Serial.println(F("HTTP server started"));
}

void loop() {
  // Load Cell Functionality
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500; // Increase value to slow down serial print activity
  if (LoadCell.update()) newDataReady = true;
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
    }
  }
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  // MPU6050 Functionality
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  if (detectStep(ax, ay, az)) {
    stepCount++;
    distance = stepCount / float(stepsPerMeter);
    Serial.print("Steps: ");
    Serial.print(stepCount);
    Serial.print(", Distance: ");
    Serial.print(distance);
    Serial.println(" meters");
  }
  delay(50); // Adjust delay for step detection sensitivity
// GPS and WiFi Functionality
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }
  server.handleClient();
  ensureWiFiConnection();
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    delay(5000);
  }
  if (millis() - lastPrintTime >= printInterval) {
    printGPSDataToSerial();
    lastPrintTime = millis();
  }
}

bool detectStep(int16_t ax, int16_t ay, int16_t az) {
  static int16_t prevAy = 0;
  static unsigned long prevStepTime = 0;
  unsigned long currentTime = millis();
  int16_t deltaAy = abs(ay - prevAy);
  if (deltaAy > accelerationThreshold && (currentTime - prevStepTime) > stepInterval) {
    prevStepTime = currentTime;
    prevAy = ay;
    return true;
  }
  prevAy = ay;
  return false;
}

void handleRoot() {
  String html = "<html><head><title>ESP8266 GPS</title>";
  html += "<style>body{font-family:Arial;text-align:center;padding:50px;}</style>";
  html += "<script>";
  html += "function fetchGPSData() {";
  html += "fetch('/gps').then(response => response.json()).then(data => {";
  html += "document.getElementById('location').innerText = data.location;";
  html += "document.getElementById('date').innerText = data.date;";
  html += "document.getElementById('time').innerText = data.time;";
  html += "document.getElementById('altitude').innerText = data.altitude;";
  html += "});";
  html += "}";
  html += "setInterval(fetchGPSData, 1000);";
  html += "</script>";
  html += "</head><body onload='fetchGPSData()'>";
  html += "<h1>ESP8266 GPS Data</h1>";
  html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
  html += "<p><strong>Location:</strong> <span id='location'>Loading...</span></p>";
  html += "<p><strong>Date:</strong> <span id='date'>Loading...</span></p>";
  html += "<p><strong>Time:</strong> <span id='time'>Loading...</span></p>";
  html += "<p><strong>Altitude:</strong> <span id='altitude'>Loading...</span></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleGPS() {
  String json = "{";
  json += "\"location\":\"";
  if (gps.location.isValid()) {
    json += String(gps.location.lat(), 6) + ", " + String(gps.location.lng(), 6);
  } else {
    json += "INVALID";
  }
  json += "\",\"date\":\"";
  if (gps.date.isValid()) {
    json += String(gps.date.month()) + "/" + String(gps.date.day()) + "/" + String(gps.date.year());
  } else {
    json += "INVALID";
  }
  json += "\",\"time\":\"";
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) json += "0";
    json += String(gps.time.hour()) + ":";
    if (gps.time.minute() < 10) json += "0";
    json += String(gps.time.minute()) + ":";
    if (gps.time.second() < 10) json += "0";
    json += String(gps.time.second());
  } else {
    json += "INVALID";
  }
  json += "\",\"altitude\":\"";
  if (gps.altitude.isValid()) {
    json += String(gps.altitude.meters());
  } else {
    json += "INVALID";
  }
  json += "\"}";
  server.send(200, "application/json", json);
}
void printGPSDataToSerial() {
  Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    double currentLat = gps.location.lat();
    double currentLng = gps.location.lng();
    Serial.print(currentLat, 6);
    Serial.print(F(","));
    Serial.print(currentLng, 6);
  } else {
    Serial.print(F("INVALID"));
  }
  Serial.print(F("  Date: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  } else {
    Serial.print(F("INVALID"));
  }
  Serial.print(F("  Time: "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
  } else {
    Serial.print(F("INVALID"));
  }
  Serial.print(F("  Altitude: "));
  if (gps.altitude.isValid()) {
    Serial.print(gps.altitude.meters());
  } else {
    Serial.print(F("INVALID"));
  }
  Serial.print(F("  IP Address: "));
  Serial.println(WiFi.localIP());
  Serial.println();
}

void ensureWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi disconnected, attempting reconnection..."));
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(F("."));
    }
    Serial.println(F("Reconnected to WiFi!"));
  }
}
