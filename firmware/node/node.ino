// node.ino — ESP32-C3 Super Mini
// Nodo sensor: genera temperatura ficticia y la transmite por E32-433T30D (LoRa UART)

#define INTERVAL_MS 5000
#define LED_PIN     8

// --- E32-433T30D (UART transparente) ---
// M0 y M1 conectados a GND (fijo en modo normal, sin control por software)
#define E32_TX_PIN  20  // ESP32 TX → E32 RXD
#define E32_RX_PIN  21  // ESP32 RX ← E32 TXD

#define E32_BAUD    9600  // baud rate por defecto del E32

unsigned long lastSend = 0;
float temperature = 22.0;  // temperatura inicial ficticia (°C)

float readTemperature() {
  // Simula variación realista: drift ±0.3 °C por lectura, rango 18–30 °C
  temperature += (random(-30, 31) / 100.0);
  temperature = constrain(temperature, 18.0, 30.0);
  return temperature;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // active LOW: start with LED off

  Serial.begin(115200);

  // Blink 5x en boot
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LOW);  delay(100);
    digitalWrite(LED_PIN, HIGH); delay(100);
  }

  // UART hacia el E32
  Serial1.begin(E32_BAUD, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);

  Serial.println("[NODE] Ready. Sending temperature via LoRa.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSend >= INTERVAL_MS) {
    lastSend = now;

    float temp = readTemperature();

    char payload[40];
    snprintf(payload, sizeof(payload), "NODE:node1,TEMP:%.2f\n", temp);

    Serial1.print(payload);
    Serial.print("[NODE] Sent: ");
    Serial.print(payload);

    digitalWrite(LED_PIN, LOW);
    delay(80);
    digitalWrite(LED_PIN, HIGH);
  }

  // Loopback test: imprime todo lo que llega por Serial1
  // (con jumper GPIO20-GPIO21 confirma que el UART llega al E32)
  while (Serial1.available()) {
    Serial.print("[NODE] Echo: ");
    Serial.write(Serial1.read());
  }
}
