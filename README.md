Smart Walker IoT Monitoring System

📌 Overview
This project implements a complete IoT data pipeline for real-time smart walker monitoring. Sensor data from load cells, motion sensors, and GPS is collected at the edge, processed, and streamed to Firebase for cloud-based analytics and mobile visualization.

Key capabilities:

Streaming data ingestion from IoT sensors

Real-time edge processing (step detection, distance calculation)

Cloud storage with Firebase Realtime Database

Interactive mobile dashboard with Flutter

⚙️ System Architecture
text
IoT Sensors (Load Cell + MPU6050 + GPS)
           ↓
      ESP8266 (NodeMCU)
           ↓ WiFi
     Firebase Realtime DB
           ↓
     Flutter Mobile App
🟢 Components
Layer	Technology	Purpose
🟢 Data Source	Load Cell + HX711, MPU6050, Neo-6M GPS	Weight, motion, location tracking
🔵 Ingestion	ESP8266 NodeMCU	WiFi streaming via HTTP
🟣 Storage	Firebase Realtime Database	Cloud persistence & sync
🔴 Processing	Embedded C	Step detection, distance calculation
🟡 Visualization	Flutter App	Real-time dashboard
🔧 Hardware Requirements
ESP8266 NodeMCU (main microcontroller)

Load Cell + HX711 Amplifier (weight measurement)

MPU6050 (accelerometer + gyroscope)

Neo-6M GPS Module (location tracking)

5V Power Supply

💻 Software Stack
Embedded: Arduino IDE + C (ESP8266)

Cloud: Firebase Realtime Database

Mobile: Flutter (built with FlutterFlow)

Communication: WiFi (HTTP), Serial debugging

📊 Core Features
🟢 Weight Monitoring - Tracks patient support & rehabilitation progress

🔵 Step Detection - Real-time gait analysis using MPU6050

🟣 Distance Calculation - Walking distance computation

🟠 GPS Tracking - Patient location monitoring

☁️ Real-time Sync - Live data streaming to cloud

📱 Mobile Dashboard - Weight, steps, distance, location display

📁 Project Structure
text
Smart-Walker/
├── embedded/           # ESP8266 firmware
│   ├── main/           # Production code
│   ├── calibration/    # Sensor calibration routines
│   ├── testing/        # Test sketches
│   └── config_example.h # WiFi/Firebase credentials
├── mobile_app/         # Flutter mobile application
├── hardware/           # Schematics & wiring diagrams
├── docs/               # Documentation
└── README.md
🎯 Applications
Rehabilitation Monitoring

Elderly Care Systems

Smart Healthcare IoT

Fall Prevention Analytics

🚀 Quick Start
Setup Hardware - Wire sensors to ESP8266 (see hardware/ folder)

Configure Firebase - Update config_example.h with your credentials

Flash ESP8266 - Upload embedded/main/ code via Arduino IDE

Run Mobile App - Deploy Flutter app and connect to Firebase

👨‍💻 Author
Adham Ahmed
Biomedical Engineer | Embedded Systems | IoT
LinkedIn | Portfolio

Built with ❤️ for accessible healthcare
