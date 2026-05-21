/**
 * Motor Controller Test — DRV8705S-Q1 + ESP32-S3-WROOM-1
 *
 * Wiring (ESP32-S3 SPI2 / FSPI):
 *   GPIO 10  →  nSCS   (SPI chip select, active low)
 *   GPIO 12  →  SCLK   (SPI clock)
 *   GPIO 13  →  SDO    (MISO)
 *   GPIO 11  →  SDI    (MOSI)
 *   GPIO 14  →  nSLEEP (drive HIGH to wake)
 *   GPIO 15  →  DRVOFF (drive LOW to enable outputs) — tie to GND if not used
 *   GPIO 16  →  nFAULT (active-low fault input)      — tie to 3.3V if not used
 *   GPIO 17  →  IN1/EN (PWM speed)
 *   GPIO 18  →  IN2/PH (direction)
 *   GPIO 4   →  SO    (CSA analog output — connect DRV8705S SO pin here)
 *
 * Current sensing:
 *   SO voltage = VREF + (I × R_SENSE × CSA_GAIN)
 *   VREF = VCC/2 (provided by DRV8705S VREF_OUT pin, ~1.65V on 3.3V supply)
 *   Adjust R_SENSE_OHMS to match your sense resistor.
 *   Adjust CSA_GAIN to match CSA_CTRL register (reset default = 5 V/V).
 *
 * Serial commands (115200 baud):
 *   f  — forward at current speed
 *   r  — reverse at current speed
 *   s  — stop (coast)
 *   e  — toggle output enable (enable/disable bridge)
 *   +  — increase speed by 10 (max 100)
 *   -  — decrease speed by 10 (min 10)
 *   0-9 — set speed to N*10  (e.g. '7' → 70%)
 */

#include <Arduino.h>
#include <SPI.h>
#include <DRV8x06.h>

// ── Pin assignments ──────────────────────────────────────────────────────────
static constexpr uint8_t PIN_CS      = 10;
static constexpr uint8_t PIN_SCLK    = 13;
static constexpr uint8_t PIN_MISO    = 11;
static constexpr uint8_t PIN_MOSI    = 12;
static constexpr uint8_t PIN_NSLEEP  = 18;
static constexpr int8_t  PIN_DRVOFF  = -1;   // set to DRV8x06_NO_PIN if unconnected
static constexpr int8_t  PIN_NFAULT  = 8;   // set to DRV8x06_NO_PIN if unconnected
static constexpr uint8_t PIN_IN1     = 9;   // PWM / speed
static constexpr uint8_t PIN_IN2     = 3;   // direction
static constexpr uint8_t PIN_SO      = 4;   // CSA analog output (ADC input)

// ── Current sense constants ──────────────────────────────────────────────────
// SO = VREF + (I × R_SENSE × CSA_GAIN)
// DRV8705S VREF_OUT = VCC/2. CSA_CTRL reset default = gain 5 V/V.
// Adjust both to match your hardware.
static constexpr float R_SENSE_OHMS = 0.010f;   // sense resistor in ohms (e.g. 10 mΩ)
static constexpr float CSA_GAIN     = 5.0f;     // CSA gain V/V (reset default; 5,10,20,40)
static constexpr float VREF_MV      = 1650.0f;  // VREF_OUT in mV (VCC/2 on 3.3V supply)

// ── Driver instance ──────────────────────────────────────────────────────────
DRV8x06 motor(
    PIN_CS,
    PIN_NSLEEP,
    PIN_DRVOFF,
    PIN_NFAULT,
    PIN_IN1,
    PIN_IN2
);

// ── State ────────────────────────────────────────────────────────────────────
static int16_t  g_speed     = 75;    // 1–100 %
static int8_t   g_direction = 1;     // +1 = forward, -1 = reverse
static bool     g_running   = false;
static bool     g_enabled   = false;

// ── Current sensing ─────────────────────────────────────────────────────────
static float readCurrentAmps()
{
    // analogReadMilliVolts() applies the built-in ADC calibration (ESP32 Arduino 3.x)
    float so_mv  = (float)analogReadMilliVolts(PIN_SO);
    float diff   = so_mv - VREF_MV;               // signed: + = forward, - = reverse
    return diff / (R_SENSE_OHMS * CSA_GAIN * 1000.0f);  // mV → V handled by /1000
}

