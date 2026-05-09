# UI Navigation Guide

## Root Menu Screen

```
+----------------------------------+
|  Office Panel v1.0    o o o     |  <- Status bar (WiFi, MQTT, reserved)
+----------------------------------+
|                                  |
|         Office Control          |  <- Header
|                                  |
|  +----------------------------+  |
|  | Lab Room              [3]  |  |  <- Device count badge
|  +----------------------------+  |
|                                  |
|  +----------------------------+  |
|  | Main Room             [4]  |  |  <- Selected (cyan border)
|  +----------------------------+  |
|                                  |
+----------------------------------+
```

## Lab Room Submenu

```
+----------------------------------+
|  Office Panel v1.0    o o o     |
+----------------------------------+
|            Lab Room             |
+----------------------------------+
|  +----------------------------+  |
|  | Light 1             [ ON ]|  |  <- Selected (cyan border)
|  +----------------------------+  |
|  | Light 2             [OFF] |  |
|  +----------------------------+  |
|  | PIR               [CLEAR]  |  |  <- Read-only badge
|  +----------------------------+  |
|  | ...                          |
+----------------------------------+
```

## Main Room Submenu

```
+----------------------------------+
|  Office Panel v1.0    o o o     |
+----------------------------------+
|           Main Room             |
+----------------------------------+
|  +----------------------------+  |
|  | Light 1             [ ON ]|  |
|  +----------------------------+  |
|  | Light 2             [OFF] |  |
|  +----------------------------+  |
|  | Light 3             [OFF] |  |
|  +----------------------------+  |
|  | Door Lock           [ ON ]|  |
|  +----------------------------+  |
+----------------------------------+
```

---

## Navigation Controls

| Action | Result | Screen Behavior |
|---|---|---|
| **Rotate CW** (clockwise) | Scroll down / next item | Highlights next item |
| **Rotate CCW** (counter-clockwise) | Scroll up / previous item | Highlights previous item |
| **Press** (short tap < 1s) | Enter submenu (root) / Toggle device (submenu) | Submenu: toggles ON↔OFF. PIR is read-only, press has no effect |
| **Hold 1 second** | Go back to parent menu | Returns from submenu to root, or shows boot screen |

---

## Status Bar Indicators

| Indicator | Position | Color Meaning |
|---|---|---|
| Dot 1 | Status bar, left | WiFi status: **Green** = connected, **Red** = disconnected |
| Dot 2 | Status bar, center | MQTT status: **Green** = connected, **Red** = disconnected |
| Dot 3 | Status bar, right | Reserved (currently same as MQTT) |

---

## Color Scheme Reference

| Element | Color | Hex (565) | RGB Approx |
|---|---|---|---|
| Background | Black | `0x0000` | `#000000` |
| Header bar | Dark Gray | `0x1082` | `#121212` |
| Selected item | Dark Teal | `0x02B5` | `#024545` |
| Accent (borders, text) | Cyan | `0x07FF` | `#00FFFF` |
| ON state badge | Green | `0x07E0` | `#00FF00` |
| OFF state badge | Red | `0xF800` | `#FF0000` |
| MOTION badge | Green | `0x07E0` | `#00FF00` |
| CLEAR badge | Red | `0xF800` | `#FF0000` |
| Normal text | White | `0xFFFF` | `#FFFFFF` |

---

## State Badge Descriptions

| Badge | Color | Meaning |
|---|---|---|
| `[ ON ]` | Green | Device is on / active |
| `[OFF]` | Red | Device is off / inactive |
| `[MOTION]` | Green | PIR motion detected |
| `[CLEAR]` | Red | PIR area is clear |

### Badge Dimensions

- Width: variable (text width + padding)
- Text size: 2 (16px equivalent)
- Positioned: right-aligned at x=230

---

## Device Count Badges (Root Menu)

On the root menu, each room shows a device count in the right column:

| Room | Count |
|---|---|
| Lab Room | 3 |
| Main Room | 4 |

---

## Boot Screen

```
+----------------------------------+
|                                  |
|                                  |
|         Southern IoT             |  <- Cyan, size 3
|                                  |
|       Control Panel v1.0        |  <- White, size 2
|                                  |
|        Initializing...           |  <- Cyan, size 1
|                                  |
+----------------------------------+
```

### Boot Sequence

1. Screen initializes (TFT_eSPI setup)
2. "Southern IoT" splash text displayed
3. "Control Panel v1.0" subtitle shown
4. "Connecting WiFi..." message appears
5. If WiFi succeeds: "WiFi OK!" (green text)
6. If WiFi fails: "WiFi FAILED" (red text)
7. MQTT connection attempted
8. Root menu displayed

---

## Backlight Control

- GPIO32 controls the TFT backlight
- Set to `HIGH` during boot screen initialization
- Display remains on continuously (no sleep mode in current firmware)
- Future: can add auto-dim based on ambient light sensor
