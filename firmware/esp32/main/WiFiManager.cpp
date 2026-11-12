#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "ATLASCore.h"
#include "WiFiManager.h"
#include "MotorControl.h"
#include "ObstacleAvoidance.h"
#include "GoogleSheets.h"

static WebServer dashboardServer(80);
static bool dashboardPreviouslyConnected = false;
static unsigned long dashboardLastReconnectAttempt = 0;

static const char DASHBOARD_HTML[] PROGMEM = R"ATLASHTML(
<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" content="#101827"><title>ATLAS AGV | Local Control</title>
<style>
:root{color-scheme:dark;--bg:#0b1220;--panel:#141f31;--line:#26364d;--muted:#91a2ba;--text:#edf4ff;--cyan:#45d6e8;--green:#39d98a;--amber:#ffc766;--red:#ff6b78}*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at top,#1a2c43 0,#0b1220 55%);font:15px system-ui,Segoe UI,Arial;color:var(--text);min-height:100vh}.wrap{max-width:1100px;margin:auto;padding:26px 18px 40px}.top{display:flex;justify-content:space-between;align-items:center;gap:15px;margin-bottom:22px}.brand{display:flex;align-items:center;gap:12px}.logo{width:45px;height:45px;border-radius:13px;background:linear-gradient(135deg,#1aa6bd,#2454a6);display:grid;place-items:center;font-size:23px;font-weight:800}.eyebrow{color:var(--cyan);font-size:11px;letter-spacing:2px;font-weight:800}.title{font-size:25px;font-weight:800;letter-spacing:.3px}.sub{color:var(--muted);font-size:12px}.live{border:1px solid var(--line);border-radius:30px;padding:8px 12px;color:var(--green);font-size:12px}.dot{display:inline-block;width:7px;height:7px;background:var(--green);border-radius:50%;margin-right:7px;box-shadow:0 0 10px var(--green)}.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:13px}.card{background:linear-gradient(145deg,#172438,#111b2b);border:1px solid var(--line);border-radius:17px;padding:18px;box-shadow:0 12px 35px #0002}.metric{min-height:115px}.label{color:var(--muted);font-size:12px;text-transform:uppercase;letter-spacing:1px}.value{font-size:22px;font-weight:750;margin-top:12px;overflow-wrap:anywhere}.small{color:var(--muted);font-size:12px;margin-top:5px}.section{margin:23px 0 12px;font-size:14px;color:#c6d5e8;letter-spacing:1px;text-transform:uppercase}.main{display:grid;grid-template-columns:1.15fr .85fr;gap:14px}.statusline{display:flex;justify-content:space-between;align-items:center;padding:13px 0;border-bottom:1px solid var(--line)}.statusline:last-child{border:0}.pill{border-radius:30px;padding:6px 10px;font-size:12px;background:#25364b;color:#dbeafe}.controls{display:grid;gap:12px}select,button{font:inherit;border-radius:11px;padding:12px 14px;border:1px solid var(--line);background:#0e1929;color:var(--text);width:100%}button{cursor:pointer;font-weight:750;background:#145a78;border-color:#237f9b}button.secondary{background:#26364d}button.danger{background:#632b39;border-color:#8b3b4b}button:disabled{opacity:.4;cursor:not-allowed}.btnrow{display:grid;grid-template-columns:1fr 1fr;gap:10px}.note{color:var(--muted);font-size:12px;line-height:1.5;margin-top:12px}.radar{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:14px}.zone{text-align:center;border:1px solid var(--line);border-radius:12px;padding:12px 5px;background:#0d1928}.zone b{display:block;font-size:17px;margin-top:7px}.footer{text-align:center;color:#71839c;font-size:11px;margin-top:25px}.toast{position:fixed;bottom:18px;left:50%;transform:translateX(-50%);background:#20354a;border:1px solid #36516e;border-radius:12px;padding:12px 18px;display:none;max-width:90%;z-index:2}@media(max-width:760px){.grid{grid-template-columns:repeat(2,1fr)}.main{grid-template-columns:1fr}.title{font-size:21px}}@media(max-width:380px){.grid{grid-template-columns:1fr}.top{align-items:flex-start;flex-direction:column}}
</style></head><body><div class="wrap">
<header class="top"><div class="brand"><div class="logo">A</div><div><div class="eyebrow">SMART WAREHOUSE ROBOTICS</div><div class="title">ATLAS AGV</div><div class="sub">Local monitoring & control panel</div></div></div><div class="live"><span class="dot"></span><span id="connection">CONNECTING</span></div></header>
<div class="grid">
<div class="card metric"><div class="label">Robot state</div><div class="value" id="robot">--</div><div class="small">Current operating mode</div></div>
<div class="card metric"><div class="label">Delivery</div><div class="value" id="delivery">--</div><div class="small">Mission progress</div></div>
<div class="card metric"><div class="label">Wi-Fi signal</div><div class="value" id="rssi">--</div><div class="small" id="ip">IP: --</div></div>
<div class="card metric"><div class="label">Total missions</div><div class="value" id="count">0</div><div class="small">Delivery counter</div></div>
</div>
<div class="section">Live system overview</div><div class="main">
<div class="card"><div class="statusline"><span>Selected destination</span><span class="pill" id="warehouse">None</span></div><div class="statusline"><span>Obstacle detection</span><span class="pill" id="obstacle">Clear</span></div><div class="statusline"><span>Radar direction</span><span class="pill" id="radar">Front</span></div><div class="statusline"><span>Servo angle</span><span class="pill" id="angle">90°</span></div><div class="statusline"><span>ESP32-CAM</span><span class="pill" id="camera">--</span></div><div class="statusline"><span>Last upload</span><span class="pill" id="upload">--</span></div>
<div class="radar"><div class="zone">LEFT<b id="left">--</b></div><div class="zone">FRONT<b id="front">--</b></div><div class="zone">RIGHT<b id="right">--</b></div></div></div>
<div class="card"><div class="label">Mission controls</div><div class="controls" style="margin-top:15px"><label for="destination" class="small">Choose warehouse destination</label><select id="destination"><option value="0">Select warehouse</option><option value="1">Warehouse 1</option><option value="2">Warehouse 2</option><option value="3">Warehouse 3</option></select><button id="start" onclick="control('start')">START DELIVERY</button><button class="secondary" id="return" onclick="control('return')">RETURN TO BASE</button><div class="note">Controls are enabled only when the robot is in a safe state. RFID checkpoints remain responsible for confirming arrival and return.</div></div></div>
</div><div class="footer">ATLAS • ESP32 local dashboard • Auto-refresh every 1.5 seconds</div></div><div class="toast" id="toast"></div>
<script>
const $=id=>document.getElementById(id);let busy=false;
function showToast(t){$('toast').textContent=t;$('toast').style.display='block';setTimeout(()=>$('toast').style.display='none',2600)}
async function refresh(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error('HTTP '+r.status);const d=await r.json();$('connection').textContent='LIVE';$('robot').textContent=d.robot;$('delivery').textContent=d.delivery;$('rssi').textContent=d.rssi+' dBm';$('ip').textContent='IP: '+d.ip;$('count').textContent=d.count;$('warehouse').textContent=d.warehouse===0?'None':'W'+d.warehouse;$('obstacle').textContent=d.obstacle?'Detected':'Clear';$('obstacle').style.color=d.obstacle?'#ff6b78':'#39d98a';$('radar').textContent=d.radar;$('angle').textContent=d.angle+'°';$('camera').textContent=d.camera;$('upload').textContent=d.upload;$('left').textContent=d.left;$('front').textContent=d.front;$('right').textContent=d.right;if(document.activeElement!==$('destination'))$('destination').value=d.warehouse;$('start').disabled=busy||d.state!=='IDLE'||d.warehouse===0;$('return').disabled=busy||d.state!=='AT_WAREHOUSE';}catch(e){$('connection').textContent='OFFLINE';$('connection').style.color='#ff6b78'}}
async function control(action){if(busy)return;busy=true;try{const body=action==='start'?{warehouse:Number($('destination').value)}:{};const r=await fetch('/api/'+action,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});const d=await r.json();showToast(d.message|| (d.ok?'Command accepted':'Command rejected'));}catch(e){showToast('Could not reach robot dashboard')}finally{busy=false;refresh()}}
$('destination').addEventListener('change',async e=>{try{const r=await fetch('/api/warehouse',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({warehouse:Number(e.target.value)})});const d=await r.json();showToast(d.message)}catch(e){showToast('Warehouse update failed')}});refresh();setInterval(refresh,1500);
</script></body></html>
)ATLASHTML";

