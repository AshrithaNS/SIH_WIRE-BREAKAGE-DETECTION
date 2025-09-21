#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "POCO X5 Pro 5G";
const char* password = "nik endhuku";

String apiKey = "EHNZ45V1PQPUVCE9";   // Your Write API key
const char* server = "http://api.thingspeak.com/update";

#define VIBRATION_DIGITAL 25   // D0 from vibration sensor
#define VIBRATION_ANALOG 34    // A0 from vibration sensor
#define VOLTAGE_PIN 35         // Voltage sensor (via divider)
#define CURRENT_PIN 32         // Current sensor (ACS712 type)

float voltageDividerRatio = 11.0;   // e.g., 100k/10k resistors → 11x scaling
float ACS712_Sensitivity = 0.066;   // 66 mV per A (ACS712-30A module)
float ACS712_Zero = 1.65;           // Midpoint voltage (for no current)

void setup() {
  Serial.begin(115200);

  pinMode(VIBRATION_DIGITAL, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  int vibDigital = digitalRead(VIBRATION_DIGITAL);   // D0
  int vibAnalog  = analogRead(VIBRATION_ANALOG);     // A0 (0–4095)

  int voltageValue = analogRead(VOLTAGE_PIN);
  int currentValue = analogRead(CURRENT_PIN);

  float sensorVoltage = (voltageValue / 4095.0) * 3.3;  
  float measuredVoltage = sensorVoltage * voltageDividerRatio;

  float sensorCurrentVoltage = (currentValue / 4095.0) * 3.3;  
  float measuredCurrent = (sensorCurrentVoltage - ACS712_Zero) / ACS712_Sensitivity;

  loat vibrationLevel = (vibAnalog / 4095.0) * 100.0;
  
  Serial.print("Vibration Digital: ");
  Serial.print(vibDigital);
  Serial.print(" | Vibration Analog: ");
  Serial.print(vibAnalog);
  Serial.print(" | Voltage: ");
  Serial.print(measuredVoltage, 2);
  Serial.print(" V | Current: ");
  Serial.print(measuredCurrent, 2);
  Serial.println(" A");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + apiKey;
    url += "&field1=" + String(vibDigital);        // Field 1 → Digital Vibration
    url += "&field2=" + String(vibAnalog);         // Field 2 → Analog Vibration
    url += "&field3=" + String(measuredVoltage,2); // Field 3 → Voltage
    url += "&field4=" + String(measuredCurrent,2); // Field 4 → Current

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent to ThingSpeak.");
    } else {
      Serial.println("Error sending data: " + String(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }

  delay(20000); // >= 15s (ThingSpeak free version)
}
