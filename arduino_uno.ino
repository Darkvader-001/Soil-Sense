#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define TFT_CS  10
#define TFT_DC   9
#define TFT_RST  8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define SENSOR_PIN A0
#define THRESHOLD  250

SoftwareSerial espSerial(3, 2);

unsigned long startTime;
int lastRaw = -1;

uint16_t COLOR_BG        = 0x0820;
uint16_t COLOR_HEADER    = 0x0C4A;
uint16_t COLOR_WHITE     = ST77XX_WHITE;
uint16_t COLOR_GREEN     = 0x07E0;
uint16_t COLOR_RED       = 0xF800;
uint16_t COLOR_YELLOW    = 0xFFE0;
uint16_t COLOR_CYAN      = 0x07FF;
uint16_t COLOR_DARKGRAY  = 0x4208;
uint16_t COLOR_LIGHTGRAY = 0x8410;

void drawHeader() {
  tft.fillRect(0, 0, 320, 36, COLOR_HEADER);
  tft.drawLine(0, 36, 320, 36, COLOR_CYAN);
  tft.setTextColor(COLOR_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("SOIL MONITOR");
}

void drawBattery(int percent) {
  int x = 248, y = 8, w = 52, h = 20, tip = 4;
  tft.fillRect(x-2, y-2, w+tip+4, h+4, COLOR_HEADER);
  uint16_t col = (percent > 60) ? COLOR_GREEN : (percent > 30) ? COLOR_YELLOW : COLOR_RED;
  tft.drawRect(x, y, w, h, COLOR_WHITE);
  tft.fillRect(x+w, y+5, tip, h-10, COLOR_WHITE);
  tft.fillRect(x+2, y+2, (int)((w-4)*percent/100.0), h-4, col);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(x+10, y+6);
  tft.print(percent);
  tft.print("%");
}

void drawWifiStatus(bool connected) {
  tft.fillRect(190, 8, 50, 14, COLOR_HEADER);
  tft.setTextSize(1);
  tft.setCursor(192, 10);
  if (connected) {
    tft.setTextColor(COLOR_GREEN);
    tft.print("WiFi OK");
  } else {
    tft.setTextColor(COLOR_RED);
    tft.print("No WiFi");
  }
}

void drawStatusBox(bool isDry) {
  tft.fillRect(0, 42, 320, 90, COLOR_BG);
  tft.drawRect(8, 46, 304, 80, isDry ? COLOR_RED : COLOR_GREEN);
  tft.fillRect(9, 47, 302, 78, isDry ? 0x2000 : 0x0200);
  tft.setTextColor(isDry ? COLOR_RED : COLOR_GREEN);
  tft.setTextSize(5);
  tft.setCursor(isDry ? 90 : 80, 60);
  tft.print(isDry ? "DRY" : "WET");
}

void drawMessage(bool isDry) {
  tft.fillRect(0, 134, 320, 24, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(10, 140);
  if (isDry) {
    tft.setTextColor(COLOR_RED);
    tft.print("  >> Please water your plant! <<");
  } else {
    tft.setTextColor(COLOR_GREEN);
    tft.print("   >> Soil moisture is healthy <<");
  }
}

void drawBarGraph(int raw) {
  int bx=10, by=168, bw=300, bh=22;
  tft.fillRect(0, 158, 320, 82, COLOR_BG);
  tft.setTextColor(COLOR_CYAN);
  tft.setTextSize(1);
  tft.setCursor(bx, 158);
  tft.print("RAW ADC");
  tft.setCursor(bx+60, 158);
  tft.setTextColor(COLOR_WHITE);
  tft.print(raw);
  tft.print(" / 1023");
  tft.setCursor(200, 158);
  tft.setTextColor(COLOR_LIGHTGRAY);
  tft.print("THRESHOLD: 250");
  tft.drawRect(bx, by, bw, bh, COLOR_LIGHTGRAY);
  tft.fillRect(bx+1, by+1, bw-2, bh-2, COLOR_DARKGRAY);
  tft.fillRect(bx+1, by+1, (int)(bw*raw/1023.0), bh-2,
               (raw > THRESHOLD) ? COLOR_RED : COLOR_GREEN);
  int tx = bx + (int)(bw * THRESHOLD / 1023.0);
  tft.drawLine(tx, by-2, tx, by+bh+2, COLOR_YELLOW);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LIGHTGRAY);
  tft.setCursor(bx, by+bh+6);
  tft.print("0");
  tft.setCursor(bx+bw-24, by+bh+6);
  tft.print("1023");
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(tx-8, by+bh+6);
  tft.print("250");
  tft.drawLine(0, 240, 320, 240, COLOR_CYAN);
  tft.setTextColor(COLOR_DARKGRAY);
  tft.setTextSize(1);
  tft.setCursor(90, 244);
  tft.print("PLANT HEALTH SYSTEM v1.0");
}

int getBatteryPercent() {
  unsigned long elapsed = (millis() - startTime) / 1000UL;
  int percent = 100 - (int)(elapsed * 100UL / 18000UL);
  return max(0, percent);
}

bool wifiConnected = false;

void setup() {
  Serial.begin(9600);
  espSerial.begin(4800);
  startTime = millis();
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH); delay(50);
  digitalWrite(TFT_RST, LOW);  delay(50);
  digitalWrite(TFT_RST, HIGH); delay(50);
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);
  delay(100);
  drawHeader();
  tft.setTextColor(COLOR_CYAN);
  tft.setTextSize(2);
  tft.setCursor(60, 110);
  tft.print("Starting up...");
  delay(1500);
  tft.fillScreen(COLOR_BG);
  drawHeader();
}

void loop() {
  int raw    = analogRead(SENSOR_PIN);
  bool isDry = raw > THRESHOLD;
  int batPct = getBatteryPercent();
  if (espSerial.available()) {
    String msg = espSerial.readStringUntil('\n');
    msg.trim();
    if (msg == "WIFI_OK")   wifiConnected = true;
    if (msg == "WIFI_FAIL") wifiConnected = false;
  }
  drawBattery(batPct);
  drawWifiStatus(wifiConnected);
  if (raw != lastRaw) {
    drawStatusBox(isDry);
    drawMessage(isDry);
    drawBarGraph(raw);
    lastRaw = raw;
  }
  String data = String(raw) + "," + String(isDry ? "DRY" : "WET") + "\n";
  espSerial.print(data);
  delay(1000);
}
