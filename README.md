# Soil-Sense

A full-stack IoT soil moisture monitoring system built with Arduino Uno, ESP32, a 2" ST7789V LCD display, and a capacitive soil moisture sensor. Live data is pushed to Supabase and visualized in a premium mobile app built with Emergent AI.

## Features

- Real-time soil moisture reading on a 2" color LCD display
- DRY / WET status with color-coded alerts
- Raw ADC bar graph with threshold marker
- Battery level indicator
- WiFi connectivity via ESP32
- Live data pushed to Supabase cloud database every 5 seconds
- Local web dashboard accessible via browser on same network
- Premium mobile app built with Emergent AI

## Hardware Required

- Arduino Uno R3
- ESP32-WROOM-32 (WiFi + BT)
- Waveshare 2" ST7789V LCD Module (240x320)
- Capacitive soil moisture sensor
- Mini breadboard
- Female-to-female jumper wires
- 9V battery + barrel connector (for portable use)

## Wiring

### Arduino Uno → LCD (ST7789V)
| Arduino | LCD |
|---------|-----|
| Pin 13 | CLK |
| Pin 11 | DIN |
| Pin 10 | CS |
| Pin 9 | DC |
| Pin 8 | RST |
| Breadboard + rail | VCC + BL |
| Breadboard - rail | GND |

### Arduino Uno → Soil Moisture Sensor
| Arduino | Sensor |
|---------|--------|
| A0 | SIG |
| Breadboard + rail | VCC |
| Breadboard - rail | GND |

### Arduino Uno → ESP32
| Arduino | ESP32 |
|---------|-------|
| Pin 2 | GPIO16 (RX2) |
| 5V | VIN |
| GND (via breadboard) | GND |

### Arduino Uno → Breadboard
| Arduino | Breadboard |
|---------|------------|
| 3.3V | Red rail (+) |
| GND | Blue rail (-) |

## Moisture Threshold

- Raw ADC above 250 = DRY
- Raw ADC below 250 = WET

## Software Setup

### Arduino Libraries Required
- Adafruit ST7789
- Adafruit GFX Library

### Arduino IDE Board Settings
- Arduino sketch: Board = Arduino Uno, Port = COM7
- ESP32 sketch: Board = ESP32 Dev Module, Port = COM3

### Configuration
Before uploading, update these values in `esp32.ino`:

```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* supabaseUrl = "YOUR_SUPABASE_URL";
const char* supabaseKey = "YOUR_SUPABASE_ANON_KEY";
```

### Supabase Setup
1. Create a free project at supabase.com
2. Create a table called `Readings` with columns:
   - `id` (int8, primary key)
   - `raw` (int8)
   - `status` (text)
   - `created_at` (timestamptz)
3. Enable public RLS policy for insert and select

## Upload Order
1. Upload `esp32.ino` to ESP32 (hold BOOT button if needed)
2. Upload `arduino_uno.ino` to Arduino Uno
3. Connect wires as per wiring table
4. Power via USB or 9V battery on barrel jack

## Web Dashboard
Once the ESP32 connects to WiFi, open Serial Monitor at 115200 baud to find the IP address. Open that IP in any browser on the same network.

## Mobile App
Built with Emergent AI connecting directly to Supabase. Features live DRY/WET status, moisture bar graph, raw ADC gauge, history chart, and push notifications.

## License
MIT License — open source, use freely.

## Author
Darkvader-001
