#include "sensors.h"
#include "motors.h"
#include "web_server.h"

// Define global sensor/servo state variables
int servoAngle = 90;
bool sweepForward = true;
unsigned long lastServoMove = 0;

long radarDist = -1;
long frontDist = -1;
bool frontBlocked = false;
String latestAlert = "CLEAR";

#define SERVO_PWM_CH  1
#define SERVO_FREQ    50
#define SERVO_RES     12

// Timeout for pulse measurement: 20ms = ~340cm max range
#define PULSE_TIMEOUT_US 20000 

void writeServo(int angle) {
  // Map angle (0-180) to pulse width (500us-2400us)
  long pulseWidth = 500 + (angle * 1900) / 180;
  // Map pulse width to 12-bit duty cycle (0-4095) for 50Hz (20000us period)
  long duty = (pulseWidth * 4096) / 20000;
  
  #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
    ledcWrite(SERVO_PIN, duty);
  #else
    ledcWrite(SERVO_PWM_CH, duty);
  #endif
}

void sensorsBegin() {
  // Config pin modes
  pinMode(TRIG1, OUTPUT); 
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); 
  pinMode(ECHO2, INPUT);
  
  // Servo initialization using standard LEDC to avoid timer frequency conflicts
  #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
    ledcAttach(SERVO_PIN, SERVO_FREQ, SERVO_RES);
  #else
    ledcSetup(SERVO_PWM_CH, SERVO_FREQ, SERVO_RES);
    ledcAttachPin(SERVO_PIN, SERVO_PWM_CH);
  #endif

  writeServo(90);
  servoAngle = 90;
  sweepForward = true;
  lastServoMove = millis();
  
  Serial.println("[SENSORS] Pulse measurement initialized");
}

long measureCM(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // pulseInLong uses the ESP32 native hardware cycle counter, which is highly accurate
  unsigned long dur = pulseInLong(echo, HIGH, (unsigned long)PULSE_TIMEOUT_US);
  if (dur == 0) return -1;
  long cm = (long)(dur * 0.0343 / 2.0);
  return (cm > MAX_RANGE_CM) ? MAX_RANGE_CM : cm;
}

void readSensorsTick() {
  static bool readSensor1 = true;
  
  if (readSensor1) {
    radarDist = measureCM(TRIG1, ECHO1);
  } else {
    frontDist = measureCM(TRIG2, ECHO2);
    frontBlocked = (frontDist > 0 && frontDist < SAFETY_THRESHOLD_CM);
    latestAlert = frontBlocked ? "OBSTACLE" : "CLEAR";
    
    if (frontBlocked && currentCmd == FORWARD) {
      motorsStop();
      currentCmd = STOPCMD;
      Serial.print("[SAFETY] Front blocked at ");
      Serial.print(frontDist); Serial.println("cm. Motors STOPPED.");
    }
  }
  
  // Broadcast values to WebSocket clients
  webServerBroadcastRadar(servoAngle, radarDist, frontDist, latestAlert);
  
  readSensor1 = !readSensor1; // Alternate sensors
}

void servoSweepTick() {
  if (millis() - lastServoMove < SERVO_DELAY) return;
  lastServoMove = millis();
  
  if (sweepForward) {
    servoAngle += SERVO_STEP;
    if (servoAngle >= SERVO_MAX) { 
      servoAngle = SERVO_MAX; 
      sweepForward = false; 
    }
  } else {
    servoAngle -= SERVO_STEP;
    if (servoAngle <= SERVO_MIN) { 
      servoAngle = SERVO_MIN; 
      sweepForward = true; 
    }
  }
  writeServo(servoAngle);
}
