#include <HX711_ADC.h>
#include <EEPROM.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FirebaseESP8266.h>

// Load Cell Pins (ESP8266):
const int HX711_dout = D7; // ESP8266 GPIO pin connected to HX711 DOUT
const int HX711_sck = D8;  // ESP8266 GPIO pin connected to HX711 SCK

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAddress = 0;
unsigned long t = 0;

// MPU6050 setup
MPU6050 mpu;
const int stepsPerMeter = 9; // 9 steps per meter
volatile int stepCount = 0;
float distance = 0.0;
const int accelerationThreshold = 1500; // Lower threshold for more sensitivity
const unsigned long stepInterval = 100; // Shorter interval for faster step detection

// GPS setup
static const int RXPin = D2, TXPin = D1; // Use GPIO4 (D2) for RX, and GPIO5 (D1) for TX
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// WiFi credentials
const char* ssid = "WE_4C6BBA";
const char* password = "d0281505";

// Firebase setup
#define FIREBASE_HOST "https://grud-771b5-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyBPvu_elFMFXsF59x1-cf9hfO1F6_-8J14"

FirebaseData firebaseData;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Load Cell setup
  float calibrationValue;
  EEPROM.begin(512);
  EEPROM.get(calVal_eepromAddress, calibrationValue);
  LoadCell.begin();
  unsigned long stabilizingTime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingTime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(-55);
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());

  // MPU6050 setup
  Wire.begin(12, 14);
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  } else {
    Serial.println("MPU6050 connected.");
  }
  mpu.setDLPFMode(MPU6050_DLPF_BW_5);
  mpu.setRate(4);

  // GPS setup
  ss.begin(GPSBaud);

  // WiFi setup
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Firebase setup
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Load Cell
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500;
  if (LoadCell.update()) newDataReady = true;
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(weight);
      newDataReady = 0;
      t = millis();
      // Send weight to Firebase
      Firebase.setFloat(firebaseData, "/sensors/weight", weight);
    }
  }
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  // MPU6050
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
    // Send step count and distance to Firebase
    Firebase.setInt(firebaseData, "/sensors/steps", stepCount);
    Firebase.setFloat(firebaseData, "/sensors/distance", distance);
  }
  delay(50);
// GPS
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("No GPS detected: check wiring.");
    delay(5000);
  }
  if (gps.location.isValid()) {
    double lat = gps.location.lat();
    double lng = gps.location.lng();
    String timestamp = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
    Serial.print("Location: ");
    Serial.print(lat, 6);
    Serial.print(", ");
    Serial.print(lng, 6);
    Serial.print(" Time: ");
    Serial.println(timestamp);
    // Send GPS data to Firebase
    Firebase.setFloat(firebaseData, "/sensors/location/latitude", lat);
    Firebase.setFloat(firebaseData, "/sensors/location/longitude", lng);
    Firebase.setString(firebaseData, "/sensors/location/timestamp", timestamp);
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
