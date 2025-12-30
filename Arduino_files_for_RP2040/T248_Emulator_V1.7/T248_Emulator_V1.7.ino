#include <Rotary.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hardware/watchdog.h"
#include <RPi_Pico_TimerInterrupt.h>

//Uart Pins
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define START_PIN 11

//I2C Pins
#define I2C_SDA 4
#define I2C_SCL 5

//encoder Pins
#define RotaryPinA 27
#define RotaryPinB 28
#define RotaryButton 26

//I2C
#define NUM_LAYERS 4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SSD1306_I2C_ADDRESS 0x3C
#define OLED_RESET -1

//timeout for different states ~3X max. frame length
#define TIMEOUT_DX_MS 5   // 4 Bytes
#define TIMEOUT_F3_MS 10  // 18 Bytes
#define TIMEOUT_42_MS 10  // max 16 Bytes

#define AUTOREBOOT 0

RPI_PICO_Timer ITimer0(0);
Rotary rotary = Rotary(RotaryPinB, RotaryPinA);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//4 Hz Button Message
unsigned long lastPeriodicSend = 0;
const unsigned long PERIOD_MS = 250;  // 4 Hz

//Button Toggle timer
unsigned long lastButtonTime = 0;
const unsigned long BUTTONDELAY_MS = 100;
bool btnpressed = 0;

//state machine timeout
unsigned long uartStateStartMs = 0;
unsigned long uartStateTimeoutMs = 0;

//for Encoder pulse
unsigned long lastEncoderTime = 0;
const unsigned long ENCODERIMPULSE_MS = 250;

volatile int currentLayer = 0;
char lastSymbol = '\0';  // saves current symbol ('+', '-', 'p', '\0' for no symbol)
bool displayNeedsUpdate = true;

const char* variableNames[NUM_LAYERS] = { "TC", "ABS", "BB", "ECU" };
//const char* variableNames[NUM_LAYERS] = { "E1", "E2", "E3", "E4" };

//for startup sequence
const uint8_t initSeq[8] = { 0xF4, 0x02, 0x00, 0x02, 0xAA, 0xE1, 0xFF, 0xDE };
const uint8_t responseSeq[] = { 0xF0, 0x00, 0x00, 0x00 };
const unsigned long TIMEOUT_MS = 5000;

// UART receive state machine
enum UartState {
  UART_IDLE,
  UART_RECV_DX,  // D0–D3 keep-alive frames
  UART_RECV_F3,  // F3 general data frame
  UART_RECV_42   // 0x42 screen/data frame
};

UartState uartState = UART_IDLE;

// Button state bytes (mapped to B0 frame payload)
uint8_t btnByte1 = 0;
uint8_t btnByte2 = 0;
uint8_t btnByte3 = 0;

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

  Serial.begin(115200);

  pinMode(RotaryButton, INPUT);

  attachInterrupt(RotaryPinB, rotate, CHANGE);
  attachInterrupt(RotaryPinA, rotate, CHANGE);
  attachInterrupt(RotaryButton, changeLayer, FALLING);

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();

  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(28, 9);
  display.println("Init");
  display.display();

  Serial1.setTX(UART_TX_PIN);
  Serial1.setRX(UART_RX_PIN);
  Serial1.begin(115200, SERIAL_8E1);

  Serial.println("Wheelbase Start");

  delay(100);  //100 ms delay, to start up wheelbase

  sendInitSequence();

  if (!waitForResponse()) {
    Serial.println("Timeout → reboot");
    delay(1000);
#if AUTOREBOOT
    rebootPico();
#endif
  }

  display.clearDisplay();
  display.setCursor(28, 9);
  display.println("T248");
  display.display();
  displayNeedsUpdate = false;
  ITimer0.attachInterruptInterval(6000 * 1000, resetSymbol);
}

void sendInitSequence() {
  Serial.println("init seq");
  Serial1.write(initSeq, sizeof(initSeq));
  Serial1.flush();
  delayMicroseconds(100);  // ca. 1 Byte
  Serial1.write(initSeq, sizeof(initSeq));
}

void rebootPico() {
  watchdog_reboot(0, 0, 0);
  while (true)
    ; 
}