static String jsonEscape(const String &s) {
  String out; out.reserve(s.length()+8);
  for (size_t i=0;i<s.length();i++) { char c=s[i]; if(c=='"'||c=='\\')out+='\\'; if(c=='\n')out+="\\n"; else if(c!='\r')out+=c; }
  return out;
}
static String distanceText(float d) { return d < 0 ? "--" : String(d,0)+" cm"; }
static void sendMessage(int code, bool ok, const String &message) {
  String body="{\"ok\":"+String(ok?"true":"false")+",\"message\":\""+jsonEscape(message)+"\"}";
  dashboardServer.send(code,"application/json",body);
}
static void handleStatus() {
  String state;
  switch(currentState){case IDLE:state="IDLE";break;case MOVING_TO_WAREHOUSE:state="MOVING";break;case OBSTACLE_STOPPED:state="OBSTACLE_STOPPED";break;case AT_WAREHOUSE:state="AT_WAREHOUSE";break;case RETURNING:state="RETURNING";break;case ARRIVED_HOME:state="ARRIVED_HOME";break;default:state="UNKNOWN";}
  String robot=lastRobotStatusText.length()?lastRobotStatusText:state;
  String delivery=lastDeliveryStatusText.length()?lastDeliveryStatusText:"Waiting";
  String body="{";
  body+="\"state\":\""+state+"\",\"robot\":\""+jsonEscape(robot)+"\",\"delivery\":\""+jsonEscape(delivery)+"\",";
  body+="\"warehouse\":"+String(selectedWarehouse)+",\"obstacle\":"+String(obstaclePresent?"true":"false")+",\"radar\":\""+jsonEscape(radarCurrentZoneLabel)+"\",\"angle\":"+String(radarCurrentAngle)+",";
  body+="\"camera\":\""+jsonEscape(lastCameraStatusText.length()?lastCameraStatusText:(cameraReady?"Ready":"Offline"))+"\",\"upload\":\""+jsonEscape(lastUploadStatus)+"\",\"count\":"+String(deliveryCounter)+",";
  body+="\"rssi\":"+String(WiFi.status()==WL_CONNECTED?WiFi.RSSI():0)+",\"ip\":\""+(WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():String("Not connected"))+"\",";
  body+="\"left\":\""+distanceText(radarLeftDistanceCm)+"\",\"front\":\""+distanceText(radarFrontDistanceCm)+"\",\"right\":\""+distanceText(radarRightDistanceCm)+"\"}";
  dashboardServer.send(200,"application/json",body);
}
static void handleWarehouse() {
  if(!dashboardServer.hasArg("plain")){sendMessage(400,false,"Missing request body");return;}
  String body=dashboardServer.arg("plain"); int p=body.indexOf("\"warehouse\""); int colon=body.indexOf(':',p);
  if(p<0||colon<0){sendMessage(400,false,"Warehouse value missing");return;}
  int value=body.substring(colon+1).toInt();
  if(value<0||value>3){sendMessage(400,false,"Choose W1, W2 or W3");return;}
  if(currentState!=IDLE){sendMessage(409,false,"Warehouse can only be changed while robot is idle");return;}
  selectedWarehouse=value;
  setDeliveryStatus(value?"Warehouse Selected":"Waiting");
  sendMessage(200,true,value?"Warehouse W"+String(value)+" selected":"Warehouse selection cleared");
}
static void handleStart() {
  if(!dashboardServer.hasArg("plain")){sendMessage(400,false,"Missing request body");return;}
  String body=dashboardServer.arg("plain"); int p=body.indexOf("\"warehouse\""); int colon=body.indexOf(':',p);
  int value=(p>=0&&colon>=0)?body.substring(colon+1).toInt():selectedWarehouse;
  if(value<1||value>3){sendMessage(400,false,"Select a warehouse first");return;}
  if(currentState!=IDLE){sendMessage(409,false,"Robot is not idle");return;}
  selectedWarehouse=value; deliveryCounter++; deliveryStartMillis=millis(); resetRadarMissionSummary();
  lastUploadStatus="Disabled"; lastDriveImageUrl="-"; lastCameraStatusForSheet=cameraReady?"Camera Ready":"Camera Offline";
  radarHoldCenter=false; obstaclePresent=false; currentState=MOVING_TO_WAREHOUSE;
  setRobotStatus("Starting..."); setDeliveryStatus("Package In Transit");
  logDeliveryToSheets(selectedWarehouse,"Started","-");
  Serial.println("Robot started from local dashboard.");
  sendMessage(200,true,"Delivery to W"+String(value)+" started");
}
static void handleReturn() {
  if(currentState!=AT_WAREHOUSE){sendMessage(409,false,"Return is available only after warehouse arrival");return;}
  radarHoldCenter=false; obstaclePresent=false; currentState=RETURNING;
  Serial.println("Return to start requested from local dashboard.");
  sendMessage(200,true,"Return-to-base started");
}
void initWiFiManager() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println();
  Serial.println("[WiFi] Connecting to configured network...");
  Serial.print("[WiFi] SSID: "); Serial.println(WIFI_SSID);

  dashboardServer.on("/",HTTP_GET,[]{dashboardServer.send_P(200,"text/html",DASHBOARD_HTML);});
  dashboardServer.on("/api/status",HTTP_GET,handleStatus);
  dashboardServer.on("/api/warehouse",HTTP_POST,handleWarehouse);
  dashboardServer.on("/api/start",HTTP_POST,handleStart);
  dashboardServer.on("/api/return",HTTP_POST,handleReturn);
  dashboardServer.onNotFound([](){sendMessage(404,false,"Endpoint not found");});
  dashboardServer.begin();
  Serial.println("[Dashboard] HTTP server started on port 80.");
  Serial.println("[Dashboard] Waiting for Wi-Fi; the actual URL will print after connection.");
}
void serviceLocalDashboard() { dashboardServer.handleClient(); }
void checkWiFiReconnect() {
  bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !dashboardPreviouslyConnected) {
    Serial.println();
    Serial.println("========== ATLAS LOCAL DASHBOARD ==========");
    Serial.print("[WiFi] Connected to: "); Serial.println(WiFi.SSID());
    Serial.print("[WiFi] Signal: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
    Serial.print("[WiFi] ESP32 IP: "); Serial.println(WiFi.localIP());
    Serial.print("[Dashboard] OPEN THIS ADDRESS: http://");
    Serial.print(WiFi.localIP()); Serial.println("/");
    Serial.println("============================================");
  } else if (!connected && dashboardPreviouslyConnected) {
    Serial.println("[WiFi] Connection lost. Reconnecting...");
  }
  dashboardPreviouslyConnected = connected;

  if (!connected && millis() - dashboardLastReconnectAttempt >= 10000UL) {
    dashboardLastReconnectAttempt = millis();
    Serial.print("[WiFi] Not connected. Status code: ");
    Serial.println((int)WiFi.status());
    Serial.println("[WiFi] Retrying connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}
