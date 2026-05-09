# Hardware Schematics

## Display Wiring (ILI9341)

| ILI9341 Pin | ESP32 GPIO | Notes |
|---|---|---|
| VCC | 3.3V | Do NOT connect to 5V |
| GND | GND | Ground reference |
| CS | GPIO15 | TFT chip select |
| RST | GPIO4 | Display reset |
| RS/DC | GPIO33 | Register select (data/command) |
| MOSI | GPIO13 | SPI data out (master out) |
| SCK | GPIO14 | SPI clock |
| LED | GPIO32 | Backlight control (PWM capable) |
| MISO | GPIO12 | SPI data in (not used but connected) |

## Rotary Encoder Wiring (KY-040)

| KY-040 Pin | ESP32 GPIO | Notes |
|---|---|---|
| VCC | 3.3V | Do NOT connect to 5V |
| GND | GND | Ground reference |
| CLK | GPIO18 | Encoder clock (A phase) |
| DT | GPIO19 | Encoder data (B phase) |
| SW | GPIO5 | Encoder switch (button) |

## Power Connections

| Component | Power Source |
|---|---|
| ESP32 DevKit v1 | USB-C 5V (or VIN 5V) |
| ILI9341 VCC | ESP32 3.3V pin |
| KY-040 VCC | ESP32 3.3V pin |

## GPIO Notes

### GPIO32 (Backlight Control)
- Connected to TFT LED pin
- Set to `HIGH` for full brightness
- Can be used for PWM dimming in future firmware versions

### GPIO33 (DC/RS)
- Used as data/command register select pin
- Chosen to avoid GPIO2 boot strapping issue on ESP32 v1
- Standard GPIO, no special constraints

### GPIO15 (CS)
- Standard GPIO with no boot strapping constraints
- Confirmed safe for TFT chip select

### GPIO12 (MISO)
- Connected for completeness
- The ILI9341 display does not send data back to ESP32
- Required by TFT_eSPI SPI bus initialization

## KiCad Schematic

A KiCad schematic file (`.kicad_sch`) will be added here to provide a professional schematic for PCB design and documentation.

The schematic will include:
- ESP32 DevKit v1 footprint with all GPIO pin assignments
- ILI9341 display module with SPI bus labeling
- KY-040 rotary encoder with signal labeling
- Power distribution with 3.3V and GND rails
- Optional: pull-up resistors on encoder lines

---

*To be added: KiCad schematic file*
