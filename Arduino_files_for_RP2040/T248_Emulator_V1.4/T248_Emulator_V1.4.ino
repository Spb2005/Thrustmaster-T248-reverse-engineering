#define UART_TX_PIN 0
#define UART_RX_PIN 1

unsigned long lastPeriodicSend = 0;
const unsigned long PERIOD_MS = 250;  // Periodic message rate (4 Hz)

// Momentary button handling
unsigned long lastButtonTime = 0;
const unsigned long BUTTONDELAY_MS = 100;
bool btnpressed = 0;

// Wheelbase initialization sequence
const uint8_t initSeq[8] = {
  0xF4, 0x02, 0x00, 0x02, 0xAA, 0xE1, 0xFF, 0xDE
};

// Button state bytes (mapped to B0 frame payload)
uint8_t btnByte1 = 0;
uint8_t btnByte2 = 0;
uint8_t btnByte3 = 0;

// UART receive state machine
enum UartState {
  UART_IDLE,
  UART_RECV_DX,  // D0–D3 keep-alive frames
  UART_RECV_F3,  // F3 general data frame
  UART_RECV_42   // 0x42 screen/data frame
};

UartState uartState = UART_IDLE;

// ---- D0–D3 keep-alive buffer ----
uint8_t dxBuffer[4];
uint8_t dxCount = 0;

// ---- F3 frame buffer ----
uint8_t f3Buffer[18];
uint8_t f3Count = 0;

// ---- 0x42 screen frame buffer ----
uint8_t x42Buffer[32];  // variable length
uint8_t x42Count = 0;
uint8_t x42Length = 3;


void setup() {

  // USB serial for debugging and command input
  Serial.begin(115200);

  // UART0 (Serial1) connection to wheelbase
  Serial1.setTX(UART_TX_PIN);
  Serial1.setRX(UART_RX_PIN);

  Serial1.begin(115200, SERIAL_8E1);

  Serial.println("Wheelbase Start");

  // Short delay to allow wheelbase startup
  delay(100);

  // Send initialization sequence twice
  Serial.println("init seq");
  Serial1.write(initSeq, sizeof(initSeq));
  Serial1.flush();
  delayMicroseconds(100);  // Small inter-frame delay
  Serial1.write(initSeq, sizeof(initSeq));
}

void loop() {

  // Handle all incoming UART data via state machine
  handleUART();

  // Periodic button state transmission
  unsigned long now = millis();
  if (now - lastPeriodicSend >= PERIOD_MS) {
    lastPeriodicSend = now;
    sendButtons(1);
  }

  // Reset momentary buttons after timeout (debugging)
  if (btnpressed) {
    unsigned long now = millis();
    if (now - lastButtonTime >= BUTTONDELAY_MS) {
      btnpressed = 0;
      btnByte1 = 0x00;
      btnByte2 = 0x00;
      btnByte3 = 0x00;
      sendButtons(1);
    }
  }

  // USB serial console commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);
  }
}

// UART receive state machine
void handleUART() {
  while (Serial1.available()) {
    uint8_t b = Serial1.read();

    switch (uartState) {

      case UART_IDLE:

        // ---- D0–D3 Keep Alive ----
        if (b == 0xD0 || b == 0xD1 || b == 0xD2 || b == 0xD3) {
          dxBuffer[0] = b;
          dxCount = 1;
          uartState = UART_RECV_DX;
        }

        // ---- F3 General Data Frame ----
        else if (b == 0xF3) {
          f3Buffer[0] = b;
          f3Count = 1;
          uartState = UART_RECV_F3;
        }

        // ---- 0x42 Screen/Data Frame ----
        else if (b == 0x42) {
          x42Buffer[0] = b;
          x42Count = 1;
          uartState = UART_RECV_42;
        }

        // Unknown or unhandled byte
        else {
          Serial.printf("%0.2x\n", b);
        }

        break;

      // ---- Receive remaining 3 bytes of D0–D3 frame ----
      case UART_RECV_DX:
        dxBuffer[dxCount++] = b;

        if (dxCount == 4) {

          uint8_t response[4] = { 0xF0, 0x00, 0x00, 0x00 };
          Serial1.write(response, 4);


          uartState = UART_IDLE;
        }
        break;

      // ---- Receive remaining 17 bytes of F3 frame ----
      case UART_RECV_F3:
        f3Buffer[f3Count++] = b;

        if (f3Count == 18) {
          processF3Frame(f3Buffer);
          uartState = UART_IDLE;
        }
        break;

      // ---- Receive variable-length 0x42 frame ----
      case UART_RECV_42:
        x42Buffer[x42Count++] = b;

        // Length byte is at position 2
        if (x42Count == 3) {
          x42Length = b;
        }

        if (x42Count == x42Length) {
          Serial.print("0X42 Frame: ");
          printBuffer(x42Buffer, x42Length);

          // Response to screen frame (no observed functional effect)
          uint8_t response[12] = {
            0x47, 0x0C, 0x2A, 0x02, 0x01, 0x00,
            0x80, 0x00, 0x00, 0x00, 0x00, 0x00
          };
          Serial1.write(response, 12);

          uartState = UART_IDLE;
        }
        break;
    }
  }
}

// Decode selected fields from F3 frame (encoder-related data)
void processF3Frame(uint8_t *buf) {
  Serial.print("F3 Frame: ");
  printBuffer(buf, 18);

  if (buf[5] == 0x45) {
    uint8_t val7 = buf[7] & 0b00000111;
    String result = "E" + String(val7);

    uint8_t b9 = buf[9];
    bool bit2 = b9 & (1 << 1);
    bool bit3 = b9 & (1 << 2);

    if (!bit2 && !bit3) {
      if (b9 & (1 << 4)) {
        result += "P";
      }
    } else if (bit3 && !bit2) {
      result += "-";
    } else if (bit2 && !bit3) {
      result += "+";
    }

    Serial.print("Status decoded: ");
    Serial.println(result);
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

  // ---------- Special Buttons ----------
  // Potentially dangerous: can change wheel settings without feedback
  // else if (cmd == "display") toggleBit(btnByte1, 1);
  // else if (cmd == "mode")    toggleBit(btnByte1, 2);

  else {
    Serial.println("Unknown command");
    return;
  }

  // Mark button as momentary press
  btnpressed = 1;
  lastButtonTime = millis();

  Serial.println("Button triggered");

  sendButtons(0);
}

// Toggle a single bit in a button state byte
void toggleBit(uint8_t &byte, uint8_t bit) {
  byte ^= (1 << bit);
}

// Send button frames to wheelbase
// full = true  -> send B0 and B1
// full = false -> send B0 only
void sendButtons(bool full) {

  uint8_t msgB0[4] = { 0xB0, btnByte1, btnByte2, btnByte3 };
  uint8_t msgB1[4] = { 0xB1, 0x00, 0x00, 0x00 };

  Serial1.write(msgB0, 4);
  Serial1.flush();
  delayMicroseconds(500);

  if (full) {
    Serial1.write(msgB1, 4);
  }
  //Serial.printf("Button Message: %x %x %x %x\n", msgB0[0], msgB0[1], msgB0[2], msgB0[3]);
}

// Print raw frame data for debugging
void printBuffer(uint8_t *buf, int count) {
  for (int i = 0; i < count; i++) {
    Serial.printf("%02X ", buf[i]);
  }
  Serial.println();
}
