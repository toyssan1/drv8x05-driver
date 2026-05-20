// Dual Motor example — DRV8718S-Q1 / DRV8714S-Q1
//
// The DRV871x family contains two independent full H-bridges in one chip.
// Bridge 1 is controlled by IN1/IN2; bridge 2 by IN3/IN4.
// Both bridges share the same SPI bus and nSLEEP/DRVOFF/nFAULT pins.
//
// This example runs two motors simultaneously:
//   Motor A (bridge 1): ramps forward, then reverse
//   Motor B (bridge 2): ramps in the opposite direction
//
// Wiring (DRV8718S-Q1):
//   GPIO 5  — nSCS
//   GPIO 18 — SCLK
//   GPIO 19 — SDO (MISO)
//   GPIO 23 — SDI (MOSI)
//   GPIO 14 — nSLEEP
//   GPIO 27 — DRVOFF
//   GPIO 26 — nFAULT
//   GPIO 25 — IN1  (motor A PWM)
//   GPIO 33 — IN2  (motor A direction)
//   GPIO 32 — IN3  (motor B PWM)
//   GPIO 35 — IN4  (motor B direction)

#include <DRV8x06.h>

#define CS_PIN      5
#define NSLEEP_PIN  14
#define DRVOFF_PIN  27
#define NFAULT_PIN  26
#define IN1_PIN     25
#define IN2_PIN     33
#define IN3_PIN     32
#define IN4_PIN     35

DRV8x06 driver(
    CS_PIN, NSLEEP_PIN, DRVOFF_PIN, NFAULT_PIN,
    IN1_PIN, IN2_PIN,
    IN3_PIN, IN4_PIN   // bridge 2 pins — required for DRV871x
);

void setup()
{
    Serial.begin(115200);

    driver.begin(DRV8718S_Q1);
    // DRV8718S_Q1 has 8 gate drive channels.
    // begin() writes 0xEE to all IDRV_CTRL1-8 and 0xAA to VDS_CTRL1-4 by default.

    driver.setPWMMode(PWM_MODE_PH_EN);
    driver.setOutputEnable(true);

    Serial.println("DRV8718S-Q1 dual motor ready");
    Serial.print("PWM mode: "); Serial.println(driver.getPWMMode());
}

void setMotorA(int16_t speed) { driver.setSpeed(speed); }

// Bridge 2 PH/EN: IN3 = PWM, IN4 = direction GPIO
void setMotorB(int16_t speed)
{
    if (speed >  100) speed =  100;
    if (speed < -100) speed = -100;

    if (speed >= 0) {
        driver.setHB4Duty(100);      // IN4 high = forward
        driver.setHB3Duty((uint8_t)speed);
    } else {
        driver.setHB4Duty(0);        // IN4 low = reverse
        driver.setHB3Duty((uint8_t)(-speed));
    }
}

void loop()
{
    if (driver.hasFault()) {
        Serial.println("Fault! Stopping both motors.");
        setMotorA(0);
        setMotorB(0);
        driver.setOutputEnable(false);
        delay(200);
        driver.clearFault();
        driver.setOutputEnable(true);
        return;
    }

    // Ramp A forward, B reverse simultaneously
    Serial.println("A: forward | B: reverse");
    for (int s = 0; s <= 100; s += 2) {
        setMotorA(s);
        setMotorB(-s);
        delay(20);
    }
    delay(1000);

    // Ramp both to stop
    for (int s = 100; s >= 0; s -= 2) {
        setMotorA(s);
        setMotorB(-s);
        delay(20);
    }
    delay(500);

    // Ramp A reverse, B forward simultaneously
    Serial.println("A: reverse | B: forward");
    for (int s = 0; s <= 100; s += 2) {
        setMotorA(-s);
        setMotorB(s);
        delay(20);
    }
    delay(1000);

    // Coast to stop
    for (int s = 100; s >= 0; s -= 2) {
        setMotorA(-s);
        setMotorB(s);
        delay(20);
    }
    delay(500);

    // Print DRV871x-specific status registers
    driver.readAllRegisters();
    Serial.print("IC_STAT1:  0x"); Serial.println(driver.regs.ic_stat1_reg, HEX);
    Serial.print("VDS_STAT1: 0x"); Serial.println(driver.regs.vds_stat1_reg, HEX);
    Serial.print("VDS_STAT2: 0x"); Serial.println(driver.regs.vds_stat2_reg, HEX);
    Serial.print("IC_STAT2:  0x"); Serial.println(driver.regs.ic_stat2_reg,  HEX);
}
