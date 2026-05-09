# Node-RED Flow Documentation

## Flow Overview

The Node-RED flow acts as a bidirectional bridge between Home Assistant and the MQTT broker. For each of the 7 devices, there are two data flow paths.

---

## Direction 1: Home Assistant → MQTT (State Broadcast)

When an entity state changes in Home Assistant, the flow publishes that state to the MQTT broker.

### Flow Path

```
server-state-changed node (HA entity)
        |
        v
  [msg.payload = "ON" or "OFF"]
        |
        v
mqtt-out node (publish to /state topic)
        |
        v
    MQTT Broker (retain=true, qos=1)
        |
        v
    ESP32 (subscribes and updates display)
```

### Example: Lab Light 1

**1. server-state-changed node**
- Entity: `light.lab_room_room_light_1`
- Type: exact
- Event: `CURRENT`
- Output: `toJSON`

**2. mqtt-out node**
- Topic: `homeassistant/light/lab_room_room_light_1/state`
- QoS: 1
- Retain: true

---

## Direction 2: MQTT → Home Assistant (Command Handling)

When the ESP32 publishes a command to the MQTT broker, the flow calls the corresponding Home Assistant service.

### Flow Path

```
mqtt-in node (subscribe to /set topic)
        |
        v
  [msg.payload = "ON" or "OFF"]
        |
        v
function node (build HA service call payload)
        |
        v
  [msg.payload = { domain, service, data }]
        |
        v
api-call-service node (call HA API)
        |
        v
  Home Assistant (changes entity state)
```

### Function Node Code for Light Devices

```javascript
var payload = msg.payload;
var data = {
    domain: "light",
    service: "turn_" + payload.toLowerCase(),
    data: {
        entity_id: "light.lab_room_room_light_1"
    }
};
msg.payload = data;
return msg;
```

### Function Node Code for Door Lock (Switch)

```javascript
var payload = msg.payload;
var data = {
    domain: "switch",
    service: "turn_" + payload.toLowerCase(),
    data: {
        entity_id: "switch.main_room_door_lock"
    }
};
msg.payload = data;
return msg;
```

---

## All 14 Node Pairs

### Lab Room

| Device | State Node (HA→MQTT) | Command Node (MQTT→HA) |
|---|---|---|
| Light 1 | server-state-changed → mqtt-out | mqtt-in → function → api-call-service |
| Light 2 | server-state-changed → mqtt-out | mqtt-in → function → api-call-service |
| PIR | server-state-changed → mqtt-out | (read-only, no command path) |

### Main Room

| Device | State Node (HA→MQTT) | Command Node (MQTT→HA) |
|---|---|---|
| Light 1 | server-state-changed → mqtt-out | mqtt-in → function → api-call-service |
| Light 2 | server-state-changed → mqtt-out | mqtt-in → function → api-call-service |
| Light 3 | server-state-changed → mqtt-out | mqtt-in → function → api-call-service |
| Door Lock | server-state-changed → mqtt-out | mqtt-in → function → api-call-service |

---

## Configuration Nodes

### MQTT Broker

- Host: `192.168.0.147`
- Port: `1883`
- Protocol: `MQTTv3.1.1`
- Client ID: `nodered_mqtt_lab`
- Keepalive: `60`
- Cleansession: `true`

### Home Assistant Server

- Host: `192.168.0.147`
- Port: `8123`
- API URL: `http://192.168.0.147:8123/api/`

---

## Import Instructions

1. Open Node-RED (`http://localhost:1880`)
2. Click the hamburger menu → Import → Clipboard
3. Paste the contents of `flow.json`
4. Click "Import"
5. Deploy

Before deploying, verify the configuration nodes (MQTT broker and HA server) have the correct IP addresses.

---

## Troubleshooting

### HA Server node not connecting
1. Verify Home Assistant is running at `http://192.168.0.147:8123`
2. Check the Long-Lived Access Token is valid
3. Verify `node-red-contrib-home-assistant-websocket` is installed

### MQTT nodes show disconnected
1. Verify Mosquitto Docker container is running: `docker ps`
2. Check broker IP and port in the MQTT config node
3. Test with `mosquitto_pub/sub` from the Node-RED host

### State not syncing
1. Check that the entity ID in `server-state-changed` matches the HA entity ID exactly
2. Verify MQTT retain is set to true on mqtt-out nodes
3. Use `mosquitto_sub -h 192.168.0.147 -t "#" -v` to see live messages

### ESP32 not receiving state
1. Check that all `/state` subscriptions are active on the ESP32
2. Verify `resubscribeAll()` is called on MQTT reconnect
3. Use `mosquitto_sub` to confirm messages are being published
