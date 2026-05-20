# DRV8x06 Arduino/ESP32 Library

Arduino and PlatformIO driver for the TI **DRV8x06-Q1** and **DRV871x-Q1** families of SPI-configurable H-bridge motor drivers.

Ported from TI's official MSP430 EVM firmware (v1.5). All SPI register defaults match the EVM firmware exactly.

---

## Supported Chips

| Device ID | Part Number    | Bridges | SPI | Notes                        |
|-----------|---------------|---------|-----|------------------------------|
| 1         | DRV8706S-Q1   | 1 full  | Yes |                              |
| 2         | DRV8706H-Q1   | 1 full  | No  | GPIO control only            |
| 3         | DRV8106S-Q1   | 1 half  | Yes | Single half-bridge           |
| 4         | DRV8106H-Q1   | 1 half  | No  | GPIO control only            |
| 5         | DRV8705S-Q1   | 1 full  | Yes |                              |
| 6         | DRV8705H-Q1   | 1 full  | No  | GPIO control only            |
| 7         | DRV8718S-Q1   | 2 full  | Yes | DRV871x family, dual bridge  |
| 8         | DRV8714S-Q1   | 2 full  | Yes | DRV871x family, dual bridge  |
| 9         | DRV8714H-Q1   | 2 full  | No  | DRV871x family, GPIO only    |

**S variants** have SPI configuration registers. **H variants** are pin-strapped only — `begin()` still manages nSLEEP/DRVOFF/IN pins, but no SPI register calls are made.

---

## Installation

### Arduino IDE (manual)

1. Download or clone this repository.
2. In the Arduino IDE: **Sketch → Include Library → Add .ZIP Library…**
3. Select the `DRV8x06-Arduino` folder (zip it first, or use the folder directly in Arduino 2.x).
4. The library will appear under **Sketch → Include Library → DRV8x06**.

### PlatformIO

