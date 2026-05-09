# Home Assistant Integration Guide

Complete step-by-step guide to integrating the ESP32 control panel with Home Assistant.

---

## Step 1: Deploy Mosquitto MQTT Broker

Run Mosquitto as a Docker container on the same host machine or network.

### docker-compose.yml

```yaml
version: '3'
services:
  mosquitto:
    image: eclipse-mosquitto:2
    container_name: mosquitto
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto.conf:/mosquitto/config/mosquitto.conf
      - ./mosquitto/data:/mosquitto/data
      - ./mosquitto/log:/mosquitto/log
    restart: unless-stopped
```

### mosquitto.conf

```
listener 1883
allow_anonymous true
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
log_dest stdout
```

### Start the container

```bash
docker-compose up -d
```

### Verify

```bash
docker ps | grep mosquitto
```

---

## Step 2: Add MQTT Integration in Home Assistant

1. Open Home Assistant (`http://192.168.0.147:8123`)
2. Go to **Settings** → **Devices & Services**
3. Click **Add Integration**
4. Search for **MQTT**
5. Click on MQTT → **Configure**
6. Enter MQTT broker IP: `192.168.0.147`
7. Enter port: `1883`
8. Enable **Discovery** (optional — can be disabled)
9. Click **Submit**
10. If prompted for a user, use your HA username/password or create an MQTT user

### Note
The MQTT integration in HA is for built-in MQTT entity discovery. Since we're using Node-RED as the primary bridge, discovery can be left disabled. The entities are controlled via Node-RED, not directly via HA's MQTT integration.

---

## Step 3: Install Node-RED HA Plugin

1. Open Node-RED (`http://localhost:1880`)
2. Click the **hamburger menu** → **Manage Palette**
3. Go to the **Install** tab
4. Search for `node-red-contrib-home-assistant-websocket`
5. Click **Install**
6. Wait for installation to complete
7. Restart Node-RED if prompted

### Alternative: Install via command line

```bash
cd ~/.node-red
npm install node-red-contrib-home-assistant-websocket
sudo systemctl restart nodered
```

---

## Step 4: Configure HA Server Node in Node-RED

1. Open Node-RED
2. Drag an **HA Server** node from the sidebar onto the canvas
3. Double-click the node to configure:
   - **Host**: `192.168.0.147`
   - **Port**: `8123`
   - **Secure**: Enable SSL only if HA has `ssl_certificate` configured
4. Click **Done**

### Generate Long-Lived Access Token in Home Assistant

1. Open Home Assistant
2. Click on your **profile icon** (top left)
3. Scroll to **Long-Lived Access Tokens**
4. Click **Create Token**
5. Enter a name: `Node-RED`
6. Click **Create**
7. **Copy the token immediately** — it will not be shown again
8. Paste the token into the HA Server node configuration in Node-RED

### Verify connection

1. Deploy the flow with the HA Server node
2. Check the Node-RED debug panel (right sidebar)
3. Look for a green "connected" status on the HA Server node

---

## Step 5: Import Node-RED Flow

1. Open Node-RED
2. Click the **hamburger menu** → **Import** → **Clipboard**
3. Open `nodered/flow.json` from this repository
4. Copy the entire JSON content
5. Paste into the import dialog
6. Click **Import**
7. The flow will appear on the canvas
8. Double-check the MQTT broker configuration node has the correct IP (`192.168.0.147`) and port (`1883`)
9. Click **Deploy**

### Verify the flow

1. Open the debug panel (right sidebar)
2. You should see messages flowing from `server-state-changed` nodes
3. Use `mosquitto_sub` to verify MQTT messages are being published:

```bash
mosquitto_sub -h 192.168.0.147 -t "#" -v
```

---

## Step 6: Configure TFT_eSPI User_Setup.h

1. Locate your Arduino libraries folder. Common paths:
   - Windows: `Documents/Arduino/libraries/`
   - Linux: `~/Arduino/libraries/`
   - macOS: `~/Documents/Arduino/libraries/`

2. Navigate to `TFT_eSPI/` folder
3. Open `User_Setup.h` in a text editor
4. Replace the entire file content with the provided `firmware/main/User_Setup.h`

**Key settings:**
```cpp
#define ILI9341_DRIVER
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_MISO  12
#define TFT_CS    15
#define TFT_DC    33
#define TFT_RST    4
#define TFT_BL    32
#define SPI_FREQUENCY  27000000
#define SMOOTH_FONT
```

5. Save the file

### Important: Comment out other drivers

Make sure all other display driver `#define` statements are commented out. Only `#define ILI9341_DRIVER` should be active.

```cpp
// #define RPI_ILI9486_DRIVER
// #define ST7735_DRIVER
// #define ... (all others commented out)
```

---

## Step 7: Flash Firmware to ESP32

1. Connect ESP32 DevKit v1 to your computer via USB
2. Open the `firmware/main/main.ino` file in Arduino IDE
3. Go to **Tools** → **Board** → **ESP32 Arduino** → **ESP32 Dev Module**
4. Go to **Tools** → **Partition Scheme** → **Huge APP (3MB No OTA)**
5. Go to **Tools** → **Port** → select the correct COM port
6. Go to **Sketch** → **Upload**

Or use PlatformIO:

```bash
cd firmware/main
pio run -t upload
```

### Configure credentials

Before uploading, edit `firmware/main/main.ino` and replace the placeholders at the top:

```cpp
// ====== SECRETS - REPLACE WITH YOUR VALUES ======
const char* WIFI_SSID     = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";
const char* MQTT_BROKER   = "192.168.0.147";
const int   MQTT_PORT     = 1883;
const char* MQTT_CLIENT_ID = "control_panel";
// ===============================================
```

---

## Step 8: Verify with mosquitto_sub

From any machine on the network:

```bash
mosquitto_sub -h 192.168.0.147 -t "#" -v
```

You should see state messages like:

```
homeassistant/light/lab_room_room_light_1/state ON
homeassistant/light/main_room_room_light_3/state OFF
homeassistant/switch/main_room_door_lock/state ON
```

### Test sending a command

```bash
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_1/set -m "ON"
```

The ESP32 display should update within seconds.

---

## Quick Reference

| Step | Action | Time |
|---|---|---|
| 1 | Deploy Mosquitto Docker | 5 min |
| 2 | Add MQTT integration in HA | 5 min |
| 3 | Install HA plugin in Node-RED | 5 min |
| 4 | Configure HA server in Node-RED | 10 min |
| 5 | Import Node-RED flow | 2 min |
| 6 | Configure TFT_eSPI | 2 min |
| 7 | Flash firmware | 3 min |
| 8 | Verify with mosquitto_sub | 2 min |

**Total estimated time: ~35 minutes**
