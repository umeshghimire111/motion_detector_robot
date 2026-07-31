# User Guide & Pin Connection Diagram: Hybrid Smart Radar Bot

This guide provides instructions for assembling, wiring, programming, and operating the modular **ESP32 Hybrid Smart Radar Bot**.

---

## 1. System Pin Diagram & Wiring Schematic

### A. ESP32 Pinout Mapping
The diagram below shows how the ESP32 GPIOs are mapped to the peripheral components:

```
                      +-------------------+
                      |       ESP32       |
                      |                   |
      [L298N IN1] <---| GPIO 26   GPIO 27 |---> [L298N IN2]
      [L298N ENA] <---| GPIO 25   GPIO 14 |---> [L298N ENB] (PWM)
      [L298N IN3] <---| GPIO 32   GPIO 33 |---> [L298N IN4]
                      |                   |
    [SG90 Signal] <---| GPIO 18   GPIO 21 |---> [OLED SDA]
                      |           GPIO 22 |---> [OLED SCL]
                      |                   |
   [Radar TRIG 1] <---| GPIO 5    GPIO 4  |<--- [Radar ECHO 1]* (Via Divider)
  [Safety TRIG 2] <---| GPIO 19   GPIO 23 |<--- [Safety ECHO 2]* (Via Divider)
                      |                   |
               GND <---| GND       5V/VIN |<--- External 5V Input
                      +-------------------+
``` 

> [!WARNING]
> **5V to 3.3V Level Shifting Required!**  
> The HC-SR04 Echo pins output  **5V**, but ESP32 GPIOs are **3.3V only**. You must use a voltage divider (1kΩ and 2kΩ resistors) or a logic level shifter on the Echo lines to prevent damaging the ESP32.

---

### B. Pin Connection Reference Table

| Component | Component Pin | ESP32 GPIO | Description / Signal Type | Power Supply |
| :--- | :--- | :--- | :--- | :--- |
| **L298N Motor Driver** | ENA | **GPIO 25** | Left Motor Speed (PWM Channel 4) | 5V (from driver logic) |
| | IN1 | **GPIO 26** | Left Motor Direction Forward | - |
| | IN2 | **GPIO 27** | Left Motor Direction Backward | - |
| | ENB | **GPIO 14** | Right Motor Speed (PWM Channel 5) | 5V (from driver logic) |
| | IN3 | **GPIO 32** | Right Motor Direction Forward | - |
| | IN4 | **GPIO 33** | Right Motor Direction Backward | - |
| **SG90 Servo (Radar)** | Signal (Orange) | **GPIO 18** | Servo Angle Position (PWM) | 5V / VIN |
| **HC-SR04 #1 (Radar)** | Trig | **GPIO 5** | Radar Ultrasonic Trigger Output | 5V / VIN |
| | Echo | **GPIO 4** | Radar Echo Input (*Requires 1kΩ/2kΩ divider*) | - |
| **HC-SR04 #2 (Safety)**| Trig | **GPIO 19** | Front Ultrasonic Trigger Output | 5V / VIN |
| | Echo | **GPIO 23** | Front Echo Input (*Requires 1kΩ/2kΩ divider*) | - |
| **SSD1306 OLED Screen**| SDA | **GPIO 21** | I2C Data Line | 3.3V |
| | SCL | **GPIO 22** | I2C Clock Line | 3.3V |

---

## 2. Echo Pin Voltage Divider Circuit (Detailed Technical Guide)

### Why is a Voltage Divider Necessary?
The **HC-SR04 Ultrasonic Sensor** operates on a **5V** supply, meaning its trigger and echo logic signals run at **5V level**. 
However, the **ESP32 microcontroller's GPIO pins are not 5V tolerant**—they operate strictly at **3.3V level**. 

If you connect the 5V Echo output pin of the HC-SR04 directly to an ESP32 GPIO input pin, you risk:
1. **MCU Damage:** Over-voltage will degrade or burn out the internal electrostatic discharge (ESD) protection diodes on that GPIO pin, permanently killing it.
2. **System Instability:** Excess voltage can leak into the ESP32’s internal power rail, causing sudden crashes, watchdog resets, or permanent chip destruction.

To solve this, we use a simple **voltage divider** circuit to step down the 5V output of the sensor's Echo pin to a safe 3.3V signal before it reaches the ESP32.

