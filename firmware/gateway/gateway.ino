// gateway.ino — ESP32-C3 Super Mini
// Recibe datos de nodos vía E32-433T30D (LoRa UART)
// Obtiene códigos diarios del backend (WiFi + HTTPS) y los envía por LoRa

#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include "secrets.h"

#define LED_PIN      8
#define PARCEL_COUNT 15
// E32-433T30D (UART transparente) — M0 y M1 conectados a GND (modo normal)
#define E32_TX_PIN   3
#define E32_RX_PIN   4
#define E32_AUX_PIN  2
#define E32_BAUD    9600

#define REFRESH_MS  (24UL * 60UL * 60UL * 1000UL)

String rxBuffer    = "";
unsigned long lastFetch = 0;

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
  arr.replace("\"", "");
  arr.trim();
  return arr;
}

// ---------------------------------------------------------------------------
String fetchCodes() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (WiFi.status() != WL_CONNECTED) return "";

  NetworkClientSecure client;
  client.setInsecure();   // skip cert validation (HTTPS sobre red privada)

  HTTPClient http;
  http.begin(client, API_URL);
  http.addHeader("x-gateway-secret", API_SECRET);

  int code = http.GET();
  String csv = "";
  if (code == 200) {
    csv = extractCodesCSV(http.getString());
    Serial.println("[GW] Códigos descargados OK.");
  } else {
    Serial.print("[GW] HTTP error: ");
    Serial.println(code);
  }
  http.end();
  return csv;
}

// ---------------------------------------------------------------------------
// Envía log de acceso al backend
// msg formato: "LOG:code,parcel_id,TYPE"
// ---------------------------------------------------------------------------
void sendLog(const String& msg) {
  String csv    = msg.substring(4);  // quita "LOG:"
  int    comma1 = csv.indexOf(',');
  if (comma1 < 0) return;
  int    comma2 = csv.indexOf(',', comma1 + 1);

  String code      = csv.substring(0, comma1);
  String parcel_id, event_type;

  if (comma2 < 0) {
    parcel_id  = csv.substring(comma1 + 1);
    event_type = "ENTRY";  // tolerancia formato sin TYPE
  } else {
    parcel_id  = csv.substring(comma1 + 1, comma2);
    event_type = csv.substring(comma2 + 1);
  }

  parcel_id.trim();
  code.trim();
  event_type.trim();

  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (WiFi.status() != WL_CONNECTED) return;

  NetworkClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, API_LOG_URL);
  http.addHeader("x-gateway-secret", API_SECRET);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"code\":\"" + code + "\",\"parcel_id\":" + parcel_id + ",\"event_type\":\"" + event_type + "\"}";
  int httpCode = http.POST(body);
  if (httpCode == 201) {
    Serial.print("[GW] Log guardado — parcela ");
    Serial.print(parcel_id);
    Serial.print(" código ");
    Serial.print(code);
    Serial.print(" (");
    Serial.print(event_type);
    Serial.println(")");
  } else {
    Serial.print("[GW] Log error HTTP: ");
    Serial.println(httpCode);
  }
  http.end();
}

// ---------------------------------------------------------------------------
// Envía códigos a los nodos por LoRa — se repite 3 veces para fiabilidad
// ---------------------------------------------------------------------------
void sendCodesToNodes(const String& csv) {
  if (csv.length() == 0) return;
  String msg = "CODES:" + csv;
  Serial.println("[GW] LoRa TX: " + msg);
  for (int i = 0; i < 3; i++) {
    Serial1.println(msg);
    delay(1000);  // dar tiempo al E32 para transmitir
  }

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);  delay(80);
    digitalWrite(LED_PIN, HIGH); delay(80);
  }
}

// ---------------------------------------------------------------------------
void setup() {
  pinMode(LED_PIN,     OUTPUT);
  pinMode(E32_AUX_PIN, INPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);
  Serial1.begin(E32_BAUD, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);
  delay(1000);  // dar tiempo al E32 para inicializar antes del primer TX

  connectWiFi();

  String csv = fetchCodes();
  if (csv.length() > 0) sendCodesToNodes(csv);
  lastFetch = millis();
}

// ---------------------------------------------------------------------------
void loop() {
  if (millis() - lastFetch >= REFRESH_MS) {
    String csv = fetchCodes();
    if (csv.length() > 0) sendCodesToNodes(csv);
    lastFetch = millis();
  }

  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      rxBuffer.trim();
      int logIdx = rxBuffer.indexOf("LOG:");
      if (logIdx >= 0) {
        sendLog(rxBuffer.substring(logIdx));
      } else if (rxBuffer.length() > 0) {
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
