/*
 * D.A.V.E. - Arm & Gripper Stepper Driver Arduino Firmware
 * --------------------------------------------------------
 * Firmware for Arduino (Uno/Nano/Mega/Teensy/ESP32/STM32) acting as a slave step generator
 * for two DM556Y stepper motor drivers offloading Jetson PWM.
 *
 * Operation Modes Supported:
 * --------------------------
 * 1) GPIO DIRECT TRIGGER MODE (USE_GPIO_TRIGGER = true):
 *    - Jetson drives simple HIGH/LOW digital signals to Arduino inputs:
 *        - Jetson Pin 33 (Arm Trigger)    -> Arduino Pin 8 (HIGH = Move, LOW = Stop)
 *        - Jetson Pin 18 (Arm Dir)        -> Arduino Pin 9 (HIGH = Up, LOW = Down)
 *        - Jetson Pin 32 (Grip Trigger)   -> Arduino Pin 10 (HIGH = Move, LOW = Stop)
 *        - Jetson Pin 13 (Grip Dir)       -> Arduino Pin 11 (HIGH = Open, LOW = Close)
 *    - Arduino generates microsecond step pulses on Pins 2 & 5 at preset frequency (e.g. 50kHz)
 *      whenever the corresponding trigger pin goes HIGH.
 *
 * 2) USB SERIAL MODE (USE_GPIO_TRIGGER = false):
 *    - Commands sent over USB Serial (115200 Baud):
 *        "M <arm_dir> <arm_freq> <grip_dir> <grip_freq>\n"
 *
 * Output Hardware Pins (To DM556Y Stepper Drivers):
 *   - Driver #1 (Arm Stepper Motor):
 *       STEP Pin   : Arduino Pin 2 -> DM556Y PUL+
 *       DIR Pin    : Arduino Pin 3 -> DM556Y DIR+
 *       ENABLE Pin : Arduino Pin 4 -> DM556Y ENA+ (Active LOW)
 *
 *   - Driver #2 (Gripper Stepper Motor):
 *       STEP Pin   : Arduino Pin 5 -> DM556Y PUL+
 *       DIR Pin    : Arduino Pin 6 -> DM556Y DIR+
 *       ENABLE Pin : Arduino Pin 7 -> DM556Y ENA+ (Active LOW)
 */

#include <Arduino.h>

// ==================== MODE CONFIGURATION ====================
// Set to true if reading digital HIGH/LOW trigger pins from Jetson GPIO.
// Set to false if controlling via USB Serial commands.
const bool USE_GPIO_TRIGGER = true;

// Default step frequencies for GPIO trigger mode (Hz)
const unsigned long ARM_TRIGGER_FREQ_HZ     = 50000; // 50 kHz step pulse frequency for Arm
const unsigned long GRIPPER_TRIGGER_FREQ_HZ = 50000; // 50 kHz step pulse frequency for Gripper

// ==================== ARDUINO OUTPUT PINS (To DM556Y) ====================
const int ARM_STEP_PIN    = 2;
const int ARM_DIR_PIN     = 3;
const int ARM_ENABLE_PIN  = 4;

const int GRIPPER_STEP_PIN   = 5;
const int GRIPPER_DIR_PIN    = 6;
const int GRIPPER_ENABLE_PIN = 7;

const int STATUS_LED_PIN     = 13; // Built-in LED heartbeat

// ==================== ARDUINO INPUT PINS (From Jetson GPIO) ====================
const int JETSON_ARM_TRIG_PIN  = 8;  // Connected to Jetson Board Pin 33
const int JETSON_ARM_DIR_PIN   = 9;  // Connected to Jetson Board Pin 18
const int JETSON_GRIP_TRIG_PIN = 10; // Connected to Jetson Board Pin 32
const int JETSON_GRIP_DIR_PIN  = 11; // Connected to Jetson Board Pin 13

// ==================== CONFIGURATION ====================
const unsigned long WATCHDOG_TIMEOUT_MS = 500; // Serial watchdog timeout
const bool ENABLE_ACTIVE_LOW = true;            // DM556Y Enable pin: LOW = Driver Enabled
const bool DIR_INVERT_ARM    = false;
const bool DIR_INVERT_GRIP   = false;

