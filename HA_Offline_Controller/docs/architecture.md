# System Architecture

## Overview

The Office Control Panel is a standalone ESP32 device that acts as a physical user interface for Home Assistant. It communicates via MQTT and uses Node-RED as a bridge between Home Assistant and the ESP32.

## Full Stack Block Diagram

```
+---------------------------+
|      PHYSICAL LAYER       |
|  [ ESP32 Control Panel ]  |
|  - Rotary Encoder         |
|  - ILI9341 TFT Display    |
|  - USB-C Power            |
+----------+----------------+
           |
           v
+---------------------------+
|      FIRMWARE LAYER       |
|  [ Arduino ESP32 ]        |
|  - WiFi connection        |
|  - MQTT Pub/Sub           |
|  - TFT_eSPI display       |
|  - ESP32Encoder input     |
+----------+----------------+
           |
           v
+---------------------------+
|    NETWORK LAYER (WiFi)   |
|     802.11 b/g/n          |
|    192.168.0.0/24         |
+----------+----------------+
           |
           v
+---------------------------+
|    MQTT MESSAGE BROKER   |
|  [ Eclipse Mosquitto ]    |
|   Port: 1883              |
|  - subscribe /set topics  |
|  - publish /state topics  |
|  - retain=true, QoS=1     |
+----------+----------------+
           |
   +-------+-------+
   v               v
+---------------------------+
|    Node-RED Bridge        |
|  [ Node-RED Container ]   |
|                           |
|  HA --> MQTT:             |
|  server-state-changed -->|
|  mqtt-out                 |
|                           |
|  MQTT --> HA:             |
|  mqtt-in --> function --->|
|  api-call-service         |
+----------+----------------+
           |
           v
+---------------------------+
|   HOME AUTOMATION LAYER   |
|  [ Home Assistant ]       |
|  - MQTT integration       |
|  - Entity state mgmt      |
|  - Automations & scenes   |
|  - Frontend dashboard     |
+---------------------------+
```

## Layer Descriptions

### Physical Layer
The ESP32 DevKit v1 reads rotary encoder input and drives the ILI9341 TFT display. All firmware runs on the ESP32 with no external computation.

### Firmware Layer
Pure Arduino sketch (no ESPHome). Uses TFT_eSPI for display, PubSubClient for MQTT, and ESP32Encoder for encoder input. Handles UI state machine, MQTT pub/sub, and WiFi reconnection.

### Network Layer
ESP32 connects to the local WiFi network. All communication uses the local subnet — no cloud dependencies.

### MQTT Broker
Eclipse Mosquitto running as a Docker container. Acts as the central message bus. All devices publish state to `/state` topics and subscribe to `/set` topics for commands.

### Node-RED Bridge
Node-RED subscribes to Home Assistant WebSocket events and publishes MQTT messages. Also listens on MQTT `/set` topics and calls Home Assistant service APIs. Acts as a two-way translator.

### Home Assistant
Source of truth for all device states. Entities are exposed to Node-RED via `server-state-changed` nodes. Commands are sent back via `api-call-service` nodes.

## Data Flow

### Direction 1: ESP32 → Home Assistant (Command)

```
Encoder Press
    |
    v
ESP32 firmware detects toggle event
    |
    v
mqttClient.publish(cmdTopic, "ON"/"OFF")
    |
    v
Mosquitto broker receives message
    |
    v
Node-RED mqtt-in node receives
    |
    v
Node-RED function node builds service call
    |
    v
Node-RED api-call-service calls HA API
    |
    v
Home Assistant changes entity state
    |
    v
All HA clients (dashboard, mobile app) update
```

### Direction 2: Home Assistant → ESP32 (State Sync)

```
Home Assistant entity state changes
    |
    v
Node-RED server-state-changed node detects
    |
    v
Node-RED mqtt-out publishes to /state topic
    |
    v
Mosquitto broker retains message
    |
    v
ESP32 mqttClient callback receives message
    |
    v
ESP32 updates device state in memory
    |
    v
TFT display redraws with updated state badge
```

## Docker Container Network

```
+-------------------+     +-------------------+     +-------------------+
|  Home Assistant   |     |    Mosquitto      |     |     Node-RED      |
|   (Docker)        |     |    (Docker)       |     |    (Docker)       |
|                   |     |                   |     |                   |
| HA core runs here |     | MQTT broker       |     | Node-RED engine   |
| MQTT integration  |     | port: 1883        |     | HA websocket      |
| listens here      |     |                   |     | MQTT client       |
+-------------------+     +-------------------+     +-------------------+
        ^                         ^                        ^
        |                         |                        |
        +-------------------------+------------------------+
                    All containers on same Docker network
                    or host network (host mode)
```

## Why MQTT Over ESPHome Native API?

1. **No firmware dependency on HA version** — MQTT is a universal protocol. The ESP32 firmware works with any home automation platform that speaks MQTT.

2. **Separation of concerns** — The ESP32 only needs to know about MQTT topics. It doesn't need to understand Home Assistant's entity model, service calls, or API.

3. **Node-RED as the translator** — Node-RED handles all HA-specific logic. The ESP32 firmware stays simple.

4. **Scalability** — Adding new devices or platforms only requires updating Node-RED flows, not the ESP32 firmware.

5. **Debugging** — MQTT topics can be tested independently with `mosquitto_pub` and `mosquitto_sub`. No need to recompile firmware to trace issues.

6. **Retained messages** — MQTT retain=True ensures the ESP32 always has the last known state after a reboot, without querying HA directly.

7. **Offline resilience** — If HA is down, the ESP32 still has the last cached state and can toggle devices locally. The state will sync when HA comes back online.
