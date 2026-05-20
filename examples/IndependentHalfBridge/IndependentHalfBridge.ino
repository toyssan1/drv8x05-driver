// Independent Half-Bridge mode example
//
// Demonstrates direct per-half-bridge duty cycle control.
// Useful for:
//   - DRV8106S-Q1 (single half-bridge device)
//   - Driving two separate loads independently
//   - PWM dimmable loads (LEDs, solenoids, etc.)
//
// In this mode IN1 and IN2 each control one half-bridge switch independently,
// so each can be 0-100% duty with no implied direction relationship.
//
// Wiring (DRV8106S-Q1, single half-bridge):
//   ESP32 GPIO  |  DRV8106S-Q1 pin
//   -----------   -----------------
//   GPIO 5      |  nSCS
//   GPIO 18     |  SCLK
//   GPIO 19     |  SDO (MISO)
//   GPIO 23     |  SDI (MOSI)
//   GPIO 14     |  nSLEEP
//   GPIO 26     |  nFAULT
//   GPIO 25     |  IN1  (high-side PWM)
//
// For a DRV8706S-Q1 in independent mode, also wire:
//   GPIO 33     |  IN2  (second half-bridge PWM)

#include <DRV8x06.h>

#ifndef CS_PIN
#define CS_PIN      5
#endif
#ifndef NSLEEP_PIN
#define NSLEEP_PIN  14
#endif
#ifndef NFAULT_PIN
#define NFAULT_PIN  26
#endif
#ifndef IN1_PIN
#define IN1_PIN     25
#endif
#ifndef IN2_PIN
#define IN2_PIN     33  // not used for DRV8106S-Q1 (single HB), safe to leave unconnected
#endif
#ifndef SPI_SCK_PIN
#define SPI_SCK_PIN  18
#endif
#ifndef SPI_MISO_PIN
#define SPI_MISO_PIN 19
#endif
#ifndef SPI_MOSI_PIN
#define SPI_MOSI_PIN 23
#endif

DRV8x06 driver(
    CS_PIN,
    NSLEEP_PIN,
    -1,          // DRVOFF not wired — DRV8106S-Q1 has no DRVOFF pin
    NFAULT_PIN,
    IN1_PIN,
    IN2_PIN
);

void setup()
{
    Serial.begin(115200);

    driver.begin(DRV8106S_Q1, SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    // begin() sets BRG_CTRL = 0x00 (half-bridge mode) for DRV8106S automatically

    driver.setOutputEnable(true);
    Serial.println("DRV8106S-Q1 independent half-bridge ready");
}

void loop()
{
    if (driver.hasFault()) {
        Serial.println("Fault detected — clearing");
        driver.clearFault();
        delay(200);
        return;
    }

    // Ramp HB1 from 0 to 100 %
    Serial.println("HB1: 0 -> 100 %");
    for (int duty = 0; duty <= 100; duty += 5) {
        driver.setHB1Duty((uint8_t)duty);
        delay(50);
    }
    delay(500);

    // Ramp HB1 back down
    Serial.println("HB1: 100 -> 0 %");
    for (int duty = 100; duty >= 0; duty -= 5) {
        driver.setHB1Duty((uint8_t)duty);
        delay(50);
    }
    delay(500);

    // For a full-bridge device (DRV8706S-Q1 etc.) you can also drive HB2
    // driver.setHB2Duty(50);

    // Read back status
    driver.readAllRegisters();
    Serial.print("IC_STATUS_1: 0x");
    Serial.println(driver.regs.ic_status_1_reg, HEX);
    Serial.print("VGS_VDS:     0x");
    Serial.println(driver.regs.vgs_vds_stat_reg, HEX);

    delay(1000);
}
