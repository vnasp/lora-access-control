// bridge.ino — USB-UART passthrough para flashear ESP32 adyacente
// Serial  (USB CDC) ↔ Serial1 (GPIO20=TX, GPIO21=RX)
// GPIO5 → GPIO9/BOOT del target (mantiene LOW para bootloader)
// RST del target: presionar manualmente el botón RESET del target

#define TARGET_BOOT 5

void setup() {
  // Mantener BOOT del target en LOW permanentemente
  // El usuario debe presionar el botón RESET del target para entrar al bootloader
  pinMode(TARGET_BOOT, OUTPUT);
  digitalWrite(TARGET_BOOT, LOW);

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 21, 20);
}

void loop() {
  while (Serial.available())  Serial1.write(Serial.read());
  while (Serial1.available()) Serial.write(Serial1.read());
}


