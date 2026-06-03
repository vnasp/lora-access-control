// node.ino — ESP32-C3 Super Mini
// Nodo de acceso: recibe códigos diarios por LoRa, valida entrada de teclado 4x3

#include <Keypad.h>

#define LED_PIN      8
#define PARCEL_COUNT 15
#define CODE_LEN     6

// E32-433T30D (UART transparente) — M0 y M1 conectados a GND (modo normal)
#define E32_TX_PIN   3
#define E32_RX_PIN   4
#define E32_AUX_PIN  2
#define E32_BAUD    9600

// Teclado 4x3 — filas/cols invertidos: GPIO9 queda como INPUT para evitar
// conflicto con botón BOOT del ESP32-C3
const byte ROWS = 3;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','4','7','*'},
  {'2','5','8','0'},
  {'3','6','9','#'}
};
byte rowPins[ROWS] = {7, 6, 5};
byte colPins[COLS] = {0, 1, 10, 9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String validCodes[PARCEL_COUNT];
bool   entryDone[PARCEL_COUNT];
bool   codesReceived = false;
int    codesCount    = 0;
String inputCode     = "";
String rxBuffer      = "";

// ---------------------------------------------------------------------------
void blinkOK() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);  delay(80);
    digitalWrite(LED_PIN, HIGH); delay(80);
  }
}

void blinkFail() {
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
  codesCount    = idx;
  for (int i = 0; i < PARCEL_COUNT; i++) entryDone[i] = false;
  Serial.print("[NODE] Códigos recibidos: "); Serial.println(idx);
}

// ---------------------------------------------------------------------------
void validateCode(const String& code) {
  if (!codesReceived) {
    Serial.println("[NODE] Sin códigos cargados aún.");
    blinkFail();
    return;
  }
  for (int i = 0; i < PARCEL_COUNT; i++) {
    if (validCodes[i] == code) {
      Serial.print("[NODE] ACCESO OK — parcela ");
      Serial.println(i + 1);

      if (!entryDone[i]) {
        Serial1.println("LOG:" + code + "," + String(i + 1) + ",ENTRY");
        entryDone[i] = true;
      } else {
        Serial1.println("LOG:" + code + "," + String(i + 1) + ",EXIT");
        validCodes[i] = "";
        entryDone[i] = false;
      }

      blinkOK();
      return;
    }
  }
  Serial.println("[NODE] CÓDIGO INVÁLIDO");
  blinkFail();
}

// ---------------------------------------------------------------------------
void setup() {
  pinMode(LED_PIN,     OUTPUT);
  pinMode(E32_AUX_PIN, INPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(115200);
  Serial1.begin(E32_BAUD, SERIAL_8N1, E32_RX_PIN, E32_TX_PIN);

  for (int i = 0; i < PARCEL_COUNT; i++) entryDone[i] = false;
}

// ---------------------------------------------------------------------------
void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      rxBuffer.trim();
      int codesIdx = rxBuffer.indexOf("CODES:");
      if (codesIdx >= 0) parseCodes(rxBuffer.substring(codesIdx));
      rxBuffer = "";
    } else {
      rxBuffer += c;
    }
  }

  char key = keypad.getKey();
  if (!key) return;

  if (key == '#') {
    if (inputCode.length() == CODE_LEN) {
      validateCode(inputCode);
    } else {
      Serial.println("[NODE] Código incompleto (necesita 6 dígitos).");
      blinkFail();
    }
    inputCode = "";
  } else if (key == '*') {
    inputCode = "";
  } else {
    if (inputCode.length() < CODE_LEN) inputCode += key;
  }
}
