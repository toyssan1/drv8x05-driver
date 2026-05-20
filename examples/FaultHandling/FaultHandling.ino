// Fault Handling example
//
// Shows two approaches to nFAULT detection:
//   1. Polling — check hasFault() in the main loop
//   2. Interrupt — attach an ISR to the nFAULT pin for immediate response
//
// When a fault occurs the chip pulls nFAULT low. The fault condition must
// be resolved (e.g. overcurrent clears after the motor stops) before
// clearFault() will allow the driver to run again.
//
// Wiring (DRV8706S-Q1):
//   GPIO 5  — nSCS
//   GPIO 18 — SCLK
//   GPIO 19 — SDO (MISO)
//   GPIO 23 — SDI (MOSI)
//   GPIO 14 — nSLEEP
//   GPIO 27 — DRVOFF
//   GPIO 26 — nFAULT  (must support interrupts on your board)
//   GPIO 25 — IN1 (PWM)
//   GPIO 33 — IN2 (direction)

#include <DRV8x06.h>

#define CS_PIN      5
#define NSLEEP_PIN  14
#define DRVOFF_PIN  27
#define NFAULT_PIN  26
#define IN1_PIN     25
#define IN2_PIN     33

DRV8x06 motor(CS_PIN, NSLEEP_PIN, DRVOFF_PIN, NFAULT_PIN, IN1_PIN, IN2_PIN);

volatile bool faultFlag = false;

void IRAM_ATTR onFault()
{
    // Called on nFAULT falling edge — keep this fast, no SPI here
    faultFlag = true;
}

void handleFault()
{
    Serial.println("*** FAULT detected ***");

    // Stop motor immediately
    motor.setSpeed(0);
    motor.setOutputEnable(false);

    // Read status registers to find out what triggered
    motor.readAllRegisters();
    Serial.print("IC_STATUS_1:  0x"); Serial.println(motor.regs.ic_status_1_reg, HEX);
    Serial.print("VGS_VDS_STAT: 0x"); Serial.println(motor.regs.vgs_vds_stat_reg, HEX);
    Serial.print("IC_STATUS_2:  0x"); Serial.println(motor.regs.ic_status_2_reg, HEX);

    // Wait for fault condition to clear (overcurrent, thermal, etc.)
    delay(500);

    motor.clearFault();
    Serial.println("Fault cleared — resuming");
    motor.setOutputEnable(true);

    faultFlag = false;
}

void setup()
{
    Serial.begin(115200);

    motor.begin(DRV8706S_Q1);
    motor.setPWMMode(PWM_MODE_PH_EN);
    motor.setOutputEnable(true);

    // Attach interrupt — trigger on falling edge (nFAULT goes LOW on fault)
    attachInterrupt(digitalPinToInterrupt(NFAULT_PIN), onFault, FALLING);

    Serial.println("DRV8706S-Q1 ready — fault ISR active");
}

void loop()
{
    // Handle fault before doing anything else
    if (faultFlag) {
        handleFault();
        return;
    }

    // Normal motor ramp — forward
    Serial.println("Forward 0->80%");
    for (int s = 0; s <= 80; s += 4) {
        if (faultFlag) return;
        motor.setSpeed(s);
        delay(25);
    }
    delay(1000);

    // Reverse
    Serial.println("Reverse 0->-80%");
    for (int s = 0; s >= -80; s -= 4) {
        if (faultFlag) return;
        motor.setSpeed(s);
        delay(25);
    }
    delay(1000);

    // Stop
    motor.setSpeed(0);
    delay(500);

    // Polling fallback: check even when no interrupt fires
    // (useful if nFAULT is not on an interrupt-capable pin)
    if (motor.hasFault()) {
        handleFault();
    }
}
