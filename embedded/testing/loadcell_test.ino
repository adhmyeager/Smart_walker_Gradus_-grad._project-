#include <HX711_ADC.h>
#include <EEPROM.h>

// Pins (ESP8266):
const int HX711_dout = D7; // ESP8266 GPIO pin connected to HX711 DOUT
const int HX711_sck = D8;  // ESP8266 GPIO pin connected to HX711 SCK

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAddress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(115200); // Setting baud rate for ESP8266
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  float calibrationValue; // calibration value

  // Uncomment the following lines to fetch calibration value from EEPROM
  EEPROM.begin(512); // Initialize EEPROM with 512 bytes
  EEPROM.get(calVal_eepromAddress, calibrationValue);

  // If EEPROM is not used, set calibration value directly
  // calibrationValue = -53.60;

  LoadCell.begin();
  unsigned long stabilizingTime = 2000; // Tare precision can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; // Set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingTime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(-55); // Set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settling time ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  } else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500; // Increase value to slow down serial print activity

  // Check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // Get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
    }
  }

  // Receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // Check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}