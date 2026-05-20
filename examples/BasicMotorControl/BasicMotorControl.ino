// Basic motor control example for DRV8706S-Q1 on ESP32
// Wiring (adjust pin numbers for your board):
//
//   ESP32 GPIO  |  DRV8706S-Q1 pin
//   -----------   -----------------
//   GPIO 5      |  nSCS  (SPI chip select)
//   GPIO 18     |  SCLK  (SPI clock)    — hardware SPI
//   GPIO 19     |  SDO   (MISO)         — hardware SPI
//   GPIO 23     |  SDI   (MOSI)         — hardware SPI
//   GPIO 14     |  nSLEEP
//   GPIO 27     |  DRVOFF
//   GPIO 26     |  nFAULT
//   GPIO 25     |  IN1   (PWM)
//   GPIO 33     |  IN2   (PH direction)

#include <DRV8x06.h>

#ifndef CS_PIN
#define CS_PIN      5
#endif
#ifndef NSLEEP_PIN
#define NSLEEP_PIN  14
#endif
#ifndef DRVOFF_PIN
#define DRVOFF_PIN  27
#endif
#ifndef NFAULT_PIN
#define NFAULT_PIN  26
#endif
#ifndef IN1_PIN
#define IN1_PIN     25
#endif
#ifndef IN2_PIN
#define IN2_PIN     33
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

DRV8x06 motor(CS_PIN, NSLEEP_PIN, DRVOFF_PIN, NFAULT_PIN, IN1_PIN, IN2_PIN);

void setup()
{
    Serial.begin(115200);

    // On ESP32, set PWM frequency before begin() so analogWrite uses 20 kHz
    // analogWriteFrequency(IN1_PIN, 20000);  // requires ESP32 core >= 3.x

    motor.begin(DRV8706S_Q1, SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    motor.setPWMMode(PWM_MODE_PH_EN);
    motor.setOutputEnable(true);

    Serial.println("DRV8706S-Q1 ready");
}

void loop()
{
    if (motor.hasFault()) {
        Serial.println("Fault! Clearing.");
        motor.clearFault();
        delay(500);
        return;
    }

    // Ramp forward 0 -> 100 %
    Serial.println("Forward");
    for (int s = 0; s <= 100; s += 2) {
        motor.setSpeed(s);
        delay(20);
    }
    delay(1000);

    // Ramp to stop
    for (int s = 100; s >= 0; s -= 2) {
        motor.setSpeed(s);
        delay(20);
    }
    delay(500);

    // Ramp reverse 0 -> -100 %
    Serial.println("Reverse");
    for (int s = 0; s >= -100; s -= 2) {
        motor.setSpeed(s);
        delay(20);
    }
    delay(1000);

    // Ramp to stop
    for (int s = -100; s <= 0; s += 2) {
        motor.setSpeed(s);
        delay(20);
    }
    delay(500);

    // Dump status registers over Serial
    motor.readAllRegisters();
    Serial.print("IC_STATUS_1: 0x");
    Serial.println(motor.regs.ic_status_1_reg, HEX);
    Serial.print("VGS_VDS:     0x");
    Serial.println(motor.regs.vgs_vds_stat_reg, HEX);
}
