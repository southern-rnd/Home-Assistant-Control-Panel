# MQTT Topic Reference

## Lab Room

### Light 1

| Property | Value |
|---|---|
| **Direction** | ESP32 → HA (command) |
| **Topic** | `homeassistant/light/lab_room_room_light_1/set` |
| **Payload (ON)** | `ON` |
| **Payload (OFF)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_1/set -m "ON"
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_1/set -m "OFF"
```

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state) |
| **Topic** | `homeassistant/light/lab_room_room_light_1/state` |
| **Payload (ON)** | `ON` or `{"state":"ON"}` |
| **Payload (OFF)** | `OFF` or `{"state":"OFF"}` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_1/state -v
```

### Light 2

| Property | Value |
|---|---|
| **Direction** | ESP32 → HA (command) |
| **Topic** | `homeassistant/light/lab_room_room_light_2/set` |
| **Payload (ON)** | `ON` |
| **Payload (OFF)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_2/set -m "ON"
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_2/set -m "OFF"
```

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state) |
| **Topic** | `homeassistant/light/lab_room_room_light_2/state` |
| **Payload (ON)** | `ON` or `{"state":"ON"}` |
| **Payload (OFF)** | `OFF` or `{"state":"OFF"}` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_2/state -v
```

### PIR Motion Sensor (Read-Only)

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state read-only) |
| **Topic** | `homeassistant/binary_sensor/lab_room_pir_motion_sensor/state` |
| **Payload (motion)** | `ON` |
| **Payload (clear)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/binary_sensor/lab_room_pir_motion_sensor/state -v
```

---

## Main Room

### Light 1

| Property | Value |
|---|---|
| **Direction** | ESP32 → HA (command) |
| **Topic** | `homeassistant/light/main_room_room_light_1/set` |
| **Payload (ON)** | `ON` |
| **Payload (OFF)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_1/set -m "ON"
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_1/set -m "OFF"
```

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state) |
| **Topic** | `homeassistant/light/main_room_room_light_1/state` |
| **Payload (ON)** | `ON` or `{"state":"ON"}` |
| **Payload (OFF)** | `OFF` or `{"state":"OFF"}` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_1/state -v
```

### Light 2

| Property | Value |
|---|---|
| **Direction** | ESP32 → HA (command) |
| **Topic** | `homeassistant/light/main_room_room_light_2/set` |
| **Payload (ON)** | `ON` |
| **Payload (OFF)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_2/set -m "ON"
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_2/set -m "OFF"
```

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state) |
| **Topic** | `homeassistant/light/main_room_room_light_2/state` |
| **Payload (ON)** | `ON` or `{"state":"ON"}` |
| **Payload (OFF)** | `OFF` or `{"state":"OFF"}` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_2/state -v
```

### Light 3

| Property | Value |
|---|---|
| **Direction** | ESP32 → HA (command) |
| **Topic** | `homeassistant/light/main_room_room_light_3/set` |
| **Payload (ON)** | `ON` |
| **Payload (OFF)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_3/set -m "ON"
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_3/set -m "OFF"
```

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state) |
| **Topic** | `homeassistant/light/main_room_room_light_3/state` |
| **Payload (ON)** | `ON` or `{"state":"ON"}` |
| **Payload (OFF)** | `OFF` or `{"state":"OFF"}` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/light/main_room_room_light_3/state -v
```

### Door Lock

| Property | Value |
|---|---|
| **Direction** | ESP32 → HA (command) |
| **Topic** | `homeassistant/switch/main_room_door_lock/set` |
| **Payload (ON/Locked)** | `ON` |
| **Payload (OFF/Unlocked)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_pub -h 192.168.0.147 -t homeassistant/switch/main_room_door_lock/set -m "ON"
mosquitto_pub -h 192.168.0.147 -t homeassistant/switch/main_room_door_lock/set -m "OFF"
```

| Property | Value |
|---|---|
| **Direction** | HA → ESP32 (state) |
| **Topic** | `homeassistant/switch/main_room_door_lock/state` |
| **Payload (ON)** | `ON` |
| **Payload (OFF)** | `OFF` |
| **QoS** | 1 |
| **Retain** | true |

```
mosquitto_sub -h 192.168.0.147 -t homeassistant/switch/main_room_door_lock/state -v
```

---

## Summary Table

| Device | Room | Type | Set Topic | State Topic | Direction |
|---|---|---|---|---|---|
| Light 1 | Lab | light | `/set` | `/state` | bidirectional |
| Light 2 | Lab | light | `/set` | `/state` | bidirectional |
| PIR | Lab | binary_sensor | — | `/state` | HA → ESP32 only |
| Light 1 | Main | light | `/set` | `/state` | bidirectional |
| Light 2 | Main | light | `/set` | `/state` | bidirectional |
| Light 3 | Main | light | `/set` | `/state` | bidirectional |
| Door Lock | Main | switch | `/set` | `/state` | bidirectional |

## Test All Topics

```bash
mosquitto_sub -h 192.168.0.147 -t "#" -v
```

```bash
mosquitto_pub -h 192.168.0.147 -t homeassistant/light/lab_room_room_light_1/set -m "ON"
```
