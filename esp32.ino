#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const char* supabaseUrl = "YOUR_SUPABASE_URL";
const char* supabaseKey = "YOUR_SUPABASE_ANON_KEY";

WebServer server(80);

int   currentRaw     = 0;
String currentStatus = "WET";
bool  isDry          = false;
unsigned long lastReceived  = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastSupabase  = 0;

void sendToSupabase(int raw, String status) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = String(supabaseUrl) + "/rest/v1/Readings";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Prefer", "return=minimal");
  String body = "{\"raw\":" + String(raw) + ",\"status\":\"" + status + "\"}";
  int code = http.POST(body);
  Serial.print("Supabase: ");
  Serial.println(code);
  http.end();
}

String getPage() {
  int pct      = map(currentRaw, 1023, 0, 0, 100);
  pct          = constrain(pct, 0, 100);
  String color = isDry ? "#e74c3c" : "#2ecc71";
  String msg   = isDry ? "Your plant needs water!" : "Soil moisture is healthy.";
  unsigned long age = (millis() - lastReceived) / 1000;
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Soil Monitor</title>";
  html += "<style>";
  html += "* { margin:0; padding:0; box-sizing:border-box; }";
  html += "body { font-family:'Segoe UI',Arial,sans-serif; background:#060d1a; color:#fff; min-height:100vh; display:flex; flex-direction:column; align-items:center; justify-content:center; padding:24px; }";
  html += "h1 { font-size:22px; letter-spacing:5px; color:#00e5ff; margin-bottom:6px; text-transform:uppercase; }";
  html += ".sub { font-size:12px; color:#374151; margin-bottom:28px; letter-spacing:2px; }";
  html += ".card { background:#0f172a; border-radius:20px; padding:28px; width:100%; max-width:380px; margin-bottom:16px; border:1px solid #1e293b; }";
  html += ".status-box { border-radius:14px; padding:28px 20px; text-align:center; background:" + color + "18; border:2px solid " + color + "; margin-bottom:20px; }";
  html += ".status-text { font-size:72px; font-weight:900; color:" + color + "; letter-spacing:8px; line-height:1; }";
  html += ".status-msg { font-size:14px; color:#9ca3af; margin-top:10px; }";
  html += ".section-label { font-size:11px; color:#4b5563; text-transform:uppercase; letter-spacing:3px; margin-bottom:8px; }";
  html += ".big-val { font-size:32px; font-weight:700; margin-bottom:14px; }";
  html += ".bar-bg { background:#1e293b; border-radius:10px; height:18px; overflow:hidden; margin-bottom:6px; position:relative; }";
  html += ".bar-fill { height:18px; border-radius:10px; background:" + color + "; width:" + String(pct) + "%; }";
  html += ".bar-tick { position:absolute; left:24.4%; top:0; width:2px; height:18px; background:#f59e0b; }";
  html += ".bar-labels { display:flex; justify-content:space-between; font-size:11px; color:#4b5563; margin-bottom:4px; }";
  html += ".tick-label { font-size:11px; color:#f59e0b; text-align:center; margin-bottom:16px; }";
  html += ".pill { display:inline-block; padding:4px 12px; border-radius:20px; font-size:12px; font-weight:600; }";
  html += ".pill-green { background:#064e3b; color:#2ecc71; }";
  html += ".pill-red { background:#450a0a; color:#e74c3c; }";
  html += ".footer { font-size:11px; color:#1f2937; margin-top:8px; text-align:center; }";
  html += ".dot { display:inline-block; width:7px; height:7px; border-radius:50%; background:#2ecc71; margin-right:5px; animation:pulse 1.5s infinite; }";
  html += "@keyframes pulse{0%,100%{opacity:1}50%{opacity:.2}}";
  html += ".divider { border:none; border-top:1px solid #1e293b; margin:16px 0; }";
  html += "</style></head><body>";
  html += "<h1>Plant Monitor</h1>";
  html += "<div class='sub'>SOIL HEALTH SYSTEM v1.0</div>";
  html += "<div class='card'>";
  html += "<div class='status-box'>";
  html += "<div class='status-text'>" + currentStatus + "</div>";
  html += "<div class='status-msg'>" + msg + "</div>";
  html += "</div>";
  html += "<div class='section-label'>Raw ADC Reading</div>";
  html += "<div class='big-val'>" + String(currentRaw) + " <span style='font-size:14px;color:#4b5563;font-weight:400;'>/ 1023</span></div>";
  html += "<div class='section-label'>Moisture Level</div>";
  html += "<div class='bar-bg'><div class='bar-fill'></div><div class='bar-tick'></div></div>";
  html += "<div class='bar-labels'><span>0</span><span>" + String(pct) + "%</span><span>1023</span></div>";
  html += "<div class='tick-label'>Threshold at 250</div>";
  html += "<hr class='divider'>";
  html += "<div class='section-label'>Status</div>";
  if (isDry) {
    html += "<span class='pill pill-red'>Needs Water</span>";
  } else {
    html += "<span class='pill pill-green'>Healthy</span>";
  }
  html += "</div>";
  html += "<div class='footer'><span class='dot'></span>Auto-refreshes every 5 seconds</div>";
  html += "<div class='footer'>Last reading: " + String(age) + "s ago</div>";
  html += "</body></html>";
  return html;
}

void reconnectWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    delay(500);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(4800, SERIAL_8N1, 16, 17);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! http://");
    Serial.println(WiFi.localIP());
    Serial2.println("WIFI_OK");
  } else {
    Serial.println("WiFi failed!");
    Serial2.println("WIFI_FAIL");
  }
  server.on("/", []() {
    server.send(200, "text/html", getPage());
  });
  server.begin();
}

void loop() {
  if (millis() - lastWifiCheck > 10000) {
    reconnectWifi();
    lastWifiCheck = millis();
  }
  server.handleClient();
  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && line.indexOf(',') > 0) {
      int commaIndex = line.indexOf(',');
      currentRaw    = line.substring(0, commaIndex).toInt();
      currentStatus = line.substring(commaIndex + 1);
      isDry         = (currentStatus == "DRY");
      lastReceived  = millis();
    }
  }
  if (millis() - lastSupabase > 5000 && lastReceived > 0) {
    sendToSupabase(currentRaw, currentStatus);
    lastSupabase = millis();
  }
}
