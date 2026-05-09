#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Encoder.h>

// ====================== CONFIG ======================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_BROKER   = "192.168.0.147";
const int   MQTT_PORT     = 1883;
const char* MQTT_CLIENT   = "control_panel";

// ====================== PINS ======================
#define ENCODER_CLK  18
#define ENCODER_DT   19
#define ENCODER_SW    5
#define TFT_BL_PIN   32

// ====================== COLORS ======================
#define BG_COLOR        TFT_BLACK
#define HEADER_COLOR    0x1082   // Very dark gray
#define ITEM_BG         0x0841   // Dark
#define ITEM_SELECTED   0x02B5   // Dark teal
#define TEXT_COLOR      TFT_WHITE
#define TEXT_DIM        0x8410   // Gray
#define ACCENT_COLOR    0x07FF   // Cyan
#define ON_COLOR        0x07E0   // Green
#define OFF_COLOR       0xF800   // Red
#define BORDER_COLOR    0x2945   // Dark border

// ====================== DISPLAY ======================
TFT_eSPI tft = TFT_eSPI();

// ====================== ENCODER ======================
ESP32Encoder encoder;
long lastEncoderPos = 0;

// ====================== MQTT ======================
WiFiClient espClient;
PubSubClient mqtt(espClient);

// ====================== MENU STRUCTURE ======================
struct Device {
  const char* name;
  const char* entity_id;
  const char* cmd_topic;
  const char* state_topic;
  bool state;
  bool is_sensor;
};

struct Menu {
  const char* name;
  Device devices[5];
  int deviceCount;
};

Menu menus[] = {
  {
    "Lab Room", {
      {"Light 1",  "light.lab_room_room_light_1", "homeassistant/light/lab_room_room_light_1/set",  "homeassistant/light/lab_room_room_light_1/state",  false, false},
      {"Light 2",  "light.lab_room_room_light_2", "homeassistant/light/lab_room_room_light_2/set",  "homeassistant/light/lab_room_room_light_2/state",  false, false},
      {"PIR",      "binary_sensor.lab_room_pir_motion_sensor", "", "homeassistant/binary_sensor/lab_room_pir_motion_sensor/state", false, true},
    }, 3
  },
  {
    "Main Room", {
      {"Light 1",  "light.main_room_room_light_1", "homeassistant/light/main_room_room_light_1/set", "homeassistant/light/main_room_room_light_1/state", false, false},
      {"Light 2",  "light.main_room_room_light_2", "homeassistant/light/main_room_room_light_2/set", "homeassistant/light/main_room_room_light_2/state", false, false},
      {"Light 3",  "light.main_room_room_light_3", "homeassistant/light/main_room_room_light_3/set", "homeassistant/light/main_room_room_light_3/state", false, false},
      {"Door Lock","switch.main_room_door_lock",   "homeassistant/switch/main_room_door_lock/set",   "homeassistant/switch/main_room_door_lock/state",   false, false},
    }, 4
  }
};

const int MENU_COUNT = 2;

// ====================== STATE ======================
int  currentMenu    = 0;   // 0 = root, 1+ = inside menu
int  selectedItem   = 0;
int  rootSelected   = 0;
bool inSubmenu      = false;
bool needsRedraw    = true;

unsigned long lastButtonPress  = 0;
unsigned long lastMqttAttempt  = 0;
unsigned long lastStateRequest = 0;

// ====================== DISPLAY FUNCTIONS ======================
void drawHeader(const char* title) {
  // Dark glass header
  tft.fillRect(0, 0, 320, 40, HEADER_COLOR);
  tft.drawRect(0, 0, 320, 40, BORDER_COLOR);
  
  // Accent line
  tft.fillRect(0, 38, 320, 2, ACCENT_COLOR);
  
  tft.setTextColor(ACCENT_COLOR, HEADER_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 12);
  tft.print(title);

  // WiFi + MQTT status dots top right
  uint16_t wifiColor = WiFi.isConnected() ? ON_COLOR : OFF_COLOR;
  uint16_t mqttColor = mqtt.connected()   ? ON_COLOR : OFF_COLOR;
  tft.fillCircle(295, 12, 5, wifiColor);
  tft.fillCircle(308, 12, 5, mqttColor);
}

