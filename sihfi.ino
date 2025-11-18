/*
  ESP32 with MAX30102/05 (PPG) + ECG sensor
  Localhost Web Graphs (No MQTT)
  - WebSocket live plotting
  - 20 second display window
*/

#include <Wire.h>
#include "MAX30105.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// ---------------- SENSORS ----------------
MAX30105 particleSensor;
#define ECG_PIN 34   // ECG analog input pin

// ---------------- WIFI ----------------
const char* ssid = "Rushi";
const char* password = "11111111";

// ---------------- WEB SERVER ----------------
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// ---------------- HTML PAGE ----------------
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ECG + PPG Graph</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
  <h2>ESP32 PPG (top) + ECG (bottom) Live Graph</h2>
  
  <canvas id="ppgChart" width="600" height="200"></canvas>
  <canvas id="ecgChart" width="600" height="200"></canvas>

  <script>
    let ws = new WebSocket("ws://" + window.location.hostname + ":81/");

    const MAX_POINTS = 1000;  // 20s at ~50Hz

    // PPG Chart
    let ppgCtx = document.getElementById('ppgChart').getContext('2d');
    let ppgChart = new Chart(ppgCtx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'PPG IR Value',
          borderColor: 'red',
          data: [],
          fill: false,
          pointRadius: 0
        }]
      },
      options: { 
        animation: false,
        scales: { x: { display: false } }
      }
    });

    // ECG Chart
    let ecgCtx = document.getElementById('ecgChart').getContext('2d');
    let ecgChart = new Chart(ecgCtx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'ECG Value',
          borderColor: 'blue',
          data: [],
          fill: false,
          pointRadius: 0
        }]
      },
      options: { 
        animation: false,
        scales: { x: { display: false } }
      }
    });

    ws.onmessage = function(event) {
      let data = JSON.parse(event.data);

      // Update PPG
      ppgChart.data.labels.push('');
      ppgChart.data.datasets[0].data.push(data.ppg);
      if (ppgChart.data.labels.length > MAX_POINTS) {
        ppgChart.data.labels.shift();
        ppgChart.data.datasets[0].data.shift();
      }
      ppgChart.update();

      // Update ECG
      ecgChart.data.labels.push('');
      ecgChart.data.datasets[0].data.push(data.ecg);
      if (ecgChart.data.labels.length > MAX_POINTS) {
        ecgChart.data.labels.shift();
        ecgChart.data.datasets[0].data.shift();
      }
      ecgChart.update();
    };
  </script>
</body>
</html>
)rawliteral";

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected! IP: ");
  Serial.println(WiFi.localIP());

  // Web server
  server.on("/", []() {
    server.send(200, "text/html", webpage);
  });
  server.begin();

  // WebSocket
  webSocket.begin();

  // ECG pin
  pinMode(ECG_PIN, INPUT);

  // MAX3010x
  Wire.begin(21, 22);
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX3010x not found. Check wiring!");
    while (1);
  }


  // Sensor setup
  particleSensor.setup(0x1F, 8, 2, 100, 411, 4096);
  Serial.println("Sensors ready.");
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
  webSocket.loop();

  // Read sensors
  long ecgValue = analogRead(ECG_PIN);
  long ppgValue = particleSensor.getIR();

  // Debug print
  Serial.print("ECG: "); Serial.print(ecgValue);
  Serial.print(" | PPG: "); Serial.println(ppgValue);

  // Send JSON over WebSocket
  String message = "{\"ecg\":" + String(ecgValue) + ",\"ppg\":" + String(ppgValue) + "}";
  webSocket.broadcastTXT(message);

  delay(20); // ~50Hz sampling
}
