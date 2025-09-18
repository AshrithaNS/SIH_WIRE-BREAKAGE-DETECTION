#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "POCO X5 Pro 5G";
const char* password = "nik endhuku";

// ThingSpeak info
String apiKey = "EHNZ45V1PQPUVCE9";   // Your Write API key
const char* server = "http://api.thingspeak.com/update";

// Sensor pins
#define VIBRATION_DIGITAL 25   // D0 from vibration sensor
#define VIBRATION_ANALOG 34    // A0 from vibration sensor
#define VOLTAGE_PIN 35         // voltage sensor
#define CURRENT_PIN 32         // current sensor

void setup() {
  Serial.begin(115200);

  pinMode(VIBRATION_DIGITAL, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  // --- Read Sensors ---
  int vibDigital = digitalRead(VIBRATION_DIGITAL);   // D0
  int vibAnalog = analogRead(VIBRATION_ANALOG);      // A0 (0–4095)

  int voltageValue = analogRead(VOLTAGE_PIN);
  int currentValue = analogRead(CURRENT_PIN);

  // Convert raw ADC to voltage/current (needs calibration)
  float voltage = (voltageValue / 4095.0) * 3.3;  
  float current = (currentValue / 4095.0) * 3.3;  

  // --- Print locally (Serial Monitor) ---
  Serial.print("Vibration Digital: ");
  Serial.print(vibDigital);
  Serial.print(" | Vibration Analog: ");
  Serial.print(vibAnalog);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print(" V | Current: ");
  Serial.print(current, 2);
  Serial.println(" A");

  // --- Send to ThingSpeak ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + apiKey;
    url += "&field1=" + String(vibDigital);   // Field 1 → Digital Vibration
    url += "&field2=" + String(vibAnalog);    // Field 2 → Analog Vibration
    url += "&field3=" + String(voltage, 2);   // Field 3 → Voltage
    url += "&field4=" + String(current, 2);   // Field 4 → Current

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent to ThingSpeak.");
    } else {
      Serial.println("Error sending data.");
    }
    http.end();
  }

  delay(20000); // >= 15s (ThingSpeak free version)
}
