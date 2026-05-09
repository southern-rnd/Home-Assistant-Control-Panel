#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <SPI.h>

// ====== SECRETS - REPLACE WITH YOUR VALUES ======
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_BROKER   = "192.168.0.147";
const int   MQTT_PORT     = 1883;
const char* MQTT_CLIENT_ID = "control_panel";
// ===============================================

#define ENCODER_CLK 18
#define ENCODER_DT  19
#define ENCODER_SW   5
#define TFT_BL_PIN  32

#define PRESSED   0
#define NOT_PRESSED 1

TFT_eSPI tft = TFT_eSPI();
ESP32Encoder encoder;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

struct Device {
  const char* name;
  const char* cmdTopic;
  const char* stateTopic;
  bool state;
  bool readonly;
};

Device labDevices[] = {
  {"Light 1", "homeassistant/light/lab_room_room_light_1/set", "homeassistant/light/lab_room_room_light_1/state", false, false},
  {"Light 2", "homeassistant/light/lab_room_room_light_2/set", "homeassistant/light/lab_room_room_light_2/state", false, false},
  {"PIR", "", "homeassistant/binary_sensor/lab_room_pir_motion_sensor/state", false, true}
};

Device mainDevices[] = {
  {"Light 1", "homeassistant/light/main_room_room_light_1/set", "homeassistant/light/main_room_room_light_1/state", false, false},
  {"Light 2", "homeassistant/light/main_room_room_light_2/set", "homeassistant/light/main_room_room_light_2/state", false, false},
  {"Light 3", "homeassistant/light/main_room_room_light_3/set", "homeassistant/light/main_room_room_light_3/state", false, false},
  {"Door Lock", "homeassistant/switch/main_room_door_lock/set", "homeassistant/switch/main_room_door_lock/state", false, false}
};

enum Screen { BOOT, ROOT, LAB_SUB, MAIN_SUB };
Screen currentScreen = BOOT;

int8_t selectedIndex = 0;
int8_t scrollOffset = 0;
const int8_t VISIBLE_ITEMS = 5;
unsigned long lastButtonPress = 0;
bool buttonHeld = false;
unsigned long holdStartTime = 0;
bool inSubmenu = false;

bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastMqttReconnect = 0;
unsigned long lastResubscribe = 0;

const uint16_t COLOR_BG       = TFT_BLACK;
const uint16_t COLOR_HEADER   = 0x1082;
const uint16_t COLOR_SELECTED = 0x02B5;
const uint16_t COLOR_ACCENT   = 0x07FF;
const uint16_t COLOR_ON       = 0x07E0;
const uint16_t COLOR_OFF      = 0xF800;
const uint16_t COLOR_BORDER   = 0x2945;
const uint16_t COLOR_TEXT     = 0xFFFF;

void drawStatusBar() {
  tft.fillRect(0, 0, 240, 20, COLOR_HEADER);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_ACCENT, COLOR_HEADER);
  tft.setTextSize(1);
  tft.drawString("Office Panel v1.0", 5, 3);
  uint16_t dotColor = wifiConnected ? COLOR_ON : COLOR_OFF;
  tft.fillCircle(200, 10, 4, dotColor);
  dotColor = mqttConnected ? COLOR_ON : COLOR_OFF;
  tft.fillCircle(215, 10, 4, dotColor);
  tft.fillCircle(230, 10, 4, dotColor);
}

void drawHeader(const char* title) {
  tft.fillRect(0, 20, 240, 25, COLOR_HEADER);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.setTextSize(2);
  tft.drawString(title, 120, 23);
}

void drawDeviceRow(int index, const char* name, bool state, bool readonly, bool selected, int y) {
  uint16_t bg = selected ? COLOR_SELECTED : COLOR_BG;
  tft.fillRect(0, y, 240, 30, bg);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(COLOR_TEXT, bg);
  tft.setTextSize(2);
  tft.drawString(name, 10, y + 15);
  const char* badge;
  uint16_t badgeColor;
  if (readonly) {
    if (state) {
      badge = "MOTION";
      badgeColor = COLOR_ON;
    } else {
      badge = "CLEAR";
      badgeColor = COLOR_OFF;
    }
  } else {
    if (state) {
      badge = "ON ";
      badgeColor = COLOR_ON;
    } else {
      badge = "OFF";
      badgeColor = COLOR_OFF;
    }
  }
  tft.setTextColor(badgeColor, bg);
  tft.setTextDatum(MR_DATUM);
  tft.drawString(badge, 230, y + 15);
  if (selected) {
    tft.drawRect(2, y + 2, 236, 26, COLOR_ACCENT);
  }
}

