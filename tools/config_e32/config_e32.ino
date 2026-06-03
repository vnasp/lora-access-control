// config_e32.ino — Lee configuración del módulo E32-433T30D
//
// Conexiones:
//   M0  → GPIO2  (OUTPUT)
//   M1  → GPIO3  (OUTPUT)
//   AUX → GPIO4  (INPUT)
//   E32 RXD ← GPIO20  (Serial1 TX)
//   E32 TXD → GPIO21  (Serial1 RX)
//
// Instrucciones:
//   1. Flashear en el ESP32 conectado al E32.
//   2. Abrir monitor serie a 115200.
//   3. Presionar Enter para leer la configuración.

#define LED_PIN   8
#define AUX_PIN   4
#define M0_PIN    2
#define M1_PIN    3

HardwareSerial E32Serial(1);

// Nombres legibles para SPED
const char* AIR_RATE[]  = {"0.3k","1.2k","2.4k","4.8k","9.6k","19.2k","19.2k","19.2k"};
const char* UART_BAUD[] = {"1200","2400","4800","9600","19200","38400","57600","115200"};
const char* UART_PARITY[] = {"8N1","8O1","8E1","8N1"};
const char* TX_POWER[]  = {"30dBm","27dBm","24dBm","21dBm"};

void waitAuxHigh(unsigned long timeoutMs = 3000) {
  unsigned long t = millis();
  while (digitalRead(AUX_PIN) == LOW && millis() - t < timeoutMs) delay(5);
}

void printConfig(uint8_t* b) {
  // b[0]=head, b[1]=ADDH, b[2]=ADDL, b[3]=SPED, b[4]=CHAN, b[5]=OPTION
  Serial.println("┌─────────────────────────────────┐");
  Serial.println("│      CONFIGURACIÓN E32           │");
  Serial.println("├─────────────────────────────────┤");
  Serial.printf( "│ Head     : 0x%02X                  │\n", b[0]);
  Serial.printf( "│ Dirección: 0x%02X%02X (%u)            │\n", b[1], b[2], (b[1]<<8)|b[2]);
  Serial.printf( "│ SPED     : 0x%02X                  │\n", b[3]);
  Serial.printf( "│   UART   : %s bps              │\n", UART_BAUD[(b[3]>>5)&0x07]);
  Serial.printf( "│   Paridad: %s                  │\n", UART_PARITY[(b[3]>>3)&0x03]);
  Serial.printf( "│   Air DR : %s                 │\n", AIR_RATE[b[3]&0x07]);
  Serial.printf( "│ Canal    : %u (%.1f MHz)         │\n", b[4], 410.0 + b[4]);
  Serial.printf( "│ OPTION   : 0x%02X                  │\n", b[5]);
  Serial.printf( "│   TX Mode: %s                 │\n", (b[5]>>7)&1 ? "Fijo" : "Transparente");
  Serial.printf( "│   IO drv : %s               │\n", (b[5]>>6)&1 ? "Push-pull" : "Open-col.");
  Serial.printf( "│   FEC    : %s                   │\n", (b[5]>>2)&1 ? "ON" : "OFF");
  Serial.printf( "│   Potenci: %s                │\n", TX_POWER[b[5]&0x03]);
  Serial.println("└─────────────────────────────────┘");
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // M0 y M1 conectados directo a 3.3V — modo Sleep fijo por hardware
  delay(500);

  Serial.begin(115200);
  delay(200);

  // UART confirmado OK por loopback. E32: RXD←GPIO5, TXD→GPIO6
  E32Serial.begin(9600, SERIAL_8N1, 6, 5);  // RX=6, TX=5
  delay(2000);  // esperar boot E32

  // Vaciar buffer
  while (E32Serial.available()) E32Serial.read();

  // Escribir config: C0 00 00 62 17 44
  //   ADDH=0x00, ADDL=0x00
  // SPED=0x1A → UART 9600 / 8N1 / Air 2.4k
  //   CHAN=0x17 → 433 MHz
  //   OPTION=0x44 → Transparente / Push-pull / FEC ON / 30dBm
  Serial.println("\n[CFG] Escribiendo configuracion: C0 00 00 1A 17 44 ...");
  uint8_t cfg[] = {0xC0, 0x00, 0x00, 0x1A, 0x17, 0x44};
  E32Serial.write(cfg, 6);

  delay(1000);

  int avail = E32Serial.available();
  Serial.printf("[CFG] Respuesta (%d bytes): ", avail);
  while (E32Serial.available()) {
    Serial.printf("%02X ", E32Serial.read());
  }
  Serial.println();

  // Verificar leyendo de vuelta
  while (E32Serial.available()) E32Serial.read();
  Serial.println("[CFG] Verificando con 0xC1 0xC1 0xC1 ...");
  E32Serial.write(0xC1); E32Serial.write(0xC1); E32Serial.write(0xC1);
  delay(1000);

  avail = E32Serial.available();
  Serial.printf("[CFG] Config actual (%d bytes): ", avail);
  while (E32Serial.available()) {
    Serial.printf("%02X ", E32Serial.read());
  }
  Serial.println();
  Serial.println("[CFG] Listo. Desconecta y reconecta VCC del E32 para aplicar cambios.");
}

void loop() {}
