/*
  ================================================================
  HYBRID SMART RADAR BOT - Modular High-Performance Version
  ================================================================
  ENHANCEMENTS IN THIS VERSION:
  1. Modularized Architecture: Config, Motors, Sensors, Web Server, WiFi Portal.
  2. Non-blocking interrupt-driven ultrasonic scanning (eliminates CPU bottlenecks).
  3. ESPAsyncWebServer & WebSockets for low-latency (<10ms) telemetry & control.
  4. Smooth Touch Joystick (differential mixing) and classic D-pad toggling.
  5. Captive Portal WiFi Manager with automated fallback to Config AP.
  6. Smarter OLED indicators displaying AP details during configuration.
  ================================================================
*/

#include "config.h"
#include "motors.h"
#include "sensors.h"
#include "web_server.h"
#include "wifi_portal.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display object
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

static unsigned long lastOLEDUpdate = 0;

// ================================================================
//  OLED DISPLAY METHODS
// ================================================================
void oledBegin() {
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Display SSD1306 not found!");
    return;
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RADAR BOT INITIALIZING");
  display.display();
  Serial.println("[OLED] Display Initialized");
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  if (isAPMode()) {
    display.println("=== WIFI CONFIG ===");
    display.println();
    display.print("SSID: "); 
    display.println(AP_SSID);
    display.print("IP:   "); 
    display.println("192.168.4.1");
    display.println();
    display.println("Connect to AP to setup");
  } else {
    display.print("WiFi:  ");
    display.println(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED");
    display.print("Servo: "); 
    display.print(servoAngle); 
    display.println(" deg");
    display.print("Radar: "); 
    display.print(radarDist >= 0 ? String(radarDist) : "---");  
    display.println(" cm");
    display.print("Front: "); 
    display.print(frontDist >= 0 ? String(frontDist) : "---");  
    display.println(" cm");
    
    const char* cmdNames[] = {"STOP","FORWARD","BACKWARD","LEFT","RIGHT"};
    display.print("CMD:   "); 
    display.println(cmdNames[(int)currentCmd]);
    
    if (frontBlocked) {
      display.println();
      display.println("!! FRONT BLOCKED !!");
    }
  }
  display.display();
}

// ================================================================
//  SETUP
// ================================================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== HYBRID SMART RADAR BOT ===");

  // Initialize L298N Motors
  motorsBegin();

  // Initialize Sensors & Servo (Interrupts and Pin modes)
  sensorsBegin();

  // Initialize OLED
  oledBegin();

  // Start WiFi (STA with saved credentials or Fallback to AP Mode)
  wifiBegin();

  // Display initial connection status on OLED
  updateOLED();

  // Start Async Web Server & WebSockets
  webServerBegin();

  Serial.println("=== INITIALIZATION COMPLETE ===");
}

// ================================================================
//  LOOP
// ================================================================
void loop() {
  // 1. Sweeps the servo-mounted radar sensor (non-blocking time-gated)
  servoSweepTick();

  // 2. Read sensors every 50ms (alternates between sensor 1 and 2 each call)
  static unsigned long lastSensorTick = 0;
  if (millis() - lastSensorTick >= 50) {
    lastSensorTick = millis();
    readSensorsTick();
  }

  // 3. Handles Captive Portal redirects (DNS processing) if running in AP Mode
  handleDNSTick();

  // 4. Periodically updates OLED display (every 300ms)
  if (millis() - lastOLEDUpdate >= 300) {
    lastOLEDUpdate = millis();
    updateOLED();
  }

  // 5. Cleanup closed WebSocket client connection instances
  if (!isAPMode()) {
    ws.cleanupClients();
  }

  // 6. Safety Stop Check: Stops motors if client goes silent / disconnects during drive
  if (currentCmd != STOPCMD && millis() - lastCmdMillis > CMD_TIMEOUT_MS) {
    motorsStop();
    currentCmd = STOPCMD;
    Serial.println("[SAFETY] Command timed out. Stopped motors.");
  }
} 