void drawRootMenu() {
  tft.fillScreen(COLOR_BG);
  drawStatusBar();
  drawHeader("Office Control");
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(2);
  int y1 = 70;
  int y2 = 115;
  bool sel1 = (selectedIndex == 0);
  bool sel2 = (selectedIndex == 1);
  uint16_t bg1 = sel1 ? COLOR_SELECTED : COLOR_BG;
  uint16_t bg2 = sel2 ? COLOR_SELECTED : COLOR_BG;
  tft.fillRect(0, y1, 240, 40, bg1);
  tft.fillRect(0, y2, 240, 40, bg2);
  tft.setTextColor(COLOR_TEXT, bg1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Lab Room", 10, y1 + 20);
  tft.setTextColor(COLOR_ACCENT, bg1);
  tft.setTextDatum(MR_DATUM);
  tft.drawString("3", 220, y1 + 20);
  tft.setTextColor(COLOR_TEXT, bg2);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("Main Room", 10, y2 + 20);
  tft.setTextColor(COLOR_ACCENT, bg2);
  tft.setTextDatum(MR_DATUM);
  tft.drawString("4", 220, y2 + 20);
  if (sel1) tft.drawRect(2, y1 + 2, 236, 36, COLOR_ACCENT);
  if (sel2) tft.drawRect(2, y2 + 2, 236, 36, COLOR_ACCENT);
}

void drawSubmenu(const char* title, Device* devices, int count) {
  tft.fillScreen(COLOR_BG);
  drawStatusBar();
  drawHeader(title);
  int8_t startIdx = scrollOffset;
  int8_t endIdx = min(scrollOffset + VISIBLE_ITEMS, count);
  for (int i = startIdx; i < endIdx; i++) {
    int row = i - scrollOffset;
    bool isSelected = (i == selectedIndex);
    drawDeviceRow(i, devices[i].name, devices[i].state, devices[i].readonly, isSelected, 50 + row * 30);
  }
}

void drawBootScreen() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_ACCENT, COLOR_BG);
  tft.setTextSize(3);
  tft.drawString("Southern IoT", 120, 80);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.drawString("Control Panel v1.0", 120, 120);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_ACCENT, COLOR_BG);
  tft.drawString("Initializing...", 120, 160);
  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);
}

void connectWiFi() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(1);
  tft.drawString("Connecting WiFi...", 120, 175);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(250);
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    tft.drawString("WiFi OK!", 120, 175);
    delay(300);
  } else {
    wifiConnected = false;
    tft.setTextColor(COLOR_OFF, COLOR_BG);
    tft.drawString("WiFi FAILED", 120, 175);
    delay(1000);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topicStr = String(topic);
  String msg = String((char*)payload);
  for (int i = 0; i < 3; i++) {
    if (topicStr == labDevices[i].stateTopic) {
      labDevices[i].state = (msg.indexOf("ON") >= 0 || msg.indexOf("1") >= 0);
      break;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (topicStr == mainDevices[i].stateTopic) {
      mainDevices[i].state = (msg.indexOf("ON") >= 0 || msg.indexOf("1") >= 0);
      break;
    }
  }
}

void connectMQTT() {
  if (!wifiConnected) return;
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 5) {
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      mqttConnected = true;
      resubscribeAll();
    } else {
      attempts++;
      delay(500);
    }
  }
}

void resubscribeAll() {
  mqttClient.unsubscribe("homeassistant/light/lab_room_room_light_1/state");
  mqttClient.unsubscribe("homeassistant/light/lab_room_room_light_2/state");
  mqttClient.unsubscribe("homeassistant/binary_sensor/lab_room_pir_motion_sensor/state");
  mqttClient.unsubscribe("homeassistant/light/main_room_room_light_1/state");
  mqttClient.unsubscribe("homeassistant/light/main_room_room_light_2/state");
  mqttClient.unsubscribe("homeassistant/light/main_room_room_light_3/state");
  mqttClient.unsubscribe("homeassistant/switch/main_room_door_lock/state");
  delay(100);
  mqttClient.subscribe("homeassistant/light/lab_room_room_light_1/state", 1);
  mqttClient.subscribe("homeassistant/light/lab_room_room_light_2/state", 1);
  mqttClient.subscribe("homeassistant/binary_sensor/lab_room_pir_motion_sensor/state", 1);
  mqttClient.subscribe("homeassistant/light/main_room_room_light_1/state", 1);
  mqttClient.subscribe("homeassistant/light/main_room_room_light_2/state", 1);
  mqttClient.subscribe("homeassistant/light/main_room_room_light_3/state", 1);
  mqttClient.subscribe("homeassistant/switch/main_room_door_lock/state", 1);
}

