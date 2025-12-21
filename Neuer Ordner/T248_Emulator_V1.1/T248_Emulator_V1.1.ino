#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define START_PIN 11 

unsigned long lastPeriodicSend = 0;
const unsigned long PERIOD_MS = 250;  // Periodic message rate (4 Hz)

// Wheelbase initialization sequence
const uint8_t initSeq[8] = {
  0xF4, 0x02, 0x00, 0x02, 0xAA, 0xE1, 0xFF, 0xDE
};

// Button state bytes (mapped to B0 frame payload)
uint8_t btnByte1 = 0;
uint8_t btnByte2 = 0;
uint8_t btnByte3 = 0;

void setup() {

  // USB serial console for interactive control and debugging
  Serial.begin(115200);

  // Start/enable signal from wheelbase
  pinMode(START_PIN, INPUT_PULLDOWN);

  // UART connection to wheelbase
  Serial2.setTX(UART_TX_PIN);
  Serial2.setRX(UART_RX_PIN);
  Serial2.begin(115200, SERIAL_8E1);

  // Wait until wheelbase provides start signal
  while (digitalRead(START_PIN) == LOW) {
    delay(1);
  }
  Serial.println("Wheelbase Start");

  // Allow wheelbase to finish its startup
  delay(650);

  // Send initialization sequence twice
  Serial.println("init seq");
  Serial2.write(initSeq, sizeof(initSeq));
  Serial2.flush();
  delayMicroseconds(100);  // Small inter-frame delay
  Serial2.write(initSeq, sizeof(initSeq));
}

void loop() {

  // Handle incoming keep-alive frames from wheelbase
  if (Serial2.available()) {
    uint8_t firstByte = Serial2.read();
    if (firstByte == 0xD0 || firstByte == 0xD1 ||
        firstByte == 0xD2 || firstByte == 0xD3) {

      uint8_t buffer[3];
      uint8_t count = 0;
      unsigned long startTime = micros();

      // Read remaining bytes with timeout
      while ((micros() - startTime) < 1000 && count < 3) {
        if (Serial2.available()) {
          buffer[count++] = Serial2.read();
        }
      }

      // Send keep-alive response
      uint8_t response[4] = { 0xF0, 0x00, 0x00, 0x00 };
      Serial2.write(response, 4);
      Serial.println("Keep alive");
    }
  }

  // Periodic button state transmission
  unsigned long now = millis();
  if (now - lastPeriodicSend >= PERIOD_MS) {
    lastPeriodicSend = now;

    uint8_t msgB0[4] = { 0xB0, btnByte1, btnByte2, btnByte3 };
    uint8_t msgB1[4] = { 0xB1, 0x00, 0x00, 0x00 };

    Serial2.write(msgB0, 4);
    Serial2.flush();
    delayMicroseconds(500);
    Serial2.write(msgB1, 4);

    // Debug output of current button state
    Serial.printf("Button Message: %x %x %x %x\n",
                  msgB0[0], msgB0[1], msgB0[2], msgB0[3]);
  }

  // Check for incoming USB serial commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);
  }
}

// Parse and handle serial console commands
void handleCommand(String cmd) {
  cmd.trim();

  // ---------- Buttons 1–8 ----------
  if (cmd.toInt() >= 1 && cmd.toInt() <= 8) {
    toggleBit(btnByte3, cmd.toInt() - 1);
  }

  // ---------- Buttons 9–10 ----------
  else if (cmd == "9")
    toggleBit(btnByte2, 0);
  else if (cmd == "10") toggleBit(btnByte2, 1);

  // ---------- Other Buttons ----------
  else if (cmd == "13") toggleBit(btnByte2, 4);
  else if (cmd == "22") toggleBit(btnByte2, 2);
  else if (cmd == "23") toggleBit(btnByte2, 3);

  // ---------- DPAD ----------
  else if (cmd == "dr") toggleBit(btnByte1, 0);
  else if (cmd == "du") toggleBit(btnByte2, 5);
  else if (cmd == "dd") toggleBit(btnByte2, 6);
  else if (cmd == "dl") toggleBit(btnByte2, 7);

  // ---------- Encoder ----------
  else if (cmd == "elu") toggleBit(btnByte1, 3);
  else if (cmd == "eld") toggleBit(btnByte1, 4);
  else if (cmd == "eru") toggleBit(btnByte1, 5);
  else if (cmd == "erd") toggleBit(btnByte1, 6);

  // Potentially dangerous: can change wheel settings without feedback
  // ---------- Special Buttons ----------
  // else if (cmd == "display") toggleBit(btnByte1, 1);
  // else if (cmd == "mode")    toggleBit(btnByte1, 2);

  else {
    Serial.println("Unknown command");
    return;
  }

  Serial.println("Button toggled");
}

// Toggle a single bit in a button state byte
void toggleBit(uint8_t &byte, uint8_t bit) {
  byte ^= (1 << bit);
}