// ==================== MOTOR STATES ====================
struct StepperMotor {
  int stepPin;
  int dirPin;
  int enablePin;
  bool invertDir;
  
  int currentDir;                 // 1, -1, or 0
  unsigned long halfPeriodUs;     // Microseconds per step pulse toggle
  unsigned long lastStepTimeUs;   // Timestamp of last toggle
  bool stepPinState;              // High / Low
  bool enabled;
};

StepperMotor armMotor  = {ARM_STEP_PIN, ARM_DIR_PIN, ARM_ENABLE_PIN, DIR_INVERT_ARM, 0, 0, 0, false, true};
StepperMotor gripMotor = {GRIPPER_STEP_PIN, GRIPPER_DIR_PIN, GRIPPER_ENABLE_PIN, DIR_INVERT_GRIP, 0, 0, 0, false, true};

unsigned long lastSerialMsgTimeMs = 0;
String inputBuffer = "";

// ==================== HELPER FUNCTIONS ====================

void setMotorEnable(StepperMotor &motor, bool enable) {
  motor.enabled = enable;
  if (motor.enablePin >= 0) {
    digitalWrite(motor.enablePin, enable ? (ENABLE_ACTIVE_LOW ? LOW : HIGH) : (ENABLE_ACTIVE_LOW ? HIGH : LOW));
  }
}

void setMotorDirection(StepperMotor &motor, int dir) {
  if (dir == motor.currentDir) return;
  
  motor.currentDir = dir;
  if (dir != 0) {
    bool physicalDir = (dir > 0);
    if (motor.invertDir) physicalDir = !physicalDir;
    digitalWrite(motor.dirPin, physicalDir ? HIGH : LOW);
    delayMicroseconds(5); // Setup time constraint for DM556Y driver
  }
}

void updateMotorSpeed(StepperMotor &motor, int dir, unsigned long freqHz) {
  setMotorDirection(motor, dir);
  if (dir == 0 || freqHz == 0) {
    motor.halfPeriodUs = 0;
    digitalWrite(motor.stepPin, LOW);
    motor.stepPinState = false;
  } else {
    unsigned long freq = min(freqHz, 100000UL);
    motor.halfPeriodUs = max(5UL, 1000000UL / (2UL * freq));
  }
}

void stopAllMotors() {
  updateMotorSpeed(armMotor, 0, 0);
  updateMotorSpeed(gripMotor, 0, 0);
}

void parseCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  lastSerialMsgTimeMs = millis();
  digitalWrite(STATUS_LED_PIN, HIGH);

  if (cmd.startsWith("M") || cmd.startsWith("m")) {
    int firstSpace  = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    int thirdSpace  = cmd.indexOf(' ', secondSpace + 1);
    int fourthSpace = cmd.indexOf(' ', thirdSpace + 1);

    if (firstSpace != -1 && secondSpace != -1 && thirdSpace != -1) {
      int armDir      = cmd.substring(firstSpace + 1, secondSpace).toInt();
      long armFreq    = cmd.substring(secondSpace + 1, thirdSpace).toInt();
      int gripDir     = cmd.substring(thirdSpace + 1, fourthSpace != -1 ? fourthSpace : cmd.length()).toInt();
      long gripFreq   = (fourthSpace != -1) ? cmd.substring(fourthSpace + 1).toInt() : 0;

      updateMotorSpeed(armMotor, armDir, abs(armFreq));
      updateMotorSpeed(gripMotor, gripDir, abs(gripFreq));
    }
  } 
  else if (cmd.equalsIgnoreCase("S") || cmd.equalsIgnoreCase("STOP")) {
    stopAllMotors();
    Serial.println("ACK: STOP");
  }
  else if (cmd.equalsIgnoreCase("PING")) {
    Serial.println("PONG");
  }
}