bool waitForResponse() {
  unsigned long startTime = millis();
  uint8_t matchIndex = 0;

  while (millis() - startTime < TIMEOUT_MS) {
    while (Serial1.available()) {
      uint8_t b = Serial1.read();

      if (b == responseSeq[matchIndex]) {
        matchIndex++;
        if (matchIndex == sizeof(responseSeq)) {
          return true; //Start sequence succesful
        }
      } else {
        matchIndex = (b == responseSeq[0]) ? 1 : 0;
      }
    }
  }
  return false;  // Timeout
}

void loop() {
  handleUART();

  //periodic button messages
  unsigned long now = millis();
  if (now - lastPeriodicSend >= PERIOD_MS) {
    lastPeriodicSend = now;
    sendButtons(1);
  }

  // to reset buttons while debugging
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

  //only for debugging
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);
  }

  //for display and encoder pulses
  now = millis();
  if (now - lastEncoderTime >= ENCODERIMPULSE_MS) {
    setSymbol('\0');
    setBit(btnByte1, 3, 0);
    setBit(btnByte1, 4, 0);
    setBit(btnByte1, 5, 0);
    setBit(btnByte1, 6, 0);
    sendButtons(0);
  }

  updateDisplay();
}

void rotate() {
  unsigned char result = rotary.process();
  unsigned long now = millis();

  if (result == DIR_CW) {
    Serial.println("CW");
    setBit(btnByte1, 6, 1);
  } else if (result == DIR_CCW) {
    Serial.println("CCW");
    setBit(btnByte1, 5, 1);
  }

  lastEncoderTime = now;
}

void changeLayer() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  unsigned long currentPress = now;

  // debouncing
  if (currentPress - lastPress > 200) {
    setBit(btnByte1, 3, 1);
    Serial.println("Layer wechsel");
    lastEncoderTime = now;
  }
  lastPress = currentPress;
}

bool resetSymbol(repeating_timer* rt) {
  setSymbol('\0');
  displayNeedsUpdate = true; 
  return true;
}

void updateDisplay() {
  if (!displayNeedsUpdate) return;

  display.clearDisplay();

  int textVerticalOffset = 5;
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(variableNames[currentLayer], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2 + textVerticalOffset);
  display.println(variableNames[currentLayer]);

  if (lastSymbol == '+') {
    display.setCursor(SCREEN_WIDTH - 24, (SCREEN_HEIGHT - 24) / 2 + textVerticalOffset);
    display.println("+");
  } else if (lastSymbol == '-') {
    display.setCursor(0, (SCREEN_HEIGHT - 24) / 2 + textVerticalOffset);
    display.println("-");
  } else if (lastSymbol == 'p') {
    display.setCursor(0, (SCREEN_HEIGHT - 24) / 2 + textVerticalOffset);
    display.println("P");
    display.setCursor(SCREEN_WIDTH - 24, (SCREEN_HEIGHT - 24) / 2 + textVerticalOffset);
    display.println("P");
  }

  display.display();
  displayNeedsUpdate = false;
}

void setSymbol(char symbol) {
  if (lastSymbol != symbol) {
    lastSymbol = symbol;
    displayNeedsUpdate = true;
  }
}

