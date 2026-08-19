#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED =================

const int oledWidth = 128;
const int oledHeight = 64;
const int oledSDA = 21;
const int oledSCL = 22;
const int oledReset = -1;

Adafruit_SSD1306 display(
  oledWidth,
  oledHeight,
  &Wire,
  oledReset
);


// ================= JOYSTICK =================

const int pinX = 34;
const int pinZ = 35;
const int pinSW = 32;


// ================= STEPPER PINS =================

// Left / Right x
const int stepLR = 33;
const int dirLR = 25;

// Front / Back z
const int stepFB = 17;
const int dirFB = 26;

// Up / Down y
const int stepUD = 16;
const int dirUD = 27;


// ================= POTENTIOMETER =================

const int potRead = 4;


// ================= JOYSTICK SETTINGS =================

const int CENTER = 2048;
const int DEADZONE = 200;


// ================= POTENTIOMETER LIMITS =================

const int POT_LOW = 100;
const int POT_HIGH = 3995;


// ================= STEP SPEED =================

const int STEP_DELAY = 10;


// =====================================================
// STEP MOTOR THINGY aura 67 
// =====================================================

void stepMotor(int stepPin, int dirPin, bool direction) {

  // Set direction
  digitalWrite(dirPin, direction);

  // One step
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(STEP_DELAY);

  digitalWrite(stepPin, LOW);
  delayMicroseconds(STEP_DELAY);
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);


  // ================= OLED =================

  Wire.begin(oledSDA, oledSCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED failed!");

    while (true);
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("HELLO!");

  display.setTextSize(1);
  display.setCursor(10, 40);
  display.println("ESP32 READY");

  display.display();


  // ================= INPUTS =================

  pinMode(pinSW, INPUT_PULLUP);


  // ================= STEP PINS =================

  pinMode(stepLR, OUTPUT);
  pinMode(stepFB, OUTPUT);
  pinMode(stepUD, OUTPUT);


  // ================= DIR PINS =================

  pinMode(dirLR, OUTPUT);
  pinMode(dirFB, OUTPUT);
  pinMode(dirUD, OUTPUT);


  // ================= INITIAL STATES =================

  digitalWrite(stepLR, LOW);
  digitalWrite(stepFB, LOW);
  digitalWrite(stepUD, LOW);

  digitalWrite(dirLR, LOW);
  digitalWrite(dirFB, LOW);
  digitalWrite(dirUD, LOW);


  delay(1000);
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ================= READ INPUTS =================

  int valueX = analogRead(pinX);
  int valueZ = analogRead(pinZ);
  int potValue = analogRead(potRead);

  int stateSW = digitalRead(pinSW);


  // ===================================================
  // LEFT / RIGHT
  // ===================================================

  if (valueX > CENTER + DEADZONE) {

    // LEFT
    stepMotor(stepLR, dirLR, HIGH);

  }

  else if (valueX < CENTER - DEADZONE) {

    // RIGHT
    stepMotor(stepLR, dirLR, LOW);

  }


  // ===================================================
  // FRONT / BACK
  // ===================================================

  if (valueZ > CENTER + DEADZONE) {

    // FRONT
    stepMotor(stepFB, dirFB, HIGH);

  }

  else if (valueZ < CENTER - DEADZONE) {

    // BACK
    stepMotor(stepFB, dirFB, LOW);

  }


  // ===================================================
  // UP / DOWN
  // ===================================================

  if (potValue < POT_LOW) {

    // DOWN
    stepMotor(stepUD, dirUD, HIGH);

  }

  else if (potValue > POT_HIGH) {

    // UP
    stepMotor(stepUD, dirUD, LOW);

  }


  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.print("X: ");
  Serial.print(valueX);

  Serial.print(" | Z: ");
  Serial.print(valueZ);

  Serial.print(" | P: ");
  Serial.print(potValue);

  Serial.print(" | Button: ");

  if (stateSW == LOW) {
    Serial.println("PRESSED");
  }

  else {
    Serial.println("RELEASED");
  }


  // ===================================================
  // OLED
  // ===================================================

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);


  display.setCursor(0, 0);
  display.print("X: ");
  display.println(valueX);


  display.setCursor(0, 12);
  display.print("Z: ");
  display.println(valueZ);


  display.setCursor(0, 24);
  display.print("P: ");
  display.println(potValue);


  display.setCursor(0, 36);
  display.print("BTN: ");

  if (stateSW == LOW) {
    display.println("PRESSED");
  }

  else {
    display.println("RELEASED");
  }


  display.setCursor(0, 52);
  display.println("LAPLACE controller");


  display.display();
}
