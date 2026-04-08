📌 Overview

This project demonstrates an end-to-end IoT data pipeline, where real-time sensor data is collected, processed at the edge, and streamed to a cloud database for monitoring and analytics.

The system simulates real-world data engineering scenarios involving:

Streaming data ingestion
Real-time processing
Cloud-based storage
Data visualization

⚙️ System Architecture
Load Cell + MPU6050 + GPS
            ↓
        ESP8266
            ↓
          WiFi
            ↓
      Firebase (Cloud)
            ↓
     Flutter Mobile App

🟢 Data Source
IoT sensors (Load Cell, MPU6050, GPS)
🔵 Ingestion Layer
ESP8266 sends streaming data via WiFi
🟣 Data Storage
Firebase Realtime Database
🟡 Processing (you already did basic)
Step detection logic
Distance calculation
🔴 Consumption
Flutter app (dashboard)

     
🔧 Hardware Components
ESP8266 (NodeMCU)
Load Cell + HX711 Amplifier
MPU6050 (Accelerometer & Gyroscope)
GPS Module (Neo-6M)
Power Supply System
💻 Software & Technologies
Embedded C (Arduino IDE)
Firebase Realtime Database
Flutter (FlutterFlow)
WiFi Communication (ESP8266)
Serial Communication & Debugging
📊 Features
🟢 Weight Monitoring
Measures patient support using load cell
Helps track rehabilitation progress
🔵 Step Detection & Distance
Uses MPU6050 to detect steps
Calculates walking distance
🟣 GPS Tracking
Real-time location monitoring
Useful for patient safety
☁️ Cloud Integration
Sends data to Firebase in real time
📱 Mobile Application
Displays:
Weight
Steps
Distance
Location
📁 Project Structure
Smart-Walker/
│
├── embedded/
│   ├── main/
│   ├── calibration/
│   ├── testing/
│   └── config_example.h
│
├── mobile_app/
├── hardware/
├── docs/
└── README.md

🎯 Applications
Rehabilitation monitoring
Elderly care systems
Smart healthcare IoT devices

👨‍💻 Author

Adham Ahmed
Biomedical Engineer | Embedded Systems | IoT
