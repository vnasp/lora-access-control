// gateway.ino — ESP32-C3 Super Mini
// Fase 1: prueba de hardware — sin LoRa
// Fase 2: WiFi + ArduinoOTA (flasheo remoto sin USB)

#include <WiFi.h>
#include <ArduinoOTA.h>

#include "secrets.h"

#define LED_PIN 8

unsigned long lastHeartbeat = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // active LOW: start with LED off
  Serial.begin(115200);

  // Blink 5x on boot (active LOW)
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LOW);  delay(100);
    digitalWrite(LED_PIN, HIGH); delay(100);
  }

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[GATEWAY] Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[GATEWAY] IP: ");
  Serial.println(WiFi.localIP());

  // OTA
  ArduinoOTA.setHostname("esp32-gateway");
  ArduinoOTA.begin();

  Serial.println("[GATEWAY] Ready. OTA enabled.");
}

void loop() {
  ArduinoOTA.handle();

  if (millis() - lastHeartbeat >= 2000) {
    lastHeartbeat = millis();
    Serial.println("[GATEWAY] alive");

    // Heartbeat blink (active LOW)
    digitalWrite(LED_PIN, LOW);  delay(30);
    digitalWrite(LED_PIN, HIGH);
  }
}
