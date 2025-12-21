#include <Joystick.h>
#include <Rotary.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// D-Pad directions (HAT-Switch values)
#define HAT_CENTER -1       // no direction
#define HAT_UP 0            // up
#define HAT_UP_RIGHT 45     // up-right
#define HAT_RIGHT 90        // right
#define HAT_DOWN_RIGHT 135  // down-right
#define HAT_DOWN 180        // down
#define HAT_DOWN_LEFT 225   // down-left
#define HAT_LEFT 270        // left
#define HAT_UP_LEFT 315     // up-left

// Pins for DPAD
#define BUTTON_UP_PIN 3
#define BUTTON_DOWN_PIN 4
#define BUTTON_LEFT_PIN 5
#define BUTTON_RIGHT_PIN 2

#define NUM_ADDITIONAL_BUTTONS 11
// Shift pins
#define BUTTON_INT_1 22
#define BUTTON_INT_2 10

#define RotaryPinA 27
#define RotaryPinB 28
#define RotaryButton 26

#define NUM_LAYERS 4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SSD1306_I2C_ADDRESS 0x3C
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Rotary rotary = Rotary(RotaryPinB, RotaryPinA);
RPI_PICO_Timer ITimer0(0);

volatile bool resetNeeded = false;          // Flag to indicate if button reset is needed
volatile int currentLayer = 0;              // Start at layer 0

char lastSymbol = '\0';                      // Stores the last displayed symbol ('+', '-', '\0' for none)
bool displayNeedsUpdate = true;             // Flag indicating if the display needs updating

// Pin numbers for additional buttons
const int additionalButtonPins[NUM_ADDITIONAL_BUTTONS] = { 12, 15, 13, 14, 19, 18, 16, 17, 20, 21, 1};
// IDs used by Joystick library
const int additionalButtonIDs[NUM_ADDITIONAL_BUTTONS] = { 3, 4, 6, 5, 7, 8, 10, 9, 12, 11, 13 };

bool lastButtonStates[NUM_ADDITIONAL_BUTTONS] = { false };

// Button mappings for each layer
const int buttonMapping[NUM_LAYERS][2] = {
  { 14, 15 },  // Layer 0: Button 1 and 2
  { 16, 17 },  // Layer 1: Button 3 and 4
  { 18, 19 },  // Layer 2: Button 5 and 6
  { 20, 21 }   // Layer 3: Button 7 and 8
};

// Names displayed on OLED per layer
const char* variableNames[NUM_LAYERS] = { "TC", "ABS", "BB", "ECU" };

void setup() {
  Serial.begin(115200);
  Wire.setSDA(8);
  Wire.setSCL(9);
  Wire.begin();

  // Configure pins as INPUT_PULLUP
  for (int i = 0; i < NUM_ADDITIONAL_BUTTONS; i++) {
    pinMode(additionalButtonPins[i], INPUT);
  }
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_INT_1, INPUT);
  pinMode(BUTTON_INT_2, INPUT);
  pinMode(RotaryButton, INPUT);

  // Attach interrupts for rotary encoder and buttons
  attachInterrupt(RotaryPinB, rotate, CHANGE);
  attachInterrupt(RotaryPinA, rotate, CHANGE);
  attachInterrupt(RotaryButton, changeLayer, FALLING);
  attachInterrupt(BUTTON_INT_1, INT1_Button, CHANGE);
  attachInterrupt(BUTTON_INT_2, INT2_Button, CHANGE);

  Joystick.begin();            // Initialize Joystick
  Joystick.useManualSend(true);

  // Initialize OLED display
  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
	   
  }
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  updateDisplay();             // Show initial layer name
}

void loop() {
  DPAD();  // Handle D-Pad input

  // Process additional buttons
  for (int i = 0; i < NUM_ADDITIONAL_BUTTONS; i++) {
    bool currentState = digitalRead(additionalButtonPins[i]) == LOW;
    if (currentState != lastButtonStates[i]) {
      Joystick.button(additionalButtonIDs[i], currentState);
      lastButtonStates[i] = currentState;
    }
  }

  // Reset buttons if needed
  if (resetNeeded) {
    for (int i = buttonMapping[0][0]; i <= buttonMapping[3][1]; i++) {
      Joystick.button(i, false);
    }
    setSymbol('\0');  // Clear symbol
    resetNeeded = false;
  }

  updateDisplay();  // Refresh OLED if needed

  Joystick.send_now();  // Send joystick state
  delay(10);            // Debounce and reduce CPU usage
}

// Interrupt for Button INT1
void INT1_Button() {
  Joystick.button(1, !digitalRead(BUTTON_INT_1));
}

// Interrupt for Button INT2
void INT2_Button() {
  Joystick.button(2, !digitalRead(BUTTON_INT_2));
}

// Read D-Pad buttons and update joystick HAT switch
void DPAD() {

  int hatState = HAT_CENTER;

								  
  bool upPressed = digitalRead(BUTTON_UP_PIN) == LOW;
  bool downPressed = digitalRead(BUTTON_DOWN_PIN) == LOW;
  bool leftPressed = digitalRead(BUTTON_LEFT_PIN) == LOW;
  bool rightPressed = digitalRead(BUTTON_RIGHT_PIN) == LOW;

  // Determine HAT direction
  if (upPressed && rightPressed) {
    hatState = HAT_UP_RIGHT;
  } else if (upPressed && leftPressed) {
    hatState = HAT_UP_LEFT;
  } else if (downPressed && rightPressed) {
    hatState = HAT_DOWN_RIGHT;
  } else if (downPressed && leftPressed) {
    hatState = HAT_DOWN_LEFT;
  } else if (upPressed) {
    hatState = HAT_UP;
  } else if (downPressed) {
    hatState = HAT_DOWN;
  } else if (leftPressed) {
    hatState = HAT_LEFT;
  } else if (rightPressed) {
    hatState = HAT_RIGHT;
  }

  Joystick.hat(hatState);
}

// Rotary encoder rotation handler
void rotate() {
  unsigned char result = rotary.process();
  if (result == DIR_CW) {
    Serial.println("CW");
    Joystick.button(buttonMapping[currentLayer][0], true);
    setSymbol('-');                                            // Display "-"
    ITimer0.attachInterruptInterval(200 * 1000, resetButton);  // Start 200ms timer
  } else if (result == DIR_CCW) {
    Serial.println("CCW");
    Joystick.button(buttonMapping[currentLayer][1], true);
    setSymbol('+');                                            // Display "+"
    ITimer0.attachInterruptInterval(200 * 1000, resetButton);  // Start 200ms timer
  }
}

// Timer callback to reset button
bool resetButton(repeating_timer* rt) {
  resetNeeded = true;  // Set flag to reset buttons
  return true;
}

// Change the current layer when rotary button is pressed
void changeLayer() {
  static unsigned long lastPress = 0;
  unsigned long currentPress = millis();

  // Debounce: only change layer if last press was more than 200ms ago
  if (currentPress - lastPress > 200) {
    currentLayer = (currentLayer + 1) % NUM_LAYERS;  // Cycle to next layer
    Serial.print("Layer changed to: ");
    Serial.println(currentLayer);
    displayNeedsUpdate = true;  // Update OLED
  }
  lastPress = currentPress;
}

// Update OLED display
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
  }

  display.display();
  displayNeedsUpdate = false;
}

// Set symbol to display ('+', '-', or none)
void setSymbol(char symbol) {
  if (lastSymbol != symbol) {
    lastSymbol = symbol;
    displayNeedsUpdate = true;  // Mark display for update
  }
}
