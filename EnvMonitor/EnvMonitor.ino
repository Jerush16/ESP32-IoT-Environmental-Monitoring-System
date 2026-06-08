/*
 * ESP32-Based IoT Environmental Monitoring System
 * ------------------------------------------------
 * Author  : Jerush Thanusha T
 * Hardware: ESP32, DHT22, MQ-2, Soil Moisture Sensor, HC-SR04
 * Platform: Blynk IoT
 *
 */

#define BLYNK_TEMPLATE_ID   "TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "ESP32 Environmental Monitor"
#define BLYNK_AUTH_TOKEN    "AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi Credentials

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// Pin Definitions

#define DHTPIN        4
#define DHTTYPE       DHT22
#define MQ2_PIN       34
#define SOIL_PIN      35
#define TRIG_PIN       5
#define ECHO_PIN      18

// Blynk Virtual Pins

#define VPIN_TEMP       V0
#define VPIN_HUMIDITY   V1
#define VPIN_GAS        V2
#define VPIN_SOIL       V3
#define VPIN_DISTANCE   V4
#define VPIN_GAS_ALERT  V5

// Thresholds

#define GAS_THRESHOLD        1500   // ADC value above which gas alert triggers
#define SOIL_DRY_THRESHOLD     30   // Below 30% = dry soil warning
#define DISTANCE_MAX_CM       400   // HC-SR04 max reliable range
#define MQ2_WARMUP_MS       30000   // 30 seconds warmup before readings are valid
#define SEND_INTERVAL_MS     2000   // Blynk update interval

// Objects

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// Sensor Readers

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout = 25000us → max ~425cm, beyond reliable range
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return -1.0;  // Timeout: no echo received

  float distance = duration * 0.034 / 2.0;
  return (distance > DISTANCE_MAX_CM) ? -1.0 : distance;
}

// Converts raw soil ADC (0-4095) to moisture percentage (0-100%)
// 4095 = completely dry, 0 = submerged (inverted sensor output)
int soilToPercent(int raw) {
  int percent = map(raw, 4095, 0, 0, 100);
  return constrain(percent, 0, 100);
}

// Converts raw MQ2 ADC to approximate percentage for display
int gasToPercent(int raw) {
  return constrain(map(raw, 0, 4095, 0, 100), 0, 100);
}

// Main Data Send Function (called by timer)

void sendSensorData() {
  // ── DHT22 ──
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // Guard: don't send NaN if sensor fails
  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(VPIN_TEMP,     temperature);
    Blynk.virtualWrite(VPIN_HUMIDITY, humidity);
    Serial.printf("[DHT22] Temp: %.1f°C  Humidity: %.1f%%\n", temperature, humidity);
  } else {
    Serial.println("[DHT22] ERROR: Sensor read failed. Skipping transmission.");
  }

  // ── MQ-2 Gas Sensor ──
  int gasRaw     = analogRead(MQ2_PIN);
  int gasPercent = gasToPercent(gasRaw);
  Blynk.virtualWrite(VPIN_GAS, gasPercent);
  Serial.printf("[MQ2]   Gas Level: %d%% (raw: %d)\n", gasPercent, gasRaw);

  // Gas threshold alert
  if (gasRaw > GAS_THRESHOLD) {
    Blynk.virtualWrite(VPIN_GAS_ALERT, 1);  // Trigger LED widget on Blynk
    Serial.println("[MQ2]   ⚠ GAS ALERT: Threshold exceeded!");
  } else {
    Blynk.virtualWrite(VPIN_GAS_ALERT, 0);
  }

  // ── Soil Moisture ──
  int soilRaw     = analogRead(SOIL_PIN);
  int soilPercent = soilToPercent(soilRaw);
  Blynk.virtualWrite(VPIN_SOIL, soilPercent);
  Serial.printf("[SOIL]  Moisture: %d%% (raw: %d)%s\n",
    soilPercent, soilRaw,
    (soilPercent < SOIL_DRY_THRESHOLD) ? " ⚠ DRY" : "");

  // ── HC-SR04 Ultrasonic ──
  float distance = readDistance();
  if (distance >= 0) {
    Blynk.virtualWrite(VPIN_DISTANCE, distance);
    Serial.printf("[HCSR04] Distance: %.1f cm\n", distance);
  } else {
    Serial.println("[HCSR04] ERROR: Out of range or no echo.");
  }

  Serial.println("─────────────────────────────");
}

// Setup & Loop

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // MQ2 warmup — sensor needs 30s for stable readings
  Serial.println("[BOOT] Waiting 30s for MQ2 sensor warmup...");
  for (int i = 30; i > 0; i--) {
    Serial.printf("[BOOT] Warmup: %ds remaining\n", i);
    delay(1000);
  }
  Serial.println("[BOOT] Warmup complete. Starting data transmission.");

  timer.setInterval(SEND_INTERVAL_MS, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
