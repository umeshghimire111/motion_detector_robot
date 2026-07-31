#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

// ================================================================
//  WIFI CONFIGURATION
// ================================================================
// Fallback/Default credentials if Wifi Manager has no saved values
// #define DEFAULT_WIFI_SSID     "samiksha555_2"
// #define DEFAULT_WIFI_PASS     "CLEB4D409C"
#define AP_SSID               "RADAR_BOT_HYBRID"
#define AP_PASS               "12345678"
#define DEFAULT_WIFI_SSID     "UNIX"
#define DEFAULT_WIFI_PASS     "password"


// ================================================================
//  MOTOR PINS (L298N)
// ================================================================
#define ENA  25
#define IN1  26
#define IN2  27
#define ENB  14
#define IN3  32
#define IN4  33

#define PWM_FREQ     5000
#define PWM_RES_BITS 8
#define PWM_CH_A     4
#define PWM_CH_B     5

// ================================================================
//  SERVO
// ================================================================
#define SERVO_PIN   18
#define SERVO_MIN   15
#define SERVO_MAX   165
#define SERVO_STEP  3
#define SERVO_DELAY 25

// ================================================================
//  SENSORS
// ================================================================
#define TRIG1  5
#define ECHO1  4    // Radar Sensor (on servo)

#define TRIG2  19
#define ECHO2  23   // Safety Sensor (fixed front)

#define SAFETY_THRESHOLD_CM  25
#define MAX_RANGE_CM        400
#define PING_INTERVAL_MS      45
#define CMD_TIMEOUT_MS     2000

// ================================================================
//  OLED
// ================================================================
#define OLED_SDA   21
#define OLED_SCL   22
#define SCREEN_W  128
#define SCREEN_H   64
#define OLED_ADDR 0x3C

// ================================================================
//  GLOBAL ENUMS & TYPES
// ================================================================
enum DriveCmd { STOPCMD, FORWARD, BACKWARD, LEFT, RIGHT };

// ================================================================
//  SHARED GLOBAL VARIABLES (extern)
// ================================================================
extern DriveCmd currentCmd;
extern unsigned long lastCmdMillis;
extern int driveSpeed;

extern int servoAngle;
extern bool sweepForward;
extern unsigned long lastServoMove;

extern long radarDist;
extern long frontDist;
extern bool frontBlocked;
extern String latestAlert;

#endif // CONFIG_H