void publishToggle(Device* device) {
  if (device->readonly) return;
  const char* cmd = device->state ? "OFF" : "ON";
  mqttClient.publish(device->cmdTopic, cmd, true);
  device->state = !device->state;
}

void navigateRoot(int dir) {
  selectedIndex += dir;
  if (selectedIndex < 0) selectedIndex = 1;
  if (selectedIndex > 1) selectedIndex = 0;
}

void navigateSubmenu(int dir, int count) {
  selectedIndex += dir;
  if (selectedIndex < 0) selectedIndex = count - 1;
  if (selectedIndex >= count) selectedIndex = 0;
  if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
  if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
}

void enterSubmenu() {
  inSubmenu = true;
  selectedIndex = 0;
  scrollOffset = 0;
  if (currentScreen == ROOT && selectedIndex == 0) {
    currentScreen = LAB_SUB;
  } else if (currentScreen == ROOT && selectedIndex == 1) {
    currentScreen = MAIN_SUB;
  }
}

void goBack() {
  inSubmenu = false;
  currentScreen = ROOT;
  selectedIndex = 0;
  scrollOffset = 0;
}

void handleSubmenuAction(Device* devices, int count) {
  if (devices[selectedIndex].readonly) return;
  publishToggle(&devices[selectedIndex]);
}

void drawCurrentScreen() {
  switch (currentScreen) {
    case BOOT:
      break;
    case ROOT:
      drawRootMenu();
      break;
    case LAB_SUB:
      drawSubmenu("Lab Room", labDevices, 3);
      break;
    case MAIN_SUB:
      drawSubmenu("Main Room", mainDevices, 4);
      break;
  }
}

void checkEncoder() {
  static int8_t lastEncoded = 0;
  int8_t currentEncoded = (digitalRead(ENCODER_DT) << 1) | digitalRead(ENCODER_CLK);
  int8_t sum = (lastEncoded << 2) | currentEncoded;
  if (sum == 0b0111 || sum == 0b1110 || sum == 0b1000 || sum == 0b0001) {
    int8_t dir = (sum == 0b0111 || sum == 0b1110 || sum == 0b1000 || sum == 0b0001) ? 1 : -1;
    if (currentScreen == ROOT) {
      navigateRoot(dir);
    } else {
      if (currentScreen == LAB_SUB) navigateSubmenu(dir, 3);
      else if (currentScreen == MAIN_SUB) navigateSubmenu(dir, 4);
    }
    drawCurrentScreen();
  }
  lastEncoded = currentEncoded;

  bool swState = digitalRead(ENCODER_SW);
  if (swState == PRESSED && !buttonHeld) {
    if (millis() - lastButtonPress > 50) {
      buttonHeld = true;
      holdStartTime = millis();
    }
  }
  if (swState == NOT_PRESSED && buttonHeld) {
    unsigned long holdDuration = millis() - holdStartTime;
    if (holdDuration >= 1000) {
      goBack();
    } else {
      if (inSubmenu) {
        if (currentScreen == LAB_SUB) handleSubmenuAction(labDevices, 3);
        else if (currentScreen == MAIN_SUB) handleSubmenuAction(mainDevices, 4);
      } else {
        enterSubmenu();
      }
    }
    buttonHeld = false;
    lastButtonPress = millis();
    drawCurrentScreen();
  }
}

void updateConnections() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    mqttConnected = false;
    WiFi.reconnect();
  } else {
    wifiConnected = true;
  }
  if (wifiConnected && !mqttClient.connected()) {
    mqttConnected = false;
    if (millis() - lastMqttReconnect > 5000) {
      connectMQTT();
      lastMqttReconnect = millis();
    }
  }
  if (mqttClient.connected()) {
    mqttConnected = true;
    mqttClient.loop();
  }
  if (millis() - lastResubscribe > 30000) {
    if (mqttClient.connected()) {
      resubscribeAll();
    }
    lastResubscribe = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  encoder.attachFullQuad(ENCODER_CLK, ENCODER_DT);
  encoder.clearCount();
  drawBootScreen();
  delay(500);
  connectWiFi();
  delay(300);
  connectMQTT();
  delay(300);
  currentScreen = ROOT;
  drawCurrentScreen();
}

void loop() {
  checkEncoder();
  updateConnections();
  static unsigned long lastRedraw = 0;
  if (millis() - lastRedraw > 2000) {
    drawCurrentScreen();
    lastRedraw = millis();
  }
}
