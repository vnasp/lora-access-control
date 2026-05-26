// node.ino — ESP32-C3 Super Mini
// Nodo de acceso: recibe códigos diarios por LoRa, valida entrada de teclado 4x3

#include <Keypad.h>

#define LED_PIN      8
#define PARCEL_COUNT 15
#define CODE_LEN     6

// --- E32-433T30D (UART transparente) ---
// M0 y M1 conectados a GND (modo normal)
#define E32_TX_PIN   3   // ESP32 TX → E32 RXD
#define E32_RX_PIN   4   // ESP32 RX ← E32 TXD
#define E32_AUX_PIN  2   // AUX: LOW=ocupado, HIGH=listo
#define E32_BAUD    9600

// --- Teclado 4x3 (filas/cols invertidos: GPIO9 queda como INPUT para evitar
//     conflicto con botón BOOT del ESP32-C3) ---
const byte ROWS = 3;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','4','7','*'},
  {'2','5','8','0'},
  {'3','6','9','#'}
};
byte rowPins[ROWS] = {7, 6, 5};           // columnas físicas → ahora son filas (OUTPUT)
byte colPins[COLS] = {0, 1, 10, 9};       // filas físicas    → ahora son cols (INPUT_PULLUP)
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String validCodes[PARCEL_COUNT];
bool   codesReceived = false;
String inputCode     = "";
String rxBuffer      = "";

#define TEST_INTERVAL_MS 5000   // envía temp cada 5 s
unsigned long lastTestMs = 0;

// ---------------------------------------------------------------------------
// LED feedback
// ---------------------------------------------------------------------------
void blinkOK() {
  // 3 blinks rápidos = código válido
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);  delay(80);
    digitalWrite(LED_PIN, HIGH); delay(80);
  }
}

void blinkFail() {
  // 3 blinks lentos = código inválido
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);  delay(400);
    digitalWrite(LED_PIN, HIGH); delay(400);
  }
}

// ---------------------------------------------------------------------------
// Parsea "CODES:111111,222222,..." y guarda en validCodes[]
// ---------------------------------------------------------------------------
void parseCodes(const String& msg) {
  String csv = msg.substring(6);  // quita "CODES:"
  int idx = 0, start = 0;
  while (idx < PARCEL_COUNT) {
    int comma = csv.indexOf(',', start);
    String code = (comma < 0) ? csv.substring(start)
                               : csv.substring(start, comma);
    code.trim();
    validCodes[idx++] = code;
    if (comma < 0) break;
    start = comma + 1;
  }
  codesReceived = true;
  Serial.println("[NODE] Códigos recibidos del gateway.");
  // Blink 5x rápido para confirmar
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LOW);  delay(60);
    digitalWrite(LED_PIN, HIGH); delay(60);
  }
}

// ---------------------------------------------------------------------------
// Valida el código ingresado contra el array
// ---------------------------------------------------------------------------
void validateCode(const String& code) {
  Serial.print("[NODE] Validando: ");
  Serial.println(code);
  if (!codesReceived) {
    Serial.println("[NODE] Sin códigos cargados aún.");
    blinkFail();
    return;
  }
  for (int i = 0; i < PARCEL_COUNT; i++) {
    if (validCodes[i] == code) {
      Serial.print("[NODE] ACCESO OK — parcela ");
      Serial.println(i + 1);
      // Notifica al gateway para que registre en el log
      String logMsg = "LOG:" + code + "," + String(i + 1) + "\n";
      Serial1.print(logMsg);
      blinkOK();
      return;
    }
  }
  Serial.println("[NODE] CÓDIGO INVÁLIDO");
  blinkFail();
}

// ---------------------------------------------------------------------------
// Espera a que AUX baje (LOW=ocupado) y luego suba (HIGH=listo)
// Timeout 3 s en cada fase para no bloquear indefinidamente
// ---------------------------------------------------------------------------
void waitAuxReady() {
  unsigned long t = millis();
  while (digitalRead(E32_AUX_PIN) == HIGH && millis() - t < 3000);
  Serial.print("[AUX] LOW (ocupado) @ "); Serial.println(millis());
  t = millis();
  while (digitalRead(E32_AUX_PIN) == LOW  && millis() - t < 3000);
  Serial.print("[AUX] HIGH (listo)  @ "); Serial.println(millis());
}

// ---------------------------------------------------------------------------
void setup() {
  pinMode(LED_PIN,    OUTPUT);
  pinMode(E32_AUX_PIN, INPUT);
  digitalWrite(LED_PIN, HIGH);  // active LOW

  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, LOW);  delay(100);
    digitalWrite(LED_PIN, HIGH); delay(100);
  }

  Serial1.begin(E32_BAUD, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);

  // Mensaje de test para verificar transmisión via AUX
  Serial.println("[NODE] Enviando mensaje de test...");
  Serial.print("[AUX] antes de enviar: ");
  Serial.println(digitalRead(E32_AUX_PIN) == HIGH ? "HIGH" : "LOW");
  Serial1.println("TEST:node-ok");
  waitAuxReady();
  Serial.println("[NODE] Transmisión de test completada.");

  Serial.println("[NODE] Listo. Ingresá el código y presioná #.");
}

// ---------------------------------------------------------------------------
void loop() {
  // --- Recibir mensajes LoRa entrantes ---
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      rxBuffer.trim();
      if (rxBuffer.startsWith("CODES:")) {
        parseCodes(rxBuffer);
      }
      rxBuffer = "";
    } else {
      rxBuffer += c;
    }
  }

  // --- Envío periódico de temperatura random (test de cables) ---
  if (millis() - lastTestMs >= TEST_INTERVAL_MS) {
    lastTestMs = millis();
    float temp = 15.0 + random(0, 200) / 10.0;  // 15.0 – 34.9 °C
    String msg = "TEMP:" + String(temp, 1);
    Serial.print("[TEST] Enviando: "); Serial.println(msg);
    Serial.print("[AUX] "); Serial.println(digitalRead(E32_AUX_PIN) == HIGH ? "HIGH" : "LOW");
    Serial1.println(msg);
    waitAuxReady();
  }

  // --- Leer teclado ---
  char key = keypad.getKey();
  if (!key) return;

  if (key == '#') {
    // Confirmar
    if (inputCode.length() == CODE_LEN) {
      validateCode(inputCode);
    } else {
      Serial.println("[NODE] Código incompleto (necesita 6 dígitos).");
      blinkFail();
    }
    inputCode = "";
  } else if (key == '*') {
    // Limpiar
    inputCode = "";
    Serial.println("[NODE] Entrada borrada.");
  } else {
    if (inputCode.length() < CODE_LEN) {
      inputCode += key;
      Serial.print("[NODE] Ingresado: ");
      Serial.println(inputCode);
    }
  }
}
