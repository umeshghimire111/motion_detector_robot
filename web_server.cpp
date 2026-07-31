#include "web_server.h"
#include "motors.h"
#include "wifi_portal.h"
#include <Preferences.h>

// Define global AsyncWebServer and AsyncWebSocket instances on Port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Main Robot Web UI Page
const char INDEX_HTML[] PROGMEM = R"WEBPAGE(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>HYBRID RADAR BOT</title>
<link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@700;900&family=Outfit:wght@400;600&display=swap" rel="stylesheet">
<style>
*{margin:0;padding:0;box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{min-height:100vh;background:radial-gradient(ellipse at center,#0a0e27 0%,#000 100%);font-family:'Outfit',sans-serif;padding:12px;display:flex;justify-content:center;align-items:center}
.container{max-width:440px;width:100%}
.glow-border{position:relative;background:rgba(10,14,39,0.75);backdrop-filter:blur(20px);border-radius:24px;padding:3px;margin-bottom:12px;box-shadow:0 0 30px rgba(0,240,255,0.1)}
.glow-border::before{content:'';position:absolute;top:-2px;left:-2px;right:-2px;bottom:-2px;border-radius:26px;background:linear-gradient(45deg,#00f0ff,#ff00ff);background-size:300% 300%;animation:borderGlow 4s ease-in-out infinite;z-index:-1;opacity:0.4}
@keyframes borderGlow{0%,100%{background-position:0% 50%}50%{background-position:100% 50%}}
.topbar{display:flex;align-items:center;justify-content:space-between;padding:14px 18px;background:rgba(10,14,39,0.6);backdrop-filter:blur(10px);border-radius:21px;border:1px solid rgba(0,240,255,0.15)}
.logo{display:flex;align-items:center;gap:10px;font-family:'Orbitron',monospace;font-weight:900;font-size:14px;background:linear-gradient(135deg,#00f0ff,#0088ff);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.sdot{width:10px;height:10px;border-radius:50%;background:#ff0040;box-shadow:0 0 20px rgba(255,0,64,0.5);transition:all .3s;animation:pred 2s infinite}
.sdot.on{background:#00ff88;box-shadow:0 0 20px rgba(0,255,136,0.6);animation:pgrn 1.5s infinite}
@keyframes pred{0%,100%{opacity:.5;transform:scale(.9)}50%{opacity:1;transform:scale(1.1)}}
@keyframes pgrn{0%,100%{opacity:.7;transform:scale(.9)}50%{opacity:1;transform:scale(1.2)}}
.badge{font-family:'Orbitron',monospace;font-size:9px;padding:4px 10px;border-radius:20px;background:rgba(255,0,64,0.2);color:#ff0040;border:1px solid rgba(255,0,64,0.3);letter-spacing:1px}
.badge.on{background:rgba(0,255,136,0.15);color:#00ff88;border-color:rgba(0,255,136,0.3)}
.btn-toggle{font-family:'Orbitron',monospace;font-size:9px;padding:4px 10px;border-radius:20px;background:rgba(0,240,255,0.1);color:#00f0ff;border:1px solid rgba(0,240,255,0.3);cursor:pointer;transition:all 0.2s}
.btn-toggle:hover{background:rgba(0,240,255,0.25);transform:scale(1.05)}
.alert-bar{display:none;margin-bottom:12px;padding:12px 16px;background:rgba(255,0,64,0.15);border:1px solid rgba(255,0,64,0.3);border-radius:16px;align-items:center;gap:12px;font-family:'Orbitron',monospace;font-size:11px;color:#ff0040}
.alert-bar.show{display:flex}
.radar-card{padding:16px;background:rgba(0,10,30,0.8);border-radius:18px;border:1px solid rgba(0,240,255,0.1)}
canvas{display:block;width:100%;height:auto}
.radar-footer{display:flex;justify-content:space-between;margin-top:10px;padding:8px 4px}
.rfi{font-family:'Orbitron',monospace;font-size:10px;color:rgba(0,240,255,0.6)}
.rfi span{color:#00f0ff;font-weight:700;font-size:12px}
.stats-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-bottom:12px}
.stat-card{background:rgba(0,10,30,0.6);padding:12px 8px;border-radius:14px;border:1px solid rgba(0,240,255,0.08);text-align:center}
.stat-label{font-family:'Orbitron',monospace;font-size:8px;color:rgba(0,240,255,0.4)}
.stat-value{font-family:'Orbitron',monospace;font-size:16px;font-weight:700;background:linear-gradient(135deg,#00f0ff,#0088ff);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.controls{padding:16px;background:rgba(0,10,30,0.6);border-radius:18px;border:1px solid rgba(0,240,255,0.08)}
.dpad{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;max-width:240px;margin:0 auto 16px}
.dpad-btn{aspect-ratio:1;background:rgba(0,20,60,0.5);border:1px solid rgba(0,240,255,0.15);border-radius:14px;display:flex;align-items:center;justify-content:center;font-size:24px;color:rgba(0,240,255,0.6);cursor:pointer;user-select:none;touch-action:none}
.dpad-btn:active,.dpad-btn.active{transform:scale(.92);border-color:#00f0ff;background:rgba(0,240,255,0.2)}
.dpad-btn.stop{background:rgba(255,0,64,0.1);border-color:rgba(255,0,64,0.2);color:#ff0040;font-size:12px}
.dpad-btn.empty{background:transparent;border-color:transparent;pointer-events:none}
.joystick-panel{display:none;flex-direction:column;align-items:center;margin-bottom:16px}
.speed-control{display:flex;align-items:center;gap:12px;padding-top:14px;border-top:1px solid rgba(0,240,255,0.08)}
.speed-slider{flex:1;-webkit-appearance:none;height:4px;border-radius:2px;background:linear-gradient(90deg,#00f0ff,#ff00ff)}
.speed-slider::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;border-radius:50%;background:radial-gradient(circle,#00f0ff,#0088ff);cursor:pointer}
.speed-value{font-family:'Orbitron',monospace;font-size:14px;font-weight:700;color:#00f0ff}
.safe-bar{text-align:center;padding:8px;font-size:9px;letter-spacing:2px;border-top:1px solid rgba(0,240,255,0.08);margin-top:12px;color:#00ff88}
.safe-bar.danger{color:#ff0040}
</style>
</head>
<body>
<div class="container">
<div class="glow-border">
  <div class="topbar">
    <div class="logo"><div class="sdot" id="statusDot"></div>HYBRID RADAR</div>
    <div class="btn-toggle" id="btnMode" onclick="toggleDriveMode()">USE JOYSTICK</div>
    <div class="badge" id="statusBadge">OFFLINE</div>
  </div>
</div>
<div class="alert-bar" id="alertBar"><span>&#9888;</span><span id="alertText">Obstacle detected!</span></div>
<div class="glow-border">
  <div class="radar-card">
    <canvas id="radarCanvas" width="400" height="220"></canvas>
    <div class="radar-footer">
      <div class="rfi">Angle <span id="rfAngle">---</span></div>
      <div class="rfi">Radar <span id="rfDist">---</span></div>
      <div class="rfi">Front <span id="rfFront">---</span></div>
      <div class="rfi">Ping <span id="rfPing">--</span>ms</div>
    </div>
  </div>
</div>
<div class="stats-grid">
  <div class="stat-card"><div class="stat-label">Radar</div><div class="stat-value" id="statDist">--</div></div>
  <div class="stat-card"><div class="stat-label">Front</div><div class="stat-value" id="statFront">--</div></div>
  <div class="stat-card"><div class="stat-label">Speed</div><div class="stat-value" id="statSpeed">200</div></div>
  <div class="stat-card"><div class="stat-label">CMD</div><div class="stat-value" id="statCmd" style="font-size:12px">STOP</div></div>
</div>
<div class="glow-border">
  <div class="controls">
    <!-- D-Pad Interface -->
    <div class="dpad" id="dpadContainer">
      <div class="dpad-btn empty"></div>
      <div class="dpad-btn" id="btnForward" ontouchstart="startCmd('forward')" ontouchend="stopCmd()" onmousedown="startCmd('forward')" onmouseup="stopCmd()">&#9650;</div>
      <div class="dpad-btn empty"></div>
      <div class="dpad-btn" id="btnLeft" ontouchstart="startCmd('left')" ontouchend="stopCmd()" onmousedown="startCmd('left')" onmouseup="stopCmd()">&#9664;</div>
      <div class="dpad-btn stop" id="btnStop" onclick="stopCmd()">&#9632;</div>
      <div class="dpad-btn" id="btnRight" ontouchstart="startCmd('right')" ontouchend="stopCmd()" onmousedown="startCmd('right')" onmouseup="stopCmd()">&#9654;</div>
      <div class="dpad-btn empty"></div>
      <div class="dpad-btn" id="btnBackward" ontouchstart="startCmd('backward')" ontouchend="stopCmd()" onmousedown="startCmd('backward')" onmouseup="stopCmd()">&#9660;</div>
      <div class="dpad-btn empty"></div>
    </div>
    
    <!-- Virtual Joystick Interface -->
    <div class="joystick-panel" id="joystickContainer">
      <canvas id="joyCanvas" width="180" height="180" style="margin: 0 auto; touch-action: none;"></canvas>
    </div>
    
    <div class="speed-control">
      <span style="font-size:9px;color:rgba(0,240,255,0.5);font-family:'Orbitron'">SPEED</span>
      <input type="range" class="speed-slider" id="speedSlider" min="80" max="255" value="200" step="1" oninput="updateSpeed(this.value)">
      <span class="speed-value" id="speedDisplay">200</span>
    </div>
    <div class="safe-bar" id="safeBar">PATH CLEAR</div>
  </div>
</div>
</div>

<script>
const canvas=document.getElementById('radarCanvas');
const ctx=canvas.getContext('2d');
const W=canvas.width,H=canvas.height,cx=W/2,cy=H-20,R=H-40;
let dots=[];

// Drive Mode toggles
let driveMode = 'dpad'; // 'dpad' or 'joystick'
function toggleDriveMode() {
  const dpad = document.getElementById('dpadContainer');
  const joystick = document.getElementById('joystickContainer');
  const btn = document.getElementById('btnMode');
  if (driveMode === 'dpad') {
    driveMode = 'joystick';
    dpad.style.display = 'none';
    joystick.style.display = 'flex';
    btn.textContent = 'USE D-PAD';
    drawJoystick(jcx, jcy);
  } else {
    driveMode = 'dpad';
    dpad.style.display = 'grid';
    joystick.style.display = 'none';
    btn.textContent = 'USE JOYSTICK';
    stopCmd();
  }
}

// Draw radar display
function drawRadar(angle,dist,fDist){
  ctx.fillStyle='rgba(0,5,15,0.85)';ctx.fillRect(0,0,W,H);
  for(let i=1;i<=4;i++){
    ctx.beginPath();ctx.arc(cx,cy,R/4*i,0,Math.PI*2);
    ctx.strokeStyle=`rgba(0,240,255,${0.05+i*0.02})`;ctx.lineWidth=.5;ctx.stroke();
  }
  const ar=angle*Math.PI/180;
  const g=ctx.createLinearGradient(cx,cy,cx-R*Math.cos(ar),cy-R*Math.sin(ar));
  g.addColorStop(0,'rgba(0,240,255,0)');g.addColorStop(1,'rgba(0,240,255,0.12)');
  ctx.beginPath();ctx.moveTo(cx,cy);ctx.arc(cx,cy,R,Math.PI-ar-0.1,Math.PI-ar+0.1);
  ctx.closePath();ctx.fillStyle=g;ctx.fill();
  ctx.shadowColor='#00f0ff';ctx.shadowBlur=18;
  ctx.strokeStyle='#00f0ff';ctx.lineWidth=2;
  ctx.beginPath();ctx.moveTo(cx,cy);
  ctx.lineTo(cx-R*Math.cos(ar),cy-R*Math.sin(ar));
  ctx.stroke();ctx.shadowBlur=0;
  if(dist>0&&dist<400){
    const rr=dist/400*R;
    const px=cx-rr*Math.cos(ar),py=cy-rr*Math.sin(ar);
    dots.push({x:px,y:py,a:0});
    ctx.shadowColor='#00f0ff';ctx.shadowBlur=20;
    ctx.fillStyle='#00f0ff';ctx.beginPath();ctx.arc(px,py,5,0,2*Math.PI);ctx.fill();
    ctx.shadowBlur=0;
    ctx.fillStyle='rgba(0,240,255,0.8)';ctx.font='9px monospace';
    ctx.fillText(dist+'cm',px+7,py-6);
  }
  if(fDist>0&&fDist<400){
    const rr2=fDist/400*R;
    ctx.shadowColor='#ff0040';ctx.shadowBlur=20;
    ctx.fillStyle='#ff0040';ctx.beginPath();ctx.arc(cx,cy-rr2,6,0,2*Math.PI);ctx.fill();
    ctx.shadowBlur=0;
    ctx.fillStyle='rgba(255,0,64,0.9)';ctx.font='9px monospace';
    ctx.fillText('F:'+fDist,cx+8,cy-rr2-6);
  }
  dots=dots.filter(d=>d.a<16);
  dots.forEach(d=>{
    d.a++;
    ctx.fillStyle=`rgba(0,240,255,${(1-d.a/16)*0.5})`;
    ctx.beginPath();ctx.arc(d.x,d.y,3,0,2*Math.PI);ctx.fill();
  });
}

// Websocket logic
let ws;
let wsActive = false;
let httpPollInterval;
let t0 = 0;

function connectWS() {
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const url = protocol + '//' + window.location.host + '/ws';
  ws = new WebSocket(url);
  
  ws.onopen = () => {
    wsActive = true;
    clearInterval(httpPollInterval);
    document.getElementById('statusDot').className = 'sdot on';
    document.getElementById('statusBadge').className = 'badge on';
    document.getElementById('statusBadge').textContent = 'ONLINE (WS)';
    console.log("[WS] Connected");
  };
  
  ws.onmessage = (event) => {
    t0 = Date.now();
    try {
      const data = JSON.parse(event.data);
      handleTelemetry(data);
    } catch(e) {}
  };
  
  ws.onclose = () => {
    wsActive = false;
    document.getElementById('statusDot').className = 'sdot';
    document.getElementById('statusBadge').className = 'badge';
    document.getElementById('statusBadge').textContent = 'CONNECTING...';
    console.log("[WS] Disconnected, falling back to HTTP...");
    
    // Start HTTP polling fallback if WS disconnects
    if (!httpPollInterval) {
      httpPollInterval = setInterval(pollRadarHTTP, 200);
    }
    setTimeout(connectWS, 3000); // retry connect WS in 3s
  };
}

function handleTelemetry(data) {
  const ms = t0 > 0 ? Date.now() - t0 : 1;
  const d = parseInt(data.dist) || 0;
  const f = parseInt(data.frontDist) || 0;
  document.getElementById('rfAngle').textContent = Math.round(data.angle) + 'deg';
  document.getElementById('rfDist').textContent = (d > 0 ? d : '---') + 'cm';
  document.getElementById('rfFront').textContent = (f > 0 ? f : '---') + 'cm';
  document.getElementById('rfPing').textContent = ms;
  document.getElementById('statDist').textContent = (d > 0 ? d : '--');
  document.getElementById('statFront').textContent = (f > 0 ? f : '--');
  drawRadar(data.angle, d, f);
  
  const sb = document.getElementById('safeBar');
  if (data.alert === 'OBSTACLE') {
    document.getElementById('alertText').textContent = f > 0 && f < 25 ? 'FRONT BLOCKED: ' + f + 'cm -- STOPPED' : 'Obstacle on radar';
    document.getElementById('alertBar').className = 'alert-bar show';
    setTimeout(() => document.getElementById('alertBar').className = 'alert-bar', 3000);
    sb.className = 'safe-bar danger';
    sb.textContent = 'OBSTACLE DETECTED';
  } else {
    sb.className = 'safe-bar';
    sb.textContent = 'PATH CLEAR';
  }
}

// Fallback HTTP Polling
function pollRadarHTTP() {
  if (wsActive) return;
  const tStart = Date.now();
  fetch('/radar').then(r => r.json()).then(data => {
    t0 = tStart;
    document.getElementById('statusDot').className = 'sdot on';
    document.getElementById('statusBadge').className = 'badge on';
    document.getElementById('statusBadge').textContent = 'ONLINE (HTTP)';
    handleTelemetry(data);
  }).catch(() => {
    document.getElementById('statusDot').className = 'sdot';
    document.getElementById('statusBadge').className = 'badge';
    document.getElementById('statusBadge').textContent = 'OFFLINE';
  });
}

// Sending Commands
function startCmd(cmd){
  document.querySelectorAll('.dpad-btn').forEach(b=>b.classList.remove('active'));
  const m={forward:'btnForward',backward:'btnBackward',left:'btnLeft',right:'btnRight'};
  if(m[cmd])document.getElementById(m[cmd]).classList.add('active');
  document.getElementById('statCmd').textContent=cmd.toUpperCase();
  
  if (wsActive && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({type: 'cmd', val: cmd}));
  } else {
    fetch('/cmd?dir='+cmd).catch(()=>{});
  }
}

function stopCmd(){
  document.querySelectorAll('.dpad-btn').forEach(b=>b.classList.remove('active'));
  document.getElementById('statCmd').textContent='STOP';
  
  if (wsActive && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({type: 'cmd', val: 'stop'}));
  } else {
    fetch('/cmd?dir=stop').catch(()=>{});
  }
}

function updateSpeed(val){
  document.getElementById('speedDisplay').textContent=val;
  document.getElementById('statSpeed').textContent=val;
  
  if (wsActive && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({type: 'speed', val: parseInt(val)}));
  } else {
    fetch('/speed?val='+val).catch(()=>{});
  }
}

// Canvas Joystick Control
const joyCanvas = document.getElementById('joyCanvas');
const jctx = joyCanvas.getContext('2d');
const jW = joyCanvas.width, jH = joyCanvas.height;
const jcx = jW/2, jcy = jH/2;
const maxR = 50; // boundary travel radius
let joyX = jcx, joyY = jcy;
let isDrawingJoy = false;

function drawJoystick(x, y) {
  jctx.clearRect(0,0,jW,jH);
  
  // draw outer ring
  jctx.beginPath();
  jctx.arc(jcx, jcy, maxR, 0, Math.PI*2);
  jctx.strokeStyle = 'rgba(0, 240, 255, 0.2)';
  jctx.lineWidth = 4;
  jctx.stroke();
  
  // draw center guidelines
  jctx.beginPath();
  jctx.moveTo(jcx - 10, jcy); jctx.lineTo(jcx + 10, jcy);
  jctx.moveTo(jcx, jcy - 10); jctx.lineTo(jcx, jcy + 10);
  jctx.strokeStyle = 'rgba(0, 240, 255, 0.3)';
  jctx.lineWidth = 1;
  jctx.stroke();
  
  // draw inner handle knob
  jctx.beginPath();
  jctx.arc(x, y, 22, 0, Math.PI*2);
  const grad = jctx.createRadialGradient(x, y, 4, x, y, 22);
  grad.addColorStop(0, '#00f0ff');
  grad.addColorStop(1, '#0055aa');
  jctx.fillStyle = grad;
  jctx.shadowColor = '#00f0ff';
  jctx.shadowBlur = 12;
  jctx.fill();
  jctx.shadowBlur = 0;
}

let lastJoySend = 0;
function sendJoystickData(x, y) {
  const now = Date.now();
  if (now - lastJoySend < 60 || (x === 0 && y === 0)) { // throttle to ~16Hz
    lastJoySend = now;
    
    // Set representation in CMD stat
    const absX = Math.abs(x), absY = Math.abs(y);
    if (absX < 30 && absY < 30) document.getElementById('statCmd').textContent = 'STOP';
    else if (absY >= absX) document.getElementById('statCmd').textContent = y > 0 ? 'JOY:FWD' : 'JOY:BACK';
    else document.getElementById('statCmd').textContent = x > 0 ? 'JOY:RGHT' : 'JOY:LEFT';

    if (wsActive && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({type: 'joy', x: x, y: y}));
    }
  }
}

function handleJoyMove(e) {
  if(!isDrawingJoy) return;
  const rect = joyCanvas.getBoundingClientRect();
  const touch = e.touches ? e.touches[0] : e;
  let tx = touch.clientX - rect.left - jcx;
  let ty = touch.clientY - rect.top - jcy;
  
  const dist = Math.sqrt(tx*tx + ty*ty);
  if (dist > maxR) {
    tx = (tx / dist) * maxR;
    ty = (ty / dist) * maxR;
  }
  
  joyX = jcx + tx;
  joyY = jcy + ty;
  drawJoystick(joyX, joyY);
  
  // Map values normalized from -255 to 255
  const normX = Math.round((tx / maxR) * 255);
  const normY = Math.round(-(ty / maxR) * 255); // positive is forward
  sendJoystickData(normX, normY);
}

function handleJoyEnd() {
  if (!isDrawingJoy) return;
  isDrawingJoy = false;
  joyX = jcx;
  joyY = jcy;
  drawJoystick(joyX, joyY);
  sendJoystickData(0, 0);
}

joyCanvas.addEventListener('mousedown', (e) => { isDrawingJoy = true; handleJoyMove(e); });
window.addEventListener('mousemove', handleJoyMove);
window.addEventListener('mouseup', handleJoyEnd);

joyCanvas.addEventListener('touchstart', (e) => { isDrawingJoy = true; handleJoyMove(e); }, {passive:false});
joyCanvas.addEventListener('touchmove', (e) => { e.preventDefault(); handleJoyMove(e); }, {passive:false});
joyCanvas.addEventListener('touchend', handleJoyEnd);

// Startups
drawRadar(0,-1,-1);
connectWS();
</script>
</body>
</html>
)WEBPAGE";

// WebSocket event callback
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      // Safe conversion to String without modifying the underlying packet buffer (preventing heap corruption)
      std::string tempStr((const char*)data, len);
      String msg = tempStr.c_str();
      
      // Fast manual string parsing for speed/size optimization
      if (msg.indexOf("\"type\":\"cmd\"") >= 0) {
        int valStart = msg.indexOf("\"val\":\"") + 7;
        int valEnd = msg.indexOf("\"", valStart);
        String val = msg.substring(valStart, valEnd);
        
        if      (val == "forward")  applyDriveCmd(FORWARD);
        else if (val == "backward") applyDriveCmd(BACKWARD);
        else if (val == "left")     applyDriveCmd(LEFT);
        else if (val == "right")    applyDriveCmd(RIGHT);
        else                        applyDriveCmd(STOPCMD);
      }
      else if (msg.indexOf("\"type\":\"speed\"") >= 0) {
        int valStart = msg.indexOf("\"val\":") + 6;
        int valEnd = msg.indexOf("}", valStart);
        driveSpeed = constrain(msg.substring(valStart, valEnd).toInt(), 80, 255);
      }
      else if (msg.indexOf("\"type\":\"joy\"") >= 0) {
        int xStart = msg.indexOf("\"x\":") + 4;
        int xEnd = msg.indexOf(",", xStart);
        int xVal = msg.substring(xStart, xEnd).toInt();
        
        int yStart = msg.indexOf("\"y\":") + 4;
        int yEnd = msg.indexOf("}", yStart);
        int yVal = msg.substring(yStart, yEnd).toInt();
        
        driveJoystick(xVal, yVal);
      }
    }
  }
}

void webServerBegin() {
  if (isAPMode()) {
    // ----------------------------------------------------
    // Captive Portal Server Routing
    // ----------------------------------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", (const uint8_t*)PORTAL_HTML, strlen(PORTAL_HTML));
      request->send(response);
    });
    
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
      int n = WiFi.scanNetworks();
      String json = "[";
      for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
      }
      json += "]";
      request->send(200, "application/json", json);
    });
    
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
      String ssid = "";
      String pass = "";
      
      if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
      }
      if (request->hasParam("pass", true)) {
        pass = request->getParam("pass", true)->value();
      }
      
      if (ssid.length() > 0) {
        Preferences prefs;
        prefs.begin("wifi-config", false);
        prefs.putString("ssid", ssid);
        prefs.putString("pass", pass);
        prefs.end();
        
        request->send(200, "text/html", 
          "<html><body style='background:#0a0e27;color:#00f0ff;font-family:sans-serif;text-align:center;padding:50px;'>"
          "<h2 style='font-size:24px;'>Credentials Saved!</h2>"
          "<p style='color:rgba(255,255,255,0.7);'>Radar Bot is rebooting to connect to <b>" + ssid + "</b>...</p>"
          "</body></html>");
        
        delay(2000);
        ESP.restart();
      } else {
        request->send(400, "text/plain", "Bad Request: SSID cannot be empty");
      }
    });

    // Captive Portal DNS redirection route
    server.onNotFound([](AsyncWebServerRequest *request){
      request->redirect("http://192.168.4.1/");
    });
    
  } else {
    // ----------------------------------------------------
    // Normal Operation Server Routing
    // ----------------------------------------------------
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", (const uint8_t*)INDEX_HTML, strlen(INDEX_HTML));
      request->send(response);
    });
    
    // REST routes (backwards compatibility fallback)
    server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("dir")) {
        String d = request->getParam("dir")->value();
        if      (d == "forward")  applyDriveCmd(FORWARD);
        else if (d == "backward") applyDriveCmd(BACKWARD);
        else if (d == "left")     applyDriveCmd(LEFT);
        else if (d == "right")    applyDriveCmd(RIGHT);
        else                      applyDriveCmd(STOPCMD);
      }
      request->send(200, "text/plain", "ok");
    });
    
    server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("val")) {
        driveSpeed = constrain(request->getParam("val")->value().toInt(), 80, 255);
      }
      request->send(200, "text/plain", "ok");
    });
    
    server.on("/radar", HTTP_GET, [](AsyncWebServerRequest *request){
      char json[128];
      snprintf(json, sizeof(json),
        "{\"angle\":%d,\"dist\":%ld,\"frontDist\":%ld,\"alert\":\"%s\"}",
        servoAngle, radarDist, frontDist, latestAlert.c_str());
      request->send(200, "application/json", json);
    });

    server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "text/plain", "404: Not Found");
    });
  }
  
  server.begin();
  Serial.println("[WEB] Async Web Server initialized");
}

void webServerBroadcastRadar(int angle, long dist, long frontDist, String alert) {
  static unsigned long lastBroadcast = 0;
  bool isAlert = (alert == "OBSTACLE");
  
  // Throttle regular updates to 10Hz to prevent socket buffer overflows,
  // but allow critical safety alerts to bypass the throttle.
  if (!isAlert && (millis() - lastBroadcast < 100)) {
    return;
  }
  lastBroadcast = millis();

  // Only broadcast if there are connected clients
  if (!isAPMode() && ws.count() > 0) {
    char json[128];
    snprintf(json, sizeof(json),
      "{\"angle\":%d,\"dist\":%ld,\"frontDist\":%ld,\"alert\":\"%s\"}",
      angle, dist, frontDist, alert.c_str());
    ws.textAll(json);
  }
}
