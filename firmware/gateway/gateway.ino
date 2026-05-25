// gateway.ino — ESP32-C3 Super Mini
// Recibe datos de nodos vía E32-433T30D (LoRa UART)
// Obtiene códigos diarios del backend (WiFi + HTTPS) y los envía por LoRa

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

#define LED_PIN      8
// --- E32-433T30D (UART transparente) ---
// M0 y M1 conectados a GND (modo normal)
#define E32_TX_PIN   5  // ESP32 TX → E32 RXD
#define E32_RX_PIN   6  // ESP32 RX ← E32 TXD
#define E32_BAUD    9600

#define REFRESH_MS  (24UL * 60UL * 60UL * 1000UL)  // 24 horas

String rxBuffer    = "";
unsigned long lastFetch = 0;

// ---------------------------------------------------------------------------
// WiFi
// ---------------------------------------------------------------------------
void connectWiFi() {
  Serial.print("[GW] Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(" OK — IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" FALLO");
  }
}

// ---------------------------------------------------------------------------
// Parseo JSON mínimo
// Espera: {"date":"...","codes":["111111","222222",...]}
// Retorna: "111111,222222,..." (CSV sin comillas)
// ---------------------------------------------------------------------------
String extractCodesCSV(const String& json) {
  int start = json.indexOf('[');
  int end   = json.lastIndexOf(']');
  if (start < 0 || end < 0 || end <= start) return "";

  String arr = json.substring(start + 1, end);
  arr.replace("\"", "");   // quita todas las comillas
  arr.trim();
  return arr;              // "111111,222222,..."
}

// ---------------------------------------------------------------------------
// Fetch códigos del backend — retorna CSV o "" si falla
// ---------------------------------------------------------------------------
String fetchCodes() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (WiFi.status() != WL_CONNECTED) return "";

  HTTPClient http;
  http.begin(API_URL);
  http.setInsecure();   // skip cert validation (HTTPS sobre red privada)
  http.addHeader("x-gateway-secret", API_SECRET);

  int code = http.GET();
  String csv = "";
  if (code == 200) {
    String body = http.getString();
    Serial.print("[GW] API OK: ");
    Serial.println(body);
    csv = extractCodesCSV(body);
  } else {
    Serial.print("[GW] HTTP error: ");
    Serial.println(code);
  }
  http.end();
  return csv;
}

// ---------------------------------------------------------------------------
// Envía códigos a los nodos por LoRa
// Formato: "CODES:111111,222222,...\n"
// ---------------------------------------------------------------------------
void sendCodesToNodes(const String& csv) {
  if (csv.length() == 0) return;
  String msg = "CODES:" + csv + "\n";
  Serial1.print(msg);
  Serial.print("[GW] LoRa TX: ");
  Serial.print(msg);

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);  delay(80);
    digitalWrite(LED_PIN, HIGH); delay(80);
  }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // active LOW

  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LOW);  delay(100);
    digitalWrite(LED_PIN, HIGH); delay(100);
  }

  Serial1.begin(E32_BAUD, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);
  Serial.println("[GW] Gateway listo.");

  connectWiFi();

  // Fetch inicial al arrancar
  String csv = fetchCodes();
  if (csv.length() > 0) sendCodesToNodes(csv);
  lastFetch = millis();
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {
  // Refrescar códigos cada 24 h
  if (millis() - lastFetch >= REFRESH_MS) {
    String csv = fetchCodes();
    if (csv.length() > 0) sendCodesToNodes(csv);
    lastFetch = millis();
  }

  // Recibir paquetes LoRa entrantes
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      rxBuffer.trim();
      if (rxBuffer.length() > 0) {
        Serial.print("[GW] Received: ");
        Serial.println(rxBuffer);
        digitalWrite(LED_PIN, LOW);  delay(30);
        digitalWrite(LED_PIN, HIGH);
      }
      rxBuffer = "";
    } else {
      rxBuffer += c;
    }
  }
}
