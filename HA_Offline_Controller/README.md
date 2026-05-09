# Office Control Panel

A standalone ESP32 physical control panel for Home Assistant with rotary encoder navigation and TFT display.

## Features

- Physical rotary encoder navigation (rotate, press, hold)
- ILI9341 2.4" TFT display with dark glass UI theme
- WiFi + MQTT connectivity for Home Assistant integration
- No ESPHome — pure Arduino firmware
- Real-time state sync between HA and panel
- Auto-reconnect on WiFi/MQTT disconnect
- Node-RED MQTT bridge for HA automation
- 7 controllable devices across 2 rooms
- Boot screen with Southern IoT branding

## Hardware Components

| Component | Model/Spec | Purpose |
|---|---|---|
| Microcontroller | ESP32 DevKit v1 | Main processor |
| Display | ILI9341 2.4" SPI TFT | UI display |
| Rotary Encoder | KY-040 or equivalent | Navigation input |
| Power | USB-C 5V | Power supply |

## System Architecture

```
+-------------+      WiFi       +-----------+      MQTT       +-----------+
|   ESP32     |----------------|  Mosquitto|----------------|  Node-RED |
| Control Panel|               |  Broker   |               |          |
| (Arduino)   |<---------------| (Docker)  |<--------------|          |
+-------------+  subscribes    +-----------+  publishes   +-----------+
                                      ^                         |
                                      |                         v
                              subscribes to              calls services
                                  /state                      |
                                     ^                         v
                                     |              +------------------+
                                     +--------------| Home Assistant   |
                                       publishes    |                  |
                                       to /set      +------------------+
```

![ESP32 MQTT Control Ecosystem](ESP32%20MQTT%20Control%20Ecosystem-2026-05-08-175636.png)

## Software Stack

| Layer | Technology | Version |
|---|---|---|
| Firmware | Arduino ESP32 | 2.0.x |
| Display | TFT_eSPI | 2.5.x |
| MQTT | PubSubClient | 2.8.x |
| Encoder | ESP32Encoder | Latest |
| Broker | Mosquitto | 2.x |
| Automation | Node-RED | 3.x |
| Smart Home | Home Assistant | 2024.x |

## Installation

### 1. Install Arduino Libraries

Open Arduino IDE → Sketch → Include Library → Manage Libraries, then install:

- TFT_eSPI by Bodmer
- PubSubClient by Nick O'Leary
- ESP32Encoder by Lennart Hennigs
- WiFi (built into ESP32 core)

### 2. Configure TFT_eSPI

Replace `Arduino/libraries/TFT_eSPI/User_Setup.h` with the provided `User_Setup.h` file:

```cpp
#define ILI9341_DRIVER
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_MISO  12
#define TFT_CS    15
#define TFT_DC    33
#define TFT_RST    4
#define TFT_BL    32
#define TFT_BACKLIGHT_ON HIGH
#define SMOOTH_FONT
#define SPI_FREQUENCY 27000000
```

### 3. Configure Firmware

Edit `firmware/main/main.ino` and replace placeholders:

```cpp
// ====== SECRETS - REPLACE WITH YOUR VALUES ======
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_BROKER   = "192.168.0.147";  // Your MQTT broker IP
// ===============================================
```

### 4. Flash Firmware

1. Connect ESP32 via USB
2. Select Tools → Partition Scheme → "Huge APP"
3. Select correct port
4. Upload sketch

## Display Wiring (ILI9341)

| ILI9341 Pin | ESP32 GPIO |
|---|---|
| VCC | 3.3V |
| GND | GND |
| CS | GPIO15 |
| RST | GPIO4 |
| RS/DC | GPIO33 |
| MOSI | GPIO13 |
| SCK | GPIO14 |
| LED | GPIO32 |
| MISO | GPIO12 |

## Encoder Wiring (KY-040)

| KY-040 Pin | ESP32 GPIO |
|---|---|
| VCC | 3.3V |
| GND | GND |
| CLK | GPIO18 |
| DT | GPIO19 |
| SW | GPIO5 |

## Home Assistant Integration

### Step 1: Deploy Mosquitto

```yaml
# docker-compose.yml
services:
  mosquitto:
    image: eclipse-mosquitto:2
    container_name: mosquitto
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto.conf:/mosquitto/config/mosquitto.conf
    restart: unless-stopped
```

### Step 2: Add MQTT Integration in HA

Settings → Devices & Services → Add Integration → MQTT

### Step 3: Install Node-RED HA Plugin

In Node-RED: Menu → Manage Palette → Install `node-red-contrib-home-assistant-websocket`

### Step 4: Configure HA Server in Node-RED

1. Generate Long-Lived Access Token in HA: Profile → Long-Lived Access Tokens → Create
2. Add HA Server node with base URL `http://192.168.0.147:8123` and token

### Step 5: Import Node-RED Flow

Import `nodered/flow.json` into Node-RED

### Step 6: Verify MQTT

```bash
mosquitto_sub -h 192.168.0.147 -t "#" -v
```

## MQTT Topics

### Lab Room

| Device | Command Topic | State Topic |
|---|---|---|
| Light 1 | `homeassistant/light/lab_room_room_light_1/set` | `homeassistant/light/lab_room_room_light_1/state` |
| Light 2 | `homeassistant/light/lab_room_room_light_2/set` | `homeassistant/light/lab_room_room_light_2/state` |
| PIR | — | `homeassistant/binary_sensor/lab_room_pir_motion_sensor/state` |

### Main Room

| Device | Command Topic | State Topic |
|---|---|---|
| Light 1 | `homeassistant/light/main_room_room_light_1/set` | `homeassistant/light/main_room_room_light_1/state` |
| Light 2 | `homeassistant/light/main_room_room_light_2/set` | `homeassistant/light/main_room_room_light_2/state` |
| Light 3 | `homeassistant/light/main_room_room_light_3/set` | `homeassistant/light/main_room_room_light_3/state` |
| Door Lock | `homeassistant/switch/main_room_door_lock/set` | `homeassistant/switch/main_room_door_lock/state` |

## Node-RED Flow

The flow handles bidirectional communication:

- **HA → MQTT**: Listens to HA entity state changes, publishes to `/state` topics with retain=true
- **MQTT → HA**: Subscribes to `/set` topics, calls HA service APIs on incoming messages

## UI Navigation

| Action | Result |
|---|---|
| Rotate CW | Scroll down / increase value |
| Rotate CCW | Scroll up / decrease value |
| Press | Enter submenu / toggle device |
| Hold 1s | Back to parent menu |

## Folder Structure

```
office-control-panel/
├── firmware/
│   └── main/
│       ├── main.ino          # Arduino firmware
│       └── User_Setup.h      # TFT_eSPI config
├── nodered/
│   └── flow.json             # Node-RED flow
├── hardware/
│   ├── schematic/
│   │   └── schematic.md      # Wiring reference
│   ├── 3d/
│   │   └── README.md         # 3D files placeholder
│   └── images/
│       └── README.md        # Photos placeholder
├── docs/
│   ├── architecture.md       # System architecture
│   ├── mqtt_topics.md        # MQTT topic reference
│   ├── nodered_flow.md      # Node-RED documentation
│   ├── ha_integration.md    # HA setup guide
│   └── ui_navigation.md     # UI reference
├── LICENSE
└── README.md
```

## Author

**Julfiker Ibn Haider (Alif)**  
Southern IoT, Dhaka, Bangladesh

- [ALifHaider19](https://github.com/ALifHaider19)
- [julfikerhaider2001](https://github.com/julfikerhaider2001)

## License

MIT License
