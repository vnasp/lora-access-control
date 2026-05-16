// node.ino — ESP32-C3 Super Mini
// Fase 1: prueba de hardware — sin LoRa
// Fase 2: WiFi + ArduinoOTA (flasheo remoto sin USB)

#include <WiFi.h>
#include <ArduinoOTA.h>

#include "secrets.h"

#define INTERVAL_MS 2000
#define LED_PIN     8

unsigned long lastSend = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // active LOW: start with LED off
  Serial.begin(115200);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[NODE] Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[NODE] IP: ");
  Serial.println(WiFi.localIP());

  // OTA
  ArduinoOTA.setHostname("esp32-node1");
  ArduinoOTA.begin();

  Serial.println("[NODE] Ready. OTA enabled.");
}

void loop() {
  ArduinoOTA.handle();

  unsigned long now = millis();

  if (now - lastSend >= INTERVAL_MS) {
    lastSend = now;
    Serial.println("[NODE] node1 alive");

    // Blink once (active LOW)
    digitalWrite(LED_PIN, LOW);
    delay(80);
    digitalWrite(LED_PIN, HIGH);
  }
}
