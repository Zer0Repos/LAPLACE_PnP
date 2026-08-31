#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

//oled pins

constexpr int OLED_WIDTH  = 128;
constexpr int OLED_HEIGHT = 64;
constexpr int OLED_SDA    = 21;
constexpr int OLED_SCL    = 22;
constexpr int OLED_RESET  = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

Adafruit_SSD1306 display(
  OLED_WIDTH,
  OLED_HEIGHT,
  &Wire,
  OLED_RESET
);

//input pin

constexpr int PIN_X   = 34;
constexpr int PIN_Z   = 35;
constexpr int PIN_SW  = 32;
constexpr int PIN_POT = 4;

//claw servo pin

constexpr int SERVO_PIN = 14;

// Adjust claw
constexpr int CLAW_OPEN_ANGLE   = 30;
constexpr int CLAW_CLOSED_ANGLE = 100;

Servo clawServo;

bool clawClosed = false;

// button press
bool lastButtonState = HIGH;

//joystick setting

constexpr int JOYSTICK_CENTER   = 2048;
constexpr int JOYSTICK_DEADZONE = 200;

//pot

constexpr int POT_LOW  = 100;
constexpr int POT_HIGH = 3995;

//stepper pin

struct Stepper {
  int stepPin;
  int dirPin;
};

const Stepper MOTOR_LR = {33, 25};  // Left / Right
const Stepper MOTOR_FB = {17, 26};  // Front / Back
const Stepper MOTOR_UD = {16, 27};  // Up / Down

constexpr int STEP_DELAY_US = 10;

//stepper motor pin

void stepMotor(const Stepper& motor, bool direction)
{
  digitalWrite(motor.dirPin, direction);

  digitalWrite(motor.stepPin, HIGH);
  delayMicroseconds(STEP_DELAY_US);

  digitalWrite(motor.stepPin, LOW);
  delayMicroseconds(STEP_DELAY_US);
}

//initiate motor

void initializeMotor(const Stepper& motor)
{
  pinMode(motor.stepPin, OUTPUT);
  pinMode(motor.dirPin, OUTPUT);

  digitalWrite(motor.stepPin, LOW);
  digitalWrite(motor.dirPin, LOW);
}

//joystick motor control

void controlMotor(
  const Stepper& motor,
  int value,
  int center,
  int deadzone
)
{
  if (value > center + deadzone) {
    stepMotor(motor, true);
  }
  else if (value < center - deadzone) {
    stepMotor(motor, false);
  }
}

//y axis stepper

void controlVerticalMotor(int potValue)
{
  if (potValue < POT_LOW) {
    stepMotor(MOTOR_UD, true);
  }
  else if (potValue > POT_HIGH) {
    stepMotor(MOTOR_UD, false);
  }
}

//claw control

void toggleClaw()
{
  clawClosed = !clawClosed;

  if (clawClosed) {
    clawServo.write(CLAW_CLOSED_ANGLE);
    Serial.println("Claw: CLOSED");
  }
  else {
    clawServo.write(CLAW_OPEN_ANGLE);
    Serial.println("Claw: OPEN");
  }
}

//button control

void handleButton()
{
  bool buttonState = digitalRead(PIN_SW);
  
  if (lastButtonState == HIGH && buttonState == LOW) {
    toggleClaw();
  }

  lastButtonState = buttonState;
}

//oled

void updateDisplay(
  int valueX,
  int valueZ,
  int potValue,
  bool buttonPressed
)
{
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
  display.println(buttonPressed ? "PRESSED" : "RELEASED");

  display.setCursor(0, 48);
  display.print("CLAW: ");
  display.println(clawClosed ? "CLOSED" : "OPEN");

  display.display();
}

//serial monitor

void printSerial(
  int valueX,
  int valueZ,
  int potValue,
  bool buttonPressed
)
{
  Serial.print("X: ");
  Serial.print(valueX);

  Serial.print(" | Z: ");
  Serial.print(valueZ);

  Serial.print(" | P: ");
  Serial.print(potValue);

  Serial.print(" | Button: ");
  Serial.print(buttonPressed ? "PRESSED" : "RELEASED");

  Serial.print(" | Claw: ");
  Serial.println(clawClosed ? "CLOSED" : "OPEN");
}


void setup()
{
  Serial.begin(115200);

 //OLED

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
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

//joystick button

  pinMode(PIN_SW, INPUT_PULLUP);

//steppers

  initializeMotor(MOTOR_LR);
  initializeMotor(MOTOR_FB);
  initializeMotor(MOTOR_UD);

//claw
  clawServo.setPeriodHertz(50);
  clawServo.attach(SERVO_PIN, 500, 2400);

  // Start with claw open
  clawServo.write(CLAW_OPEN_ANGLE);
  clawClosed = false;

  delay(1000);
}


void loop()
{
//read

  const int valueX = analogRead(PIN_X);
  const int valueZ = analogRead(PIN_Z);
  const int potValue = analogRead(PIN_POT);

  const bool buttonPressed = digitalRead(PIN_SW) == LOW;

//control steppers

  controlMotor(
    MOTOR_LR,
    valueX,
    JOYSTICK_CENTER,
    JOYSTICK_DEADZONE
  );

  controlMotor(
    MOTOR_FB,
    valueZ,
    JOYSTICK_CENTER,
    JOYSTICK_DEADZONE
  );

  controlVerticalMotor(potValue);

//claw control

  handleButton();

//output

  printSerial(
    valueX,
    valueZ,
    potValue,
    buttonPressed
  );

  updateDisplay(
    valueX,
    valueZ,
    potValue,
    buttonPressed
  );
//cordinate and position system


  
}
