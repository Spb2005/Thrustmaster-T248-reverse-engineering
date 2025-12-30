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
#define I2C_SDA 8
#define I2C_SCL 9

//encoder Pins
#define RotaryPinA 27
#define RotaryPinB 28
#define RotaryButton 26

// Pins für DPAD
#define BUTTON_UP_PIN 3
#define BUTTON_DOWN_PIN 4
#define BUTTON_LEFT_PIN 5
#define BUTTON_RIGHT_PIN 2

//Shift pins (Btn 1+2)
#define BUTTON_INT_1 10
#define BUTTON_INT_2 22
//11 other normal Buttons
#define NUM_ADDITIONAL_BUTTONS 11

//Pins for Button 3 - 10, 22, 23, 13
const int additionalButtonPins[NUM_ADDITIONAL_BUTTONS] = { 12, 15, 14, 13, 19, 18, 17, 16, 21, 20, 6 };
bool btnSendUpdate = false;

volatile bool intBtn1Changed = false;
volatile bool intBtn2Changed = false;

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
#define DEBUG 0

#if DEBUG
#define SPrint(x) Serial.println(x)
#else
#define SPrint(x)
#endif

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
volatile unsigned long lastEncoderTime = 0;
unsigned long lastEncoderPress = 0;
const unsigned long ENCODERIMPULSE_MS = 250;

//for Encoder ISR
volatile int8_t encoderDelta = 0;
volatile bool encButtonPressed = false;

volatile int currentLayer = 0;
char lastSymbol = '\0';  // saves current symbol ('+', '-', 'p', '\0' for no symbol)
volatile bool displayNeedsUpdate = true;

const char* variableNames[NUM_LAYERS] = { "TC", "ABS", "BB", "ECU" };
//const char* variableNames[NUM_LAYERS] = { "E1", "E2", "E3", "E4" };

//for startup sequence
const uint8_t initSeq[8] = { 0xF4, 0x02, 0x00, 0x02, 0xAA, 0xE1, 0xFF, 0xDE };
const uint8_t responseSeq[] = { 0xF0, 0x00, 0x00, 0x00 };
const unsigned long TIMEOUT_MS = 5000;

//state Machine
enum UartState {
  UART_IDLE,
  UART_RECV_DX,
  UART_RECV_F3,
  UART_RECV_42
};

UartState uartState = UART_IDLE;

//B0 Bytes
volatile uint8_t btnByte1 = 0;
volatile uint8_t btnByte2 = 0;
volatile uint8_t btnByte3 = 0;

//earlier B0 Bytes to detect changes
uint8_t lastBtnByte1 = 0;
uint8_t lastBtnByte2 = 0;
uint8_t lastBtnByte3 = 0;

uint8_t dxBuffer[4];
uint8_t dxCount = 0;

uint8_t f3Buffer[18];
uint8_t f3Count = 0;

uint8_t x42Buffer[32];  // messages are differently long
uint8_t x42Count = 0;
uint8_t x42Length = 3;

void INT1_Button() {
  intBtn1Changed = true;
}

void INT2_Button() {
  intBtn2Changed = true;
}

//ISR for Encoder
void rotate() {
  unsigned char result = rotary.process();

  if (result == DIR_CW) {
    SPrint("CW");
    encoderDelta++;
  } else if (result == DIR_CCW) {
    SPrint("CCW");
    encoderDelta--;
  }
}

//ISR for Encoder Button
void changeLayer() {
  encButtonPressed = true;
}

