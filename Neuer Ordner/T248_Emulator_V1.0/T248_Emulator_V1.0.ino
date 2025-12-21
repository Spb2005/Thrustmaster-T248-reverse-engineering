#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define START_PIN 11

unsigned long lastPeriodicSend = 0;
const unsigned long PERIOD_MS = 250;  // 4 Hz

const uint8_t initSeq[8] = {
  0xF4, 0x02, 0x00, 0x02, 0xAA, 0xE1, 0xFF, 0xDE
};

void setup() {

  Serial.begin(115200);

  pinMode(START_PIN, INPUT);

  Serial2.setTX(UART_TX_PIN);
  Serial2.setRX(UART_RX_PIN);

  Serial2.begin(115200, SERIAL_8E1);

  // wait till start pin recieves 3.3v from Wheelbase
  while (digitalRead(START_PIN) == LOW) {
    delay(1);
  }
  Serial.println("Wheelbase Start");

  delay(650);  //650 ms delay, to start up wheelbase

  Serial.println("init seq");
  Serial2.write(initSeq, sizeof(initSeq));
  Serial2.flush();
  delayMicroseconds(100);  // ~1 Byte delay
  Serial2.write(initSeq, sizeof(initSeq));
}

void loop() {
  if (Serial2.available()) {
    uint8_t firstByte = Serial2.read();

    if (firstByte == 0xD0 || firstByte == 0xD1 || firstByte == 0xD2 || firstByte == 0xD3) {
      uint8_t buffer[3];
      uint8_t count = 0;

      unsigned long startTime = micros();

      // timeout for recieving additional 3 bytes
      while ((micros() - startTime) < 1000 && count < 3) {
        if (Serial2.available()) {
          buffer[count++] = Serial2.read();
        }
      }

      // send reply
      uint8_t response[4] = { 0xF0, 0x00, 0x00, 0x00 };
      Serial2.write(response, 4);
      Serial.println("Keep alive");
    }
  }

  // periodic Button messages (const)
  unsigned long now = millis();
  if (now - lastPeriodicSend >= PERIOD_MS) {
    lastPeriodicSend = now;

    uint8_t msgB0[4] = { 0xB0, 0x02, 0x04, 0x08 };
    uint8_t msgB1[4] = { 0xB1, 0x00, 0x00, 0x00 };

    Serial2.write(msgB0, 4);
    Serial2.flush();
    delayMicroseconds(500);  // 0,5 ms
    Serial2.write(msgB1, 4);
    Serial.println("Button message");
  }
}