// ── Comms validation ─────────────────────────────────────────────────────────
// Called after begin(). _regInit() writes BRG_CTRL=0x20 and DRV_CTRL_1=0xEE
// then calls readAllRegisters(). Re-reading those registers fresh from the
// device proves both the write and read paths of the SPI bus are functional.
static bool validateComms()
{
    Serial.println("Validating SPI communication with DRV8705S...");

    uint8_t brgCtrl  = motor.readRegister(BRG_CTRL);
    uint8_t drvCtrl1 = motor.readRegister(DRV_CTRL_1);

    Serial.printf("  BRG_CTRL   (0x05): 0x%02X  (expect 0x20)\n", brgCtrl);
    Serial.printf("  DRV_CTRL_1 (0x06): 0x%02X  (expect 0xEE)\n", drvCtrl1);

    if (brgCtrl == 0xFF || drvCtrl1 == 0xFF) {
        Serial.println("  FAIL: all-ones response — MISO floating or nSCS not driven low.");
        return false;
    }
    if (brgCtrl != 0x20) {
        Serial.printf("  FAIL: BRG_CTRL mismatch (got 0x%02X, expected 0x20)\n", brgCtrl);
        return false;
    }
    if (drvCtrl1 != 0xEE) {
        Serial.printf("  FAIL: DRV_CTRL_1 mismatch (got 0x%02X, expected 0xEE)\n", drvCtrl1);
        return false;
    }

    Serial.println("  OK: SPI comms verified.");
    return true;
}

// ── Helpers ──────────────────────────────────────────────────────────────────
static void applySpeed()
{
    int16_t cmd = g_running ? (g_direction * g_speed) : 0;
    motor.setSpeed(cmd);
    Serial.printf("Speed: %d%%  dir: %s  running: %s\n",
                  g_speed,
                  g_direction > 0 ? "FWD" : "REV",
                  g_running ? "YES" : "NO");
}

static void printCurrentAmps()
{
    float amps = readCurrentAmps();
    Serial.printf("Current: %+.3f A\n", amps);
}

static void printStatus()
{
    Serial.println("────────────────────────────────");
    Serial.printf("Speed    : %d%%\n", g_speed);
    Serial.printf("Direction: %s\n", g_direction > 0 ? "FORWARD" : "REVERSE");
    Serial.printf("Running  : %s\n", g_running ? "YES" : "NO");
    Serial.printf("Enabled  : %s\n", g_enabled ? "YES" : "NO");
    Serial.printf("Fault    : %s\n", motor.hasFault() ? "FAULT!" : "OK");
    Serial.printf("Current  : %+.3f A\n", readCurrentAmps());
    Serial.println("────────────────────────────────");
    Serial.println("Commands: f=forward  r=reverse  s=stop  e=enable/disable");
    Serial.println("          +=spd+10   -=spd-10   0-9=set speed (N*10%)  ?=status");
}

// ── Setup ────────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("\nDRV8705S Motor Controller — ESP32-S3");

    // Set PWM to 20 kHz before begin() (Arduino-ESP32 3.x API)
    analogWriteFrequency(PIN_IN1, 20000);

    motor.begin(DRV8705S_Q1, PIN_SCLK, PIN_MISO, PIN_MOSI);

    if (!validateComms()) {
        Serial.println("\nFATAL: Cannot communicate with DRV8705S. Check wiring. Halting.");
        while (true) { delay(1000); }
    }

    motor.setPWMMode(PWM_MODE_PH_EN);

    // Enable bridge outputs
    motor.setOutputEnable(true);
    g_enabled = true;

    motor.setSpeed(0);

    analogReadResolution(12);  // ensure 12-bit ADC for current sensing

    printStatus();
}

// ── Loop ─────────────────────────────────────────────────────────────────────
static uint32_t g_lastCurrentPrint = 0;
static constexpr uint32_t CURRENT_PRINT_MS = 500;  // print current every 500 ms when running

void loop()
{
    // Periodic current reporting while motor is running
    if (g_running && g_enabled) {
        uint32_t now = millis();
        if (now - g_lastCurrentPrint >= CURRENT_PRINT_MS) {
            g_lastCurrentPrint = now;
            printCurrentAmps();
        }
    }

    // Check fault pin
    if (motor.hasFault()) {
        motor.setSpeed(0);
        g_running = false;
        Serial.println("!! FAULT detected — motor stopped. Send 'e' to clear and re-enable.");
        motor.clearFault();
        delay(1000);
        return;
    }

    if (!Serial.available()) {
        return;
    }

    char cmd = (char)Serial.read();

    switch (cmd) {
        case 'f':
        case 'F':
            g_direction = 1;
            g_running   = true;
            applySpeed();
            break;

        case 'r':
        case 'R':
            g_direction = -1;
            g_running   = true;
            applySpeed();
            break;

        case 's':
        case 'S':
            g_running = false;
            applySpeed();
            break;

        case 'e':
        case 'E':
            g_enabled = !g_enabled;
            motor.setOutputEnable(g_enabled);
            if (!g_enabled) {
                g_running = false;
                motor.setSpeed(0);
            }
            Serial.printf("Output %s\n", g_enabled ? "ENABLED" : "DISABLED");
            break;

        case '+':
            if (g_speed < 100) {
                g_speed = min((int16_t)100, (int16_t)(g_speed + 10));
                applySpeed();
            }
            break;

        case '-':
            if (g_speed > 10) {
                g_speed = max((int16_t)10, (int16_t)(g_speed - 10));
                applySpeed();
            }
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            g_speed = (cmd - '0') == 0 ? 10 : (int16_t)((cmd - '0') * 10);
            applySpeed();
            break;

        case '?':
            printStatus();
            break;

        default:
            break;
    }
}
