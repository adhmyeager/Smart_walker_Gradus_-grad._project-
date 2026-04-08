#include <HX711_ADC.h>
#include <EEPROM.h> // Include EEPROM library for ESP8266

// Pins:
const int HX711_dout = D7; // Change pins as needed for ESP8266
const int HX711_sck = D8; // Change pins as needed for ESP8266

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAddress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  // LoadCell.setReverseOutput(); // Uncomment to turn a negative output value to positive
  unsigned long stabilizingTime = 2000; // Precision right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; // Set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingTime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU > HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(1.0); // User-set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  calibrate(); // Start calibration procedure
}

void loop() {
  static boolean newDataReady = false;
  const int serialPrintInterval = 0; // Increase value to slow down serial print activity

  // Check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // Get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load cell output val: ");
      Serial.println(i);
      newDataReady = false;
      t = millis();
    }
  }

  // Receive command from serial terminal
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay(); // Tare
    else if (inByte == 'r') calibrate(); // Calibrate
    else if (inByte == 'c') changeSavedCalFactor(); // Edit calibration value manually
  }

  // Check if the last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}

void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell on a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from the serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') LoadCell.tareNoDelay();
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the load cell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from the serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell.refreshDataSet(); // Refresh the dataset to be sure that the known mass is measured correctly
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); // Get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as the calibration value (calFactor) in your project sketch.");

  Serial.println("***");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from the serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  Serial.println("***");
}