---

### The Mathematical Calculation
A voltage divider splits input voltage ($V_{\text{in}}$) across two resistors ($R_1$ and $R_2$) connected in series. The output voltage ($V_{\text{out}}$) is measured across the second resistor ($R_2$) connected to Ground:

$$V_{\text{out}} = V_{\text{in}} \times \left(\frac{R_2}{R_1 + R_2}\right)$$

For our system:
* **$V_{\text{in}}$ (Echo Pin):** $5.0\text{ V}$
* **$R_1$ (Series Resistor):** $1\text{ k}\Omega$ (1,000 ohms)
* **$R_2$ (Pull-down Resistor):** $2\text{ k}\Omega$ (2,000 ohms)

Plugging these values into the formula yields:

$$V_{\text{out}} = 5\text{ V} \times \left(\frac{2000}{1000 + 2000}\right) = 5\text{ V} \times \frac{2}{3} \approx 3.33\text{ V}$$

The resulting $3.33\text{V}$ matches the ESP32 logic high requirements perfectly while protecting the pins.

---

### Alternative Resistor Values
If you do not have exactly $1\text{ k}\Omega$ and $2\text{ k}\Omega$ resistors, you can substitute other values as long as the ratio remains roughly $R_2 = 2 \times R_1$:

| Resistor 1 ($R_1$ - to Echo) | Resistor 2 ($R_2$ - to GND) | Resulting Output Voltage ($V_{\text{out}}$) | Status |
| :--- | :--- | :--- | :--- |
| **$1\text{ k}\Omega$** | **$2\text{ k}\Omega$** | $3.33\text{ V}$ | **Recommended (Optimal)** |
| $10\text{ k}\Omega$ | $20\text{ k}\Omega$ | $3.33\text{ V}$ | Safe (Slightly slower edge rise time) |
| $4.7\text{ k}\Omega$ | $10\text{ k}\Omega$ | $3.40\text{ V}$ | Safe (Standard values) |
| $2.2\text{ k}\Omega$ | $4.7\text{ k}\Omega$ | $3.41\text{ V}$ | Safe (Standard values) |

> [!IMPORTANT]
> Keep the total resistance ($R_1 + R_2$) between $3\text{ k}\Omega$ and $30\text{ k}\Omega$. If it is too low (e.g. 100 ohms), too much current will flow, heating the resistors and straining the sensor. If it is too high (e.g. 100k ohms), internal pin capacitance will distort the pulse edge shape, resulting in incorrect distance measurements.

---

### Wiring Layout Schematic
You must build this circuit on your breadboard or protoboard for **both** sensors (`ECHO1` on GPIO 4 and `ECHO2` on GPIO 23):

```
                       HC-SR04 ECHO (5V Signal Out)
                                    |
                                    | Jumper Wire
                                    v
                             [ Breadboard Column A ]
                                    |
                              [R1: 1kΩ Resistor]
                                    |
                                    v
                             [ Breadboard Column B ] <== Jumper to ESP32 GPIO (3.3V)
                                    |
                              [R2: 2kΩ Resistor]
                                    |
                                    v
                               Common GND
```