// Read digital trigger & direction pins from Jetson
void processGpioTriggers() {
  // Read Arm Trigger & Direction
  bool armActive = (digitalRead(JETSON_ARM_TRIG_PIN) == HIGH);
  bool armDirHigh = (digitalRead(JETSON_ARM_DIR_PIN) == HIGH);
  int armDir = armActive ? (armDirHigh ? 1 : -1) : 0;
  updateMotorSpeed(armMotor, armDir, armActive ? ARM_TRIGGER_FREQ_HZ : 0);

  // Read Gripper Trigger & Direction
  bool gripActive = (digitalRead(JETSON_GRIP_TRIG_PIN) == HIGH);
  bool gripDirHigh = (digitalRead(JETSON_GRIP_DIR_PIN) == HIGH);
  int gripDir = gripActive ? (gripDirHigh ? 1 : -1) : 0;
  updateMotorSpeed(gripMotor, gripDir, gripActive ? GRIPPER_TRIGGER_FREQ_HZ : 0);
}

// ==================== SETUP & LOOP ====================

void setup() {
  Serial.begin(115200);
  
  // Output pins to DM556Y drivers
  pinMode(ARM_STEP_PIN, OUTPUT);
  pinMode(ARM_DIR_PIN, OUTPUT);
  if (ARM_ENABLE_PIN >= 0) pinMode(ARM_ENABLE_PIN, OUTPUT);

  pinMode(GRIPPER_STEP_PIN, OUTPUT);
  pinMode(GRIPPER_DIR_PIN, OUTPUT);
  if (GRIPPER_ENABLE_PIN >= 0) pinMode(GRIPPER_ENABLE_PIN, OUTPUT);

  pinMode(STATUS_LED_PIN, OUTPUT);

  // Input pins from Jetson GPIO
  if (USE_GPIO_TRIGGER) {
    pinMode(JETSON_ARM_TRIG_PIN, INPUT);
    pinMode(JETSON_ARM_DIR_PIN, INPUT);
    pinMode(JETSON_GRIP_TRIG_PIN, INPUT);
    pinMode(JETSON_GRIP_DIR_PIN, INPUT);
  }

  // Initialize outputs
  digitalWrite(ARM_STEP_PIN, LOW);
  digitalWrite(ARM_DIR_PIN, HIGH);
  setMotorEnable(armMotor, true);

  digitalWrite(GRIPPER_STEP_PIN, LOW);
  digitalWrite(GRIPPER_DIR_PIN, HIGH);
  setMotorEnable(gripMotor, true);

  stopAllMotors();
  lastSerialMsgTimeMs = millis();
  
  Serial.println("DAVE_STEPPER_ARDUINO_READY");
}

void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  // 1. Mode Processing
  if (USE_GPIO_TRIGGER) {
    processGpioTriggers();
  } else {
    // Process Serial Commands
    while (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (inputBuffer.length() > 0) {
          parseCommand(inputBuffer);
          inputBuffer = "";
        }
      } else {
        if (inputBuffer.length() < 64) {
          inputBuffer += c;
        }
      }
    }

    // Safety Watchdog check in Serial mode
    if (nowMs - lastSerialMsgTimeMs > WATCHDOG_TIMEOUT_MS) {
      if (armMotor.currentDir != 0 || gripMotor.currentDir != 0) {
        stopAllMotors();
      }
    }
  }

  // 2. High-Frequency Step Pulse Generation for Arm Motor
  if (armMotor.enabled && armMotor.currentDir != 0 && armMotor.halfPeriodUs > 0) {
    if (nowUs - armMotor.lastStepTimeUs >= armMotor.halfPeriodUs) {
      armMotor.lastStepTimeUs = nowUs;
      armMotor.stepPinState = !armMotor.stepPinState;
      digitalWrite(ARM_STEP_PIN, armMotor.stepPinState ? HIGH : LOW);
    }
  }

  // 3. High-Frequency Step Pulse Generation for Gripper Motor
  if (gripMotor.enabled && gripMotor.currentDir != 0 && gripMotor.halfPeriodUs > 0) {
    if (nowUs - gripMotor.lastStepTimeUs >= gripMotor.halfPeriodUs) {
      gripMotor.lastStepTimeUs = nowUs;
      gripMotor.stepPinState = !gripMotor.stepPinState;
      digitalWrite(GRIPPER_STEP_PIN, gripMotor.stepPinState ? HIGH : LOW);
    }
  }
}
