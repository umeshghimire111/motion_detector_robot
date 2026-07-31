#include "wifi_portal.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

// Global instances
static DNSServer dnsServer;
static bool apModeActive = false;

// HTML Content for the Captive Portal page
const char PORTAL_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1'>
  <title>RADAR BOT SETUP</title>
  <link href='https://fonts.googleapis.com/css2?family=Orbitron:wght@700;900&family=Outfit:wght@400;600&display=swap' rel='stylesheet'>
  <style>
    body {
      background: radial-gradient(circle at center, #0a0e27 0%, #000 100%);
      color: #fff;
      font-family: 'Outfit', sans-serif;
      margin: 0; padding: 20px;
      display: flex; justify-content: center; align-items: center; min-height: 100vh;
      box-sizing: border-box;
    }
    .card {
      background: rgba(10, 14, 39, 0.75);
      backdrop-filter: blur(20px);
      border-radius: 20px;
      padding: 30px;
      width: 100%; max-width: 400px;
      border: 1px solid rgba(0, 240, 255, 0.2);
      box-shadow: 0 0 30px rgba(0, 240, 255, 0.15);
      text-align: center;
    }
    h2 {
      font-family: 'Orbitron', sans-serif;
      background: linear-gradient(135deg, #00f0ff, #ff00ff);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      margin-top: 0;
      margin-bottom: 10px;
      font-size: 24px;
      letter-spacing: 1px;
    }
    .subtitle {
      color: rgba(255, 255, 255, 0.6);
      font-size: 13px;
      margin-bottom: 25px;
    }
    .input-group {
      margin-bottom: 20px; text-align: left;
    }
    label {
      display: block; font-size: 11px; color: rgba(0, 240, 255, 0.8);
      margin-bottom: 8px; text-transform: uppercase; letter-spacing: 1px;
      font-weight: 600;
    }
    input {
      width: 100%; padding: 12px 16px; border-radius: 10px;
      background: rgba(0, 10, 30, 0.8);
      border: 1px solid rgba(0, 240, 255, 0.15);
      color: #fff; font-size: 15px; outline: none; box-sizing: border-box;
      transition: all 0.3s;
    }
    input:focus {
      border-color: #00f0ff; box-shadow: 0 0 10px rgba(0, 240, 255, 0.3);
    }
    .btn {
      width: 100%; padding: 14px; border: none; border-radius: 10px;
      background: linear-gradient(90deg, #00f0ff, #0088ff);
      color: #fff; font-family: 'Orbitron', sans-serif; font-weight: 700;
      cursor: pointer; box-shadow: 0 4px 15px rgba(0, 240, 255, 0.2);
      transition: all 0.3s;
      font-size: 14px;
      letter-spacing: 1px;
    }
    .btn:hover {
      transform: translateY(-2px); box-shadow: 0 6px 20px rgba(0, 240, 255, 0.4);
    }
    .btn-scan {
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid rgba(0, 240, 255, 0.15);
      box-shadow: none;
      margin-top: 15px;
    }
    .btn-scan:hover {
      background: rgba(0, 240, 255, 0.1);
      box-shadow: none;
    }
    .wifi-list {
      max-height: 150px; overflow-y: auto; margin-top: 10px;
      background: rgba(0, 5, 15, 0.9); border-radius: 10px;
      border: 1px solid rgba(0, 240, 255, 0.1);
    }
    .wifi-item {
      padding: 12px 15px; border-bottom: 1px solid rgba(255, 255, 255, 0.05);
      cursor: pointer; text-align: left; font-size: 13px; display: flex; justify-content: space-between;
      transition: background 0.2s;
    }
    .wifi-item:hover {
      background: rgba(0, 240, 255, 0.15);
    }
    .wifi-item:last-child {
      border-bottom: none;
    }
  </style>
</head>
<body>
  <div class='card'>
    <h2>RADAR BOT</h2>
    <div class='subtitle'>WIFI SETUP PORTAL</div>
    <form action='/save' method='POST'>
      <div class='input-group'>
        <label>SSID</label>
        <input type='text' name='ssid' id='ssid' placeholder='WiFi Name' required>
      </div>
      <div class='input-group'>
        <label>Password</label>
        <input type='password' name='pass' placeholder='WiFi Password'>
      </div>
      <button class='btn' type='submit'>SAVE & CONNECT</button>
    </form>
    
    <button class='btn btn-scan' onclick='scanNetworks()'>SCAN NETWORKS</button>
    <div id='scanned-networks' style='margin-top:20px; display:none;'>
      <label>Discovered Networks</label>
      <div class='wifi-list' id='wifi-list'></div>
    </div>
  </div>

  <script>
    function scanNetworks() {
      const list = document.getElementById('wifi-list');
      const container = document.getElementById('scanned-networks');
      list.innerHTML = '<div style="padding:15px;text-align:center;color:rgba(0,240,255,0.7);">Scanning...</div>';
      container.style.display = 'block';
      fetch('/scan').then(r => r.json()).then(data => {
        list.innerHTML = '';
        if(data.length === 0) {
          list.innerHTML = '<div style="padding:15px;text-align:center;color:rgba(255,255,255,0.5);">No networks found</div>';
          return;
        }
        data.forEach(net => {
          const item = document.createElement('div');
          item.className = 'wifi-item';
          item.innerHTML = '<span>' + net.ssid + '</span><span style="color:#00f0ff;font-weight:600;">' + net.rssi + ' dBm</span>';
          item.onclick = () => {
            document.getElementById('ssid').value = net.ssid;
          };
          list.appendChild(item);
        });
      }).catch(() => {
        list.innerHTML = '<div style="padding:15px;text-align:center;color:#ff0040;">Scan failed. Try again.</div>';
      });
    }
  </script>
</body>
</html>
)HTML";

void wifiBegin() {
  Preferences prefs;
  prefs.begin("wifi-config", true); // read-only mode
  String ssid = prefs.getString("ssid", DEFAULT_WIFI_SSID);
  String pass = prefs.getString("pass", DEFAULT_WIFI_PASS);
  prefs.end();

  Serial.print("[WIFI] Connecting to SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  int tries = 0;
  // Wait for 7.5 seconds max (15 * 500ms)
  while (WiFi.status() != WL_CONNECTED && tries < 15) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WIFI] Connected successfully! IP: ");
    Serial.println(WiFi.localIP());
    apModeActive = false;
  } else {
    Serial.println("[WIFI] Connection failed. Starting Captive Portal Access Point...");
    WiFi.mode(WIFI_AP);
    
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);
    
    dnsServer.start(53, "*", apIP);
    apModeActive = true;
    
    Serial.print("[WIFI] SoftAP active. Connect to: ");
    Serial.println(AP_SSID);
    Serial.print("[WIFI] Setup URL: http://");
    Serial.println(WiFi.softAPIP());
  }
}

bool isAPMode() {
  return apModeActive;
}

void handleDNSTick() {
  if (apModeActive) {
    dnsServer.processNextRequest();
  }
}