void handleUART() {
  if (uartState != UART_IDLE) {
    if (millis() - uartStateStartMs > uartStateTimeoutMs) {
      Serial.println("UART RX timeout → reset state");
      uartState = UART_IDLE;
    }
  }

  while (Serial1.available()) {
    uint8_t b = Serial1.read();

    uartStateStartMs = millis();

    switch (uartState) {

      case UART_IDLE:

        // ---- D0–D3 Keep Alive ----
        if (b == 0xD0 || b == 0xD1 || b == 0xD2 || b == 0xD3) {
          dxBuffer[0] = b;
          dxCount = 1;
          setUartState(UART_RECV_DX, TIMEOUT_DX_MS);
        }

        // ---- F3 Status Frame ----
        else if (b == 0xF3) {
          f3Buffer[0] = b;
          f3Count = 1;
          setUartState(UART_RECV_F3, TIMEOUT_F3_MS);
        }

        // ------- 42 Screen Frame ------
        else if (b == 0x42) {
          x42Buffer[0] = b;
          x42Count = 1;
          setUartState(UART_RECV_42, TIMEOUT_42_MS);
        }

        else {
          Serial.printf("%02x\n", b);
        }

        break;

      // D0–D3: reading 3 Bytes
      case UART_RECV_DX:
        dxBuffer[dxCount++] = b;

        if (dxCount == 4) {
          // send response
          uint8_t response[4] = { 0xF0, 0x00, 0x00, 0x00 };
          Serial1.write(response, 4);
          uartState = UART_IDLE;
        }
        break;

      // F3: reading 17 Bytes
      case UART_RECV_F3:
        f3Buffer[f3Count++] = b;

        if (f3Count == 18) {
          processF3Frame(f3Buffer);
          uartState = UART_IDLE;
        }
        break;

      case UART_RECV_42:
        x42Buffer[x42Count++] = b;

        //extract Frame length from 3th Byte
        if (x42Count == 3) {
          x42Length = b;
        }

        if (x42Count == x42Length) {
          Serial.print("0X42 Frame: ");
          printBuffer(x42Buffer, x42Length);
          uint8_t response[12] = { 0x47, 0x0C, 0x2A, 0x02, 0x01, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
          Serial1.write(response, 12);
          uartState = UART_IDLE;
        }
        break;
    }
  }
}

void setUartState(UartState newState, unsigned long timeoutMs) {
  uartState = newState;
  uartStateStartMs = millis();
  uartStateTimeoutMs = timeoutMs;
}

void processF3Frame(uint8_t* buf) {
  Serial.print("F3 Frame: ");
  printBuffer(buf, 18);

  if (buf[5] == 0x45) {
    //extracting current layer from Byte 7
    uint8_t val7 = buf[7] & 0b00000111;
    currentLayer = val7 - 1;
    setSymbol('\0');
    displayNeedsUpdate = true;
    String result = "E" + String(val7);

    uint8_t b9 = buf[9];
    bool bit2 = b9 & (1 << 1);
    bool bit3 = b9 & (1 << 2);

    //extracting state Change from Byte 9
    if (!bit2 && !bit3) {
      if (b9 & (1 << 4)) {
        setSymbol('p');
        ITimer0.attachInterruptInterval(250 * 1000, resetSymbol);
        result += "P";
      }
    } else if (bit3 && !bit2) {
      setSymbol('-');
      ITimer0.attachInterruptInterval(250 * 1000, resetSymbol);
      result += "-";
    } else if (bit2 && !bit3) {
      setSymbol('+');
      ITimer0.attachInterruptInterval(250 * 1000, resetSymbol);
      result += "+";
    }

    Serial.print("Status decoded: ");
    Serial.println(result);
  }
}

void handleCommand(String cmd) {
  //only for debugging
  cmd.trim();

  // ---------- Buttons 1–8 ----------
  if (cmd.toInt() >= 1 && cmd.toInt() <= 8) {
    toggleBit(btnByte3, cmd.toInt() - 1);
  }

  // ---------- Buttons 9–10 ----------
  else if (cmd == "9")
    toggleBit(btnByte2, 0);
  else if (cmd == "10") toggleBit(btnByte2, 1);

  // ---------- other Buttons ----------
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

  // ---------- special Buttons ----------
  //potentially dangerous, because ist can change settings without you knowing
  //else if (cmd == "display") toggleBit(btnByte1, 1);
  //else if (cmd == "mode") toggleBit(btnByte1, 2);

  else {
    Serial.println("Unknown command");
    return;
  }

  btnpressed = 1;  //to reset buttons later
  unsigned long now = millis();
  lastButtonTime = now;
  Serial.println("Button triggered");

  sendButtons(0);
}

void toggleBit(uint8_t& byte, uint8_t bit) {
  byte ^= (1 << bit);
}

void setBit(uint8_t& byte, uint8_t bit, bool state) {
  if (state) {
    byte |= (1 << bit);
  } else {
    byte &= ~(1 << bit);
  }
}

void sendButtons(bool full) {

  uint8_t msgB0[4] = { 0xB0, btnByte1, btnByte2, btnByte3 };
  uint8_t msgB1[4] = { 0xB1, 0x00, 0x00, 0x00 };

  Serial1.write(msgB0, 4);
  Serial1.flush();
  delayMicroseconds(500);  // 0,5 ms
  if (full) {
    Serial1.write(msgB1, 4);
  }
  //Serial.printf("Button Message: %x %x %x %x\n", msgB0[0], msgB0[1], msgB0[2], msgB0[3]);
}

void printBuffer(uint8_t* buf, int count) {
  for (int i = 0; i < count; i++) {
    Serial.printf("%02X ", buf[i]);
  }
  Serial.println();
}
