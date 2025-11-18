

# ESP32 Real-Time ECG + PPG Monitoring System  
Real-time biomedical monitoring using ESP32, MAX30102/05 PPG sensor, and an analog ECG module.  
The ESP32 hosts a **local web dashboard** that visualizes ECG and PPG waveforms in real time using **WebSockets + Chart.js**.

---

##  Features
- Real-time ECG waveform (50 Hz sampling)  
- Real-time PPG waveform (IR photoplethysmography)  
- ESP32-hosted web dashboard (no internet needed)  
- Dual live graphs (ECG + PPG)  
- WebSockets for smooth streaming  
- 20-second rolling window view  
- Works on phone or laptop  
- No MQTT or external servers required  

---

## Hardware Required
- **ESP32 Dev Board**  
- **MAX30102 / MAX30105 PPG Sensor**  
- **ECG Sensor Module (e.g., AD8232)**  
- Jumper wires  
- WiFi router or hotspot  

---

## Circuit Connections

### **PPG Sensor (MAX30102/05)**
| MAX3010x Pin | ESP32 Pin |
|--------------|-----------|
| SDA          | 21        |
| SCL          | 22        |
| VIN          | 3.3V      |
| GND          | GND       |

### **ECG Sensor**
| ECG Pin | ESP32 Pin |
|---------|-----------|
| OUTPUT  | GPIO 34   |
| VCC     | 3.3V      |
| GND     | GND       |

---

## How It Works
1. ESP32 connects to your WiFi network  
2. It starts:  
   - **HTTP Server on port 80** → serves the web dashboard  
   - **WebSocket Server on port 81** → streams data live  
3. Browser loads two Chart.js graphs  
4. ESP32 reads sensors at ~50 samples/sec  
5. Sends JSON packet:  
   ```json
   {"ecg":1234, "ppg":5678}