### Step-by-Step Breadboard Instructions
1. Run a jumper wire from the sensor's **ECHO pin** to an empty terminal strip column on the breadboard (e.g., Column A).
2. Insert one leg of your **$1\text{ k}\Omega$ resistor** ($R_1$) into Column A, and the other leg into a different empty column (e.g., Column B).
3. Connect a jumper wire from Column B to the target ESP32 pin (**GPIO 4** for Sensor #1, **GPIO 23** for Sensor #2).
4. Insert one leg of your **$2\text{ k}\Omega$ resistor** ($R_2$) into Column B, and the other leg into the **Ground (GND)** rail of the breadboard.
5. Ensure the ESP32 GND, breadboard GND rail, and sensor GND pins are all connected together to establish a common ground references plane.

---

## 3. Library Dependencies Installation

Before compiling, install the following libraries in your Arduino IDE:

1. **ESPAsyncWebServer**
   * Download the ZIP from [me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) and add it via *Sketch > Include Library > Add .ZIP Library...* (or search `ESPAsyncWebServer` inside PlatformIO/Arduino Library Manager).
2. **AsyncTCP**
   * Download the ZIP from [me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP) and add it.
3. **ESP32Servo (No longer needed!)**
   * The modular codebase drives the SG90 servo natively using direct ESP32 hardware LEDC (PWM) commands. You do **not** need to install this library.
4. **Adafruit SSD1306 & Adafruit GFX Library**
   * Search for **Adafruit SSD1306** in the Library Manager and install it (agree to install all required dependencies like Adafruit GFX).

---

## 4. Operation Instructions

### Step 1: Booting the Robot
1. Power up the ESP32.
2. The OLED screen will initialize and display:
   * **WiFi Status:** Connecting...
   * If it successfully connects to the local WiFi configured in `config.h`, it displays its IP address (e.g., `192.168.1.100`).
   * If it cannot connect within 7.5 seconds, it boots into **Configuration Mode**.

### Step 2: AP Configuration Mode (Captive Portal)
1. If the OLED screen displays `=== WIFI CONFIG ===`, search for WiFi networks on your phone or PC.
2. Connect to the Access Point:
   * **SSID:** `RADAR_BOT_HYBRID`
   * **Password:** `12345678`
3. A login/configuration page should automatically pop up (Captive Portal). If it does not, open a web browser and go to: `http://192.168.4.1/`.
4. Click **Scan Networks** to discover surrounding networks.
5. Enter the WiFi name (SSID) and Password, then click **Save & Connect**.
6. The robot will save these credentials to its Non-Volatile Storage (NVS) and reboot automatically to connect.

### Step 3: Web Driving Console
1. Once connected, open a web browser on any device connected to the same WiFi network.
2. Enter the IP address shown on the OLED screen (e.g., `http://192.168.1.100/`).
3. You will see the Hybrid Radar Bot Control Dashboard:
   * **D-Pad Mode (Default):** Use the arrows to steer and drive.
   * **Joystick Mode:** Click **Use Joystick** in the topbar. Drag the canvas joystick control to steer smoothly with analog differential mixing.
   * **Radar Display:** The dashboard displays a real-time glowing radar sweep showing objects detected by the panning sensor, alongside a static marker for the fixed front safety sensor.
   * **Telemetry Stats:** Displays actual distances, current speed, commands, and communication ping latency in real-time.

### Step 4: Safety Stop
* If the fixed front sensor detects an obstacle within 25cm, the bot automatically cuts power to the motors and sets the state to `STOP`, ignoring further forward commands until the path is cleared.
* If communication disconnects (e.g. you close the browser window or step out of range) while driving, a safety watchdog timer stops the robot after 2 seconds.

---

## 5. Troubleshooting & Diagnostics
mbedtls_md5_starts
If the dashboard at `http://192.168.1.100/` fails to load, or is stuck showing `OFFLINE`/`CONNECTING...`, follow these diagnostic steps:

### A. Network Connection Check
1. **Subnet Isolation:** Ensure your computer or smartphone is connected to the **same** local network as the ESP32. If your device is on a `5GHz` band and the ESP32 is on a `2.4GHz` band, make sure **AP Isolation (Access Point Isolation)** is disabled on your router settings, otherwise they cannot ping each other.
2. **Accessing via HTTP:** Do **not** use HTTPS. Double check the address bar in your browser says `http://192.168.1.100` and has not automatically redirected to `https://192.168.1.100`.

### B. Dashboard Stability Enhancements (Applied in Code)
The modular code contains the following safety designs to keep the connection robust:
* **Direct Flash Streaming (Zero-Copy):** Uses `AsyncWebServerResponse` with `beginResponse_P` to stream the web page in tiny chunks directly from flash storage. This eliminates heap exhaustion crashes.
* **10Hz WebSocket Throttling:** Telemetry updates are capped to 10 frames per second (10Hz). This prevents the ESP32's tiny TCP network buffer queue from overflowing and crashing the connection under heavy traffic (e.g., fast radar sweeps).
* **Heap Buffer Protection:** Incoming WebSocket frame payloads are copied safely using standard string allocations. It prevents internal frame buffer overflows that cause sudden silent resets.

### C. Clearing Browser Cache
If you previously accessed the Captive Portal on your browser at `192.168.4.1` or the robot's local IP, the browser might have cached redirects or socket descriptors. Clear your browser cache or open the dashboard in an **Incognito / Private Window** to force a clean handshake.