void drawRootMenu() {
  tft.fillScreen(BG_COLOR);
  drawHeader("Control Panel");

  for (int i = 0; i < MENU_COUNT; i++) {
    int y = 50 + i * 55;
    bool selected = (i == rootSelected);

    // Item background
    uint16_t bgColor = selected ? ITEM_SELECTED : ITEM_BG;
    tft.fillRoundRect(8, y, 304, 45, 6, bgColor);
    tft.drawRoundRect(8, y, 304, 45, 6, selected ? ACCENT_COLOR : BORDER_COLOR);

    // Arrow indicator
    if (selected) {
      tft.setTextColor(ACCENT_COLOR, bgColor);
      tft.setTextSize(2);
      tft.setCursor(15, y + 14);
      tft.print(">");
    }

    tft.setTextColor(selected ? TFT_WHITE : TEXT_DIM, bgColor);
    tft.setTextSize(2);
    tft.setCursor(35, y + 14);
    tft.print(menus[i].name);

    // Device count badge
    tft.setTextSize(1);
    tft.setTextColor(TEXT_DIM, bgColor);
    tft.setCursor(250, y + 17);
    tft.print(menus[i].deviceCount);
    tft.print(" dev");
  }

  // Footer
  tft.setTextColor(TEXT_DIM, BG_COLOR);
  tft.setTextSize(1);
  tft.setCursor(60, 225);
  tft.print("Rotate: Navigate  |  Press: Enter");
}

void drawSubmenu(int menuIdx) {
  tft.fillScreen(BG_COLOR);

  String header = String("< ") + menus[menuIdx].name;
  drawHeader(header.c_str());

  Menu& m = menus[menuIdx];

  for (int i = 0; i < m.deviceCount; i++) {
    int y = 48 + i * 46;
    bool selected = (i == selectedItem);
    bool state    = m.devices[i].state;
    bool isSensor = m.devices[i].is_sensor;

    uint16_t bgColor = selected ? ITEM_SELECTED : ITEM_BG;
    tft.fillRoundRect(8, y, 304, 38, 5, bgColor);
    tft.drawRoundRect(8, y, 304, 38, 5, selected ? ACCENT_COLOR : BORDER_COLOR);

    // Selection arrow
    if (selected) {
      tft.setTextColor(ACCENT_COLOR, bgColor);
      tft.setTextSize(2);
      tft.setCursor(14, y + 11);
      tft.print(">");
    }

    // Device name
    tft.setTextColor(selected ? TFT_WHITE : TEXT_DIM, bgColor);
    tft.setTextSize(2);
    tft.setCursor(34, y + 11);
    tft.print(m.devices[i].name);

    // State badge
    if (isSensor) {
      uint16_t sColor = state ? ON_COLOR : TEXT_DIM;
      tft.fillRoundRect(240, y + 9, 60, 20, 4, sColor);
      tft.setTextColor(TFT_BLACK, sColor);
      tft.setTextSize(1);
      tft.setCursor(248, y + 14);
      tft.print(state ? "MOTION" : "CLEAR");
    } else {
      uint16_t sColor = state ? ON_COLOR : OFF_COLOR;
      tft.fillRoundRect(252, y + 9, 48, 20, 4, sColor);
      tft.setTextColor(TFT_BLACK, sColor);
      tft.setTextSize(1);
      tft.setCursor(263, y + 14);
      tft.print(state ? " ON" : " OFF");
    }
  }

  // Footer
  tft.setTextColor(TEXT_DIM, BG_COLOR);
  tft.setTextSize(1);
  tft.setCursor(30, 225);
  tft.print("Rotate: Navigate  |  Press: Toggle/Back");
}

void drawStatusScreen(const char* line1, const char* line2, uint16_t color) {
  tft.fillScreen(BG_COLOR);
  tft.setTextColor(color, BG_COLOR);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.print(line1);
  tft.setTextColor(TEXT_DIM, BG_COLOR);
  tft.setTextSize(1);
  tft.setCursor(20, 130);
  tft.print(line2);
}

// ====================== MQTT ======================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String p = "";
  for (int i = 0; i < length; i++) p += (char)payload[i];
  p.toUpperCase();

  // Match topic to device and update state
  for (int m = 0; m < MENU_COUNT; m++) {
    for (int d = 0; d < menus[m].deviceCount; d++) {
      if (t == String(menus[m].devices[d].state_topic)) {
        bool newState = (p == "ON" || p == "TRUE" || p == "1");
        if (menus[m].devices[d].state != newState) {
          menus[m].devices[d].state = newState;
          needsRedraw = true;
        }
        return;
      }
    }
  }
}

