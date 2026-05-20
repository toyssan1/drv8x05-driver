#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "DRV8x06_RegMap.h"

// Device IDs — matches TI EVM firmware deviceID numbering
enum DRV8x06_DeviceID : uint8_t {
    DRV8x06_UNKNOWN = 0,
    DRV8706S_Q1     = 1,   // Full-bridge, S variant (SPI)
    DRV8706H_Q1     = 2,   // Full-bridge, H variant (no SPI)
    DRV8106S_Q1     = 3,   // Half-bridge, S variant (SPI)
    DRV8106H_Q1     = 4,   // Half-bridge, H variant (no SPI)
    DRV8705S_Q1     = 5,   // Full-bridge, S variant (SPI)
    DRV8705H_Q1     = 6,   // Full-bridge, H variant (no SPI)
    DRV8718S_Q1     = 7,   // Dual full-bridge, S variant (SPI) — DRV871x family
    DRV8714S_Q1     = 8,   // Dual full-bridge, S variant (SPI) — DRV871x family
    DRV8714H_Q1     = 9,   // Dual full-bridge, H variant (no SPI) — DRV871x family
};

// Bridge control modes (BRG_CTRL bits[6:5] for DRV870x, IC_CTRL1 bits[5:4] for DRV871x)
enum DRV8x06_PWMMode : uint8_t {
    PWM_MODE_INDEPENDENT_HB = 0,  // Each half-bridge driven independently
    PWM_MODE_PH_EN          = 1,  // Phase/Enable full-bridge
    PWM_MODE_PWM_FB         = 2,  // PWM full-bridge (complementary)
};

// Use NO_PIN to indicate an optional pin is not wired
static const int8_t DRV8x06_NO_PIN = -1;

class DRV8x06 {
public:
    // csPin      — SPI chip select (active low)
    // nSleepPin  — nSLEEP (LOW = sleep, HIGH = active)
    // drvOffPin  — DRVOFF (HIGH = outputs disabled). Pass DRV8x06_NO_PIN if not wired.
    // nFaultPin  — nFAULT input. Pass DRV8x06_NO_PIN if not wired.
    // in1Pin     — IN1/EN1 PWM output
    // in2Pin     — IN2/PH GPIO (PH/EN mode) or PWM output (independent mode)
    // in3Pin     — IN3/EN3, DRV871x only. Pass DRV8x06_NO_PIN otherwise.
    // in4Pin     — IN4/PH2, DRV871x only. Pass DRV8x06_NO_PIN otherwise.
    // spi        — SPI bus. Defaults to the hardware SPI singleton.
    DRV8x06(uint8_t csPin,
            uint8_t nSleepPin,
            int8_t  drvOffPin,
            int8_t  nFaultPin,
            uint8_t in1Pin,
            uint8_t in2Pin,
            int8_t  in3Pin = DRV8x06_NO_PIN,
            int8_t  in4Pin = DRV8x06_NO_PIN,
            SPIClass &spi  = SPI);

    // Call in setup(). deviceID must be supplied — no auto-detect without the EVM ID resistors.
    // Wakes the device, initialises registers with TI firmware defaults (S variants only).
    // sclkPin/misoPin/mosiPin: pass -1 to use the SPI bus defaults (platform-defined).
    void begin(DRV8x06_DeviceID deviceID,
               int8_t sclkPin = -1,
               int8_t misoPin = -1,
               int8_t mosiPin = -1);

    // Sleep / wake
    void setSleep(bool sleep);
    bool isSleeping() const { return _sleeping; }

    // Output enable (DRVOFF pin)
    void setOutputEnable(bool enable);

    // nFAULT state — returns true when a fault is active (pin is LOW)
    bool hasFault() const;
    void clearFault();

    // Raw SPI register access (S variants only)
    uint8_t readRegister(uint8_t address);
    void    writeRegister(uint8_t address, uint8_t data);

    // Read all registers into the regs image
    void readAllRegisters();

    // Motor control — PH/EN full-bridge mode (setSpeed uses IN1=PWM, IN2=direction)
    // speed: -100 (full reverse) .. 0 (stop) .. +100 (full forward)
    void setSpeed(int16_t speed);

    // Independent half-bridge mode — set duty 0..100 per input pin
    void setHB1Duty(uint8_t duty);
    void setHB2Duty(uint8_t duty);
    void setHB3Duty(uint8_t duty);  // DRV871x only
    void setHB4Duty(uint8_t duty);  // DRV871x only

    // Read/write the PWM bridge mode register field
    DRV8x06_PWMMode getPWMMode();
    void            setPWMMode(DRV8x06_PWMMode mode);

    DRV8x06_DeviceID getDeviceID() const { return _deviceID; }
    bool             isDRV871x()   const { return _isDRV871x; }
    bool             isSVariant()  const { return _isSVariant; }

    // Mirror of all device registers, updated by readAllRegisters()
    DRV8x06_Q1_REG_t regs;

private:
    SPIClass         &_spi;
    uint8_t           _csPin, _nSleepPin, _in1Pin, _in2Pin;
    int8_t            _drvOffPin, _nFaultPin, _in3Pin, _in4Pin;
    int8_t            _sclkPin, _misoPin, _mosiPin;
    DRV8x06_DeviceID  _deviceID;
    bool              _isDRV871x, _isSVariant, _isDRV8718S, _sleeping;

    void    _transfer(uint8_t msb, uint8_t lsb, uint8_t *rxMsb, uint8_t *rxLsb);
    void    _regInit();
    void    _pwmWrite(uint8_t pin, uint8_t duty0to100);
};
