#include "motors.h"

// Define the global motor state variables
DriveCmd currentCmd = STOPCMD;
unsigned long lastCmdMillis = 0;
int driveSpeed = 200;

// ================================================================
//  PWM COMPAT WRAPPER (handles core v2 and v3)
// ================================================================
#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
  // Core v3.x: ledcAttach(pin, freq, bits) — no channel needed
  void initPWM(uint8_t pin, uint32_t freq, uint8_t res, uint8_t ch) {
    ledcAttach(pin, freq, res);
  }
  void writePWM(uint8_t pin, uint8_t ch, uint32_t duty) {
    ledcWrite(pin, duty);
  }
#else
  // Core v2.x: ledcSetup + ledcAttachPin
  void initPWM(uint8_t pin, uint32_t freq, uint8_t res, uint8_t ch) {
    ledcSetup(ch, freq, res);
    ledcAttachPin(pin, ch);
  }
  void writePWM(uint8_t pin, uint8_t ch, uint32_t duty) {
    ledcWrite(ch, duty);
  }
#endif

void motorsBegin() {
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT);
  
  initPWM(ENA, PWM_FREQ, PWM_RES_BITS, PWM_CH_A);
  initPWM(ENB, PWM_FREQ, PWM_RES_BITS, PWM_CH_B);
  
  writePWM(ENA, PWM_CH_A, 0);
  writePWM(ENB, PWM_CH_B, 0);
  
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, LOW);
  
  Serial.println("[MOTORS] Ready");
}

void motorsStop() {
  writePWM(ENA, PWM_CH_A, 0);
  writePWM(ENB, PWM_CH_B, 0);
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, LOW);
}

void setMotor(int pwmPin, int ch, int in1, int in2, int spd, bool fwd) {
  digitalWrite(in1, fwd ? HIGH : LOW);
  digitalWrite(in2, fwd ? LOW  : HIGH);
  writePWM(pwmPin, ch, spd);
}

void applyDriveCmd(DriveCmd cmd) {
  currentCmd    = cmd;
  lastCmdMillis = millis();
  
  // If safety stop is active, block forward movement
  if (cmd == FORWARD && frontBlocked) {
    motorsStop();
    currentCmd = STOPCMD;
    return;
  }
  
  switch (cmd) {
    case FORWARD:
      setMotor(ENA, PWM_CH_A, IN1, IN2, driveSpeed, true);
      setMotor(ENB, PWM_CH_B, IN3, IN4, driveSpeed, true);
      break;
    case BACKWARD:
      setMotor(ENA, PWM_CH_A, IN1, IN2, driveSpeed, false);
      setMotor(ENB, PWM_CH_B, IN3, IN4, driveSpeed, false);
      break;
    case LEFT:
      setMotor(ENA, PWM_CH_A, IN1, IN2, driveSpeed, false);
      setMotor(ENB, PWM_CH_B, IN3, IN4, driveSpeed, true);
      break;
    case RIGHT:
      setMotor(ENA, PWM_CH_A, IN1, IN2, driveSpeed, true);
      setMotor(ENB, PWM_CH_B, IN3, IN4, driveSpeed, false);
      break;
    default:
      motorsStop(); 
      break;
  }
}

void driveJoystick(int x, int y) {
  lastCmdMillis = millis();
  
  // Mix X and Y for differential steering
  // Y: forward/backward (-255 to 255)
  // X: steering (-255 to 255)
  int leftSpeed = y + x;
  int rightSpeed = y - x;
  
  // Safety override for forward movement when front is blocked
  if (frontBlocked && (leftSpeed > 0 || rightSpeed > 0)) {
    motorsStop();
    currentCmd = STOPCMD;
    return;
  }
  
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);
  
  // Map left speed and direction
  bool leftFwd = leftSpeed >= 0;
  int absLeft = abs(leftSpeed);
  // Threshold to avoid motor buzzing at very low speeds
  if (absLeft < 50) absLeft = 0;
  
  // Map right speed and direction
  bool rightFwd = rightSpeed >= 0;
  int absRight = abs(rightSpeed);
  if (absRight < 50) absRight = 0;
  
  setMotor(ENA, PWM_CH_A, IN1, IN2, absLeft, leftFwd);
  setMotor(ENB, PWM_CH_B, IN3, IN4, absRight, rightFwd);
  
  // Set currentCmd to something representative of movement
  if (absLeft == 0 && absRight == 0) currentCmd = STOPCMD;
  else if (leftFwd && rightFwd) currentCmd = FORWARD;
  else if (!leftFwd && !rightFwd) currentCmd = BACKWARD;
  else if (!leftFwd && rightFwd) currentCmd = LEFT;
  else currentCmd = RIGHT;
}