**Option A — local path** (add to your project's `platformio.ini`):
```ini
lib_deps =
    file:///absolute/path/to/DRV8x06-Arduino
```

**Option B — copy into project** (no config needed):  
Copy the `DRV8x06-Arduino` folder into your project's `lib/` directory.

---

## Wiring

### Single full-bridge (DRV8706S-Q1)

| ESP32 GPIO | DRV8706S-Q1 Pin | Notes                        |
|-----------|-----------------|------------------------------|
| GPIO 5    | nSCS            | SPI chip select (active low) |
| GPIO 18   | SCLK            | SPI clock (hardware SPI)     |
| GPIO 19   | SDO             | MISO (hardware SPI)          |
| GPIO 23   | SDI             | MOSI (hardware SPI)          |
| GPIO 14   | nSLEEP          | Drive HIGH to wake           |
| GPIO 27   | DRVOFF          | Drive LOW to enable outputs  |
| GPIO 26   | nFAULT          | Input, active low            |
| GPIO 25   | IN1/EN          | PWM output                   |
| GPIO 33   | IN2/PH          | Direction GPIO               |

### Dual full-bridge (DRV8718S-Q1 / DRV8714S-Q1)

Wire as above, plus:

| ESP32 GPIO | DRV871x Pin | Notes                    |
|-----------|-------------|--------------------------|
| GPIO 32   | IN3/EN      | Bridge 2 PWM output      |
| GPIO 35   | IN4/PH      | Bridge 2 direction GPIO  |

> **Note:** SPI pins (nSCS, SCLK, SDO, SDI) are shared across all devices on the bus. Each device needs its own nSCS pin.

---

## Quick Start

```cpp
#include <DRV8x06.h>

DRV8x06 motor(
    5,   // csPin
    14,  // nSleepPin
    27,  // drvOffPin  (-1 if not wired)
    26,  // nFaultPin  (-1 if not wired)
    25,  // in1Pin (PWM)
    33   // in2Pin (direction)
);

void setup() {
    motor.begin(DRV8706S_Q1);       // specify your device
    motor.setPWMMode(PWM_MODE_PH_EN);
    motor.setOutputEnable(true);
}

void loop() {
    motor.setSpeed(75);   // 75% forward
    delay(2000);
    motor.setSpeed(-50);  // 50% reverse
    delay(2000);
    motor.setSpeed(0);
    delay(1000);
}
```

---

## API Reference

### Constructor

```cpp
DRV8x06(uint8_t csPin,
        uint8_t nSleepPin,
        int8_t  drvOffPin,   // pass -1 if not wired
        int8_t  nFaultPin,   // pass -1 if not wired
        uint8_t in1Pin,
        uint8_t in2Pin,
        int8_t  in3Pin = -1, // DRV871x only, else -1
        int8_t  in4Pin = -1, // DRV871x only, else -1
        SPIClass &spi = SPI);
```

### Initialization

```cpp
void begin(DRV8x06_DeviceID deviceID);
```

Configures GPIO, starts SPI, wakes the device, and writes TI firmware default register values (S variants only). Must be called in `setup()`.

### Sleep / Enable

```cpp
void setSleep(bool sleep);      // true = sleep mode
bool isSleeping() const;

void setOutputEnable(bool enable);  // true = outputs active (DRVOFF low)
```

### Motor Control

```cpp
// PH/EN full-bridge mode: speed -100 (full reverse) to +100 (full forward)
void setSpeed(int16_t speed);

// Independent half-bridge: duty 0–100 per input
void setHB1Duty(uint8_t duty);
void setHB2Duty(uint8_t duty);
void setHB3Duty(uint8_t duty);  // DRV871x only
void setHB4Duty(uint8_t duty);  // DRV871x only
```

### Bridge Mode

```cpp
void            setPWMMode(DRV8x06_PWMMode mode);
DRV8x06_PWMMode getPWMMode();
```

Modes:

| Value                   | Meaning                                     |
|------------------------|---------------------------------------------|
| `PWM_MODE_INDEPENDENT_HB` | Each half-bridge driven by its own PWM   |
| `PWM_MODE_PH_EN`          | PH/EN full-bridge (recommended default)  |
| `PWM_MODE_PWM_FB`         | Complementary PWM full-bridge            |

### Fault Handling

```cpp
bool hasFault() const;  // true when nFAULT pin is LOW
void clearFault();      // writes CLR_FLT bit, re-reads register
```

### Register Access (S variants)

```cpp
uint8_t readRegister(uint8_t address);
void    writeRegister(uint8_t address, uint8_t data);
void    readAllRegisters();   // refreshes the regs struct

DRV8x06_Q1_REG_t regs;       // public register image
```

All register addresses are defined in `DRV8x06_RegMap.h` (e.g. `IC_CTRL`, `BRG_CTRL`, `IC_CTRL1_DRV871XX`).

### Device Info

```cpp
DRV8x06_DeviceID getDeviceID() const;
bool             isDRV871x()   const;  // true for DRV8714/8718
bool             isSVariant()  const;  // true if SPI registers available
```

---

## SPI Protocol Details

- **Mode:** CPOL=0, CPHA=1 (SPI MODE1)
- **Bit order:** MSB first
- **Clock:** 2 MHz default
- **Frame:** 16-bit  

```
Bit 15  : reserved (0)
Bit 14  : R/W — 1=read, 0=write
Bits 13:8 : 6-bit register address
Bits 7:0  : 8-bit data (write) / 8-bit response in 2nd received byte (read)
```

---

## PWM Frequency (ESP32)

`setSpeed()` and `setHBxDuty()` use Arduino's `analogWrite()`. On ESP32 the default PWM frequency is ~1 kHz. To match the TI EVM's 20 kHz:

**ESP32 Arduino core 3.x:**
```cpp
analogWriteFrequency(IN1_PIN, 20000);
// call before motor.begin()
```

**ESP32 Arduino core 2.x (manual LEDC):**
```cpp
ledcSetup(0, 20000, 8);       // channel 0, 20 kHz, 8-bit
ledcAttachPin(IN1_PIN, 0);
// then use ledcWrite(0, duty_0_255) instead of motor.setSpeed()
```

---

## Examples

| Example | Description |
|---------|-------------|
| [BasicMotorControl](examples/BasicMotorControl/BasicMotorControl.ino) | PH/EN ramp forward/reverse, DRV8706S-Q1 |
| [IndependentHalfBridge](examples/IndependentHalfBridge/IndependentHalfBridge.ino) | Independent PWM per half-bridge, DRV8106S-Q1 |
| [FaultHandling](examples/FaultHandling/FaultHandling.ino) | ISR-based nFAULT detection and recovery |
| [DualMotor](examples/DualMotor/DualMotor.ino) | Two motors on DRV8718S-Q1 dual bridge |

---

## License

BSD 3-Clause. Register map and default values derived from TI DRV87xx_DRV8106-Q1EVM Firmware v1.5 (TI BSD license).
# drv8x05-driver