void subscribeAll() {
  for (int m = 0; m < MENU_COUNT; m++) {
    for (int d = 0; d < menus[m].deviceCount; d++) {
      mqtt.subscribe(menus[m].devices[d].state_topic);
    }
  }
}

void requestAllStates() {
  // Request current state from HA for all devices
  for (int m = 0; m < MENU_COUNT; m++) {
    for (int d = 0; d < menus[m].deviceCount; d++) {
      // Publish empty retain request — HA will respond with retained state
      mqtt.publish(menus[m].devices[d].state_topic, "", false);
    }
  }
}

bool connectMQTT() {
  if (mqtt.connected()) return true;
  unsigned long now = millis();
  if (now - lastMqttAttempt < 5000) return false;
  lastMqttAttempt = now;

  if (mqtt.connect(MQTT_CLIENT)) {
    subscribeAll();
    needsRedraw = true;
    return true;
  }
  return false;
}

void toggleDevice(int menuIdx, int deviceIdx) {
  Device& dev = menus[menuIdx].devices[deviceIdx];
  if (dev.is_sensor) return;

  const char* payload = dev.state ? "OFF" : "ON";
  mqtt.publish(dev.cmd_topic, payload);
  dev.state = !dev.state;
  needsRedraw = true;
}

// ====================== WIFI ======================
void connectWiFi() {
  drawStatusScreen("Connecting", "to WiFi...", ACCENT_COLOR);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }
}

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);

  // Backlight on
  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  // Display init
  tft.init();
  tft.setRotation(1);  // Landscape
  tft.fillScreen(BG_COLOR);

  // Boot screen
  drawStatusScreen("Southern IoT", "Control Panel v1.0", ACCENT_COLOR);
  delay(1500);

  // Encoder
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  encoder.setCount(0);
  lastEncoderPos = 0;

  // Button
  pinMode(ENCODER_SW, INPUT_PULLUP);

  // WiFi
  connectWiFi();

  // MQTT
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  connectMQTT();

  needsRedraw = true;
}

// ====================== LOOP ======================
void loop() {
  // Keep MQTT alive
  if (WiFi.isConnected()) {
    connectMQTT();
    mqtt.loop();
  }

  // Encoder rotation
  long newPos = encoder.getCount() / 2;
  if (newPos != lastEncoderPos) {
    int diff = newPos - lastEncoderPos;
    lastEncoderPos = newPos;

    if (!inSubmenu) {
      rootSelected = constrain(rootSelected + (diff > 0 ? 1 : -1), 0, MENU_COUNT - 1);
    } else {
      int maxItems = menus[currentMenu].deviceCount - 1;
      selectedItem = constrain(selectedItem + (diff > 0 ? 1 : -1), 0, maxItems);
    }
    needsRedraw = true;
  }

  // Button press
  if (digitalRead(ENCODER_SW) == LOW) {
    unsigned long now = millis();
    if (now - lastButtonPress > 300) {
      lastButtonPress = now;

      if (!inSubmenu) {
        // Enter submenu
        currentMenu  = rootSelected;
        selectedItem = 0;
        inSubmenu    = true;
      } else {
        // If first item selected = back
        // Actually toggle or go back based on long press
        // Short press = toggle device
        // We use selectedItem == last item as back? No — add back via double press
        // Single press = toggle
        toggleDevice(currentMenu, selectedItem);
      }
      needsRedraw = true;
    }
  }

  // Double press to go back (press within 400ms twice)
  // Simplified: hold button ~1 second to go back
  if (digitalRead(ENCODER_SW) == LOW && inSubmenu) {
    unsigned long held = millis();
    while (digitalRead(ENCODER_SW) == LOW) {
      if (millis() - held > 1000) {
        inSubmenu    = false;
        rootSelected = currentMenu;
        needsRedraw  = true;
        delay(300);
        break;
      }
    }
  }

  // Redraw
  if (needsRedraw) {
    needsRedraw = false;
    if (!inSubmenu) {
      drawRootMenu();
    } else {
      drawSubmenu(currentMenu);
    }
  }

  // Request states every 30 seconds
  if (millis() - lastStateRequest > 30000) {
    lastStateRequest = millis();
    subscribeAll();
  }
}