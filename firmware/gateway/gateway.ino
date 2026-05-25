// gateway.ino — ESP32-C3 Super Mini
// Recibe datos de nodos vía E32-433T30D (LoRa UART) y los reenvía al backend

#define LED_PIN  8
// --- E32-433T30D (UART transparente) ---
// M0 y M1 conectados a GND (modo normal)
#define E32_TX_PIN   5  // ESP32 TX → E32 RXD
#define E32_RX_PIN   6  // ESP32 RX ← E32 TXD
#define E32_BAUD    9600

String rxBuffer = "";

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // active LOW

  Serial.begin(115200);

  // Blink 5x en boot
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LOW);  delay(100);
    digitalWrite(LED_PIN, HIGH); delay(100);
  }

  Serial1.begin(E32_BAUD, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);

  Serial.println("[GW] Ready. Listening for LoRa packets.");
}

void loop() {
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