void setup() {
#if DEBUG
  Serial.begin(115200);
#endif
  for (int i = 0; i < NUM_ADDITIONAL_BUTTONS; i++) {
    pinMode(additionalButtonPins[i], INPUT);
  }
  //Dpad
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  //encoder
  pinMode(RotaryButton, INPUT);

  //Shifter with interrupt
  pinMode(BUTTON_INT_1, INPUT_PULLUP);
  pinMode(BUTTON_INT_2, INPUT_PULLUP);
  attachInterrupt(BUTTON_INT_1, INT1_Button, CHANGE);
  attachInterrupt(BUTTON_INT_2, INT2_Button, CHANGE);

  attachInterrupt(RotaryPinB, rotate, CHANGE);
  attachInterrupt(RotaryPinA, rotate, CHANGE);
  attachInterrupt(RotaryButton, changeLayer, FALLING);

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();

  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    SPrint("SSD1306 allocation failed");
    while (1) {}
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

  SPrint("Wheelbase Start");
  delay(100);  //100 ms delay, to start up wheelbase

  sendInitSequence();

  if (!waitForResponse()) {
    SPrint("Timeout → reboot");
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
  // remove "T248" after startup sequence is finished
  ITimer0.attachInterruptInterval(6000 * 1000, resetSymbol); 
}

void loop() {
  handleUART();

  readGPIOButtons();

  //reset encoder pulse + display message
  unsigned long now = millis();
  if (now - lastEncoderTime >= ENCODERIMPULSE_MS) {
    setSymbol('\0');
    setBit(btnByte1, 3, 0);
    setBit(btnByte1, 4, 0);
    setBit(btnByte1, 5, 0);
    setBit(btnByte1, 6, 0);
  }

  //Read Interrupt Button state
  handleInterruptButtons();

  //apply encoder delta
  handleInterruptEncoder();

  handleEncoderButton();

  //check if B0 Frame has changed
  detectButtonChange();

  //if B0 frame changed -> send
  if (btnSendUpdate) {
    sendButtons(0);
    btnSendUpdate = false;
  }

  //periodic button messages
  now = millis();
  if (now - lastPeriodicSend >= PERIOD_MS) {
    lastPeriodicSend = now;
    sendButtons(1);
  }

  updateDisplay();
#if DEBUG
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
#endif
}

void handleUART() {
  if (uartState != UART_IDLE) {
    if (millis() - uartStateStartMs > uartStateTimeoutMs) {
      SPrint("UART RX timeout → reset state");
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
#if DEBUG
        else {
          Serial.printf("%02x\n", b);
        }
#endif

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
          //to prevent buffer overrun
          if (b < 3 || b > sizeof(x42Buffer)) {
            uartState = UART_IDLE;
            break;
          }
          x42Length = b;
        }

        if (x42Count == x42Length) {
#if DEBUG
          Serial.print("0X42 Frame: ");
          printBuffer(x42Buffer, x42Length);
#endif
          uint8_t response[12] = { 0x47, 0x0C, 0x2A, 0x02, 0x01, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
          Serial1.write(response, 12);
          uartState = UART_IDLE;
        }
        break;
    }
  }
}

void readGPIOButtons() {
  // --- DPAD ---
  setBit(btnByte1, 0, !digitalRead(BUTTON_RIGHT_PIN));
  setBit(btnByte2, 5, !digitalRead(BUTTON_UP_PIN));
  setBit(btnByte2, 6, !digitalRead(BUTTON_DOWN_PIN));
  setBit(btnByte2, 7, !digitalRead(BUTTON_LEFT_PIN));

  // --- Buttons 3–8 ---
  for (int i = 0; i < 6; i++) {
    setBit(btnByte3, i + 2, !digitalRead(additionalButtonPins[i]));
  }

  // --- Buttons 9,10,22,23,13 ---
  setBit(btnByte2, 0, !digitalRead(additionalButtonPins[6]));
  setBit(btnByte2, 1, !digitalRead(additionalButtonPins[7]));
  setBit(btnByte2, 2, !digitalRead(additionalButtonPins[8]));
  setBit(btnByte2, 3, !digitalRead(additionalButtonPins[9]));
  setBit(btnByte2, 4, !digitalRead(additionalButtonPins[10]));
}

void handleInterruptButtons() {
  if (intBtn1Changed) {
    intBtn1Changed = false;
    setBit(btnByte3, 0, !digitalRead(BUTTON_INT_1));
  }

  if (intBtn2Changed) {
    intBtn2Changed = false;
    setBit(btnByte3, 1, !digitalRead(BUTTON_INT_2));
  }
}

void handleInterruptEncoder() {
  if (encoderDelta != 0) {
    if (encoderDelta > 0) {
      setBit(btnByte1, 6, 1);
    } else {
      setBit(btnByte1, 5, 1);
    }
    encoderDelta = 0;
    lastEncoderTime = millis();
  }
}

void handleEncoderButton() {
  if (encButtonPressed) {
    encButtonPressed = false;
    unsigned long now = millis();
    if (now - lastEncoderPress > 200) {
      setBit(btnByte1, 3, 1);
      lastEncoderTime = now;
    }
    lastEncoderPress = now;
  }
}

void detectButtonChange() {
  if (btnByte1 != lastBtnByte1 || btnByte2 != lastBtnByte2 || btnByte3 != lastBtnByte3) {
    btnSendUpdate = true;
    lastBtnByte1 = btnByte1;
    lastBtnByte2 = btnByte2;
    lastBtnByte3 = btnByte3;
  }
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

void setUartState(UartState newState, unsigned long timeoutMs) {
  uartState = newState;
  uartStateStartMs = millis();
  uartStateTimeoutMs = timeoutMs;
}

void processF3Frame(uint8_t* buf) {
#if DEBUG
  Serial.print("F3 Frame: ");
  printBuffer(buf, 18);
#endif

  if (buf[5] == 0x45) {
    //extracting current layer from Byte 7
    uint8_t val7 = buf[7] & 0b00000111;
    if (val7 >= 1 && val7 <= NUM_LAYERS) {
      currentLayer = val7 - 1;
    }
    setSymbol('\0');
    displayNeedsUpdate = true;
#if DEBUG
    String result = "E" + String(val7);
#endif

    uint8_t b9 = buf[9];
    bool bit2 = b9 & (1 << 1);
    bool bit3 = b9 & (1 << 2);

    //extracting state Change from Byte 9
    if (!bit2 && !bit3) {
      if (b9 & (1 << 4)) {
        setSymbol('p');
        ITimer0.attachInterruptInterval(250 * 1000, resetSymbol);
#if DEBUG
        result += "P";
#endif
      }
    } else if (bit3 && !bit2) {
      setSymbol('-');
      ITimer0.attachInterruptInterval(250 * 1000, resetSymbol);
#if DEBUG
      result += "-";
#endif
    } else if (bit2 && !bit3) {
      setSymbol('+');
      ITimer0.attachInterruptInterval(250 * 1000, resetSymbol);
#if DEBUG
      result += "+";
#endif
    }
#if DEBUG
    Serial.print("Status decoded: ");
    Serial.println(result);
#endif
  }
}

void toggleBit(uint8_t& byte, uint8_t bit) {
  byte ^= (1 << bit);
}

void setBit(volatile uint8_t& byte, uint8_t bit, bool state) {
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
  if (full) {
    Serial1.flush();
    delayMicroseconds(500);  // 0,5 ms
    Serial1.write(msgB1, 4);
  }
#if DEBUG
  Serial.printf("Button Message: %x %x %x %x\n", msgB0[0], msgB0[1], msgB0[2], msgB0[3]);
#endif
}

void sendInitSequence() {
  SPrint("init seq");
  Serial1.write(initSeq, sizeof(initSeq));
  Serial1.flush();
  delayMicroseconds(100);  // ~ 1 Byte
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
          return true;  //Response OK
        }
      } else {
        matchIndex = (b == responseSeq[0]) ? 1 : 0;
      }
    }
  }
  return false;  // Timeout
}

#if DEBUG
void printBuffer(uint8_t* buf, int count) {
  for (int i = 0; i < count; i++) {
    Serial.printf("%02X ", buf[i]);
  }
  Serial.println();
}
#endif

#if DEBUG
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
#endif
