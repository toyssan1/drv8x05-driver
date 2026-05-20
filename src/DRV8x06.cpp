#include "DRV8x06.h"

// SPI frame layout (16-bit, MSB first):
//   bit 15    : reserved (0)
//   bit 14    : R/W — 1 = read, 0 = write
//   bits 13:8 : 6-bit register address
//   bits 7:0  : 8-bit data (tx on write, rx on read during 2nd byte)
//
// On a read the register value arrives in the second received byte;
// the first received byte (during address phase) is don't-care.
//
// SPI mode: CPOL=0, CPHA=1 (MODE1), MSB first, 2 MHz
// Source: MSP430 USCI_B default = UCCKPL=0, UCCKPH=0
//   → in MSP430, UCCKPH=0 means data changes on leading edge (rising),
//     captured on trailing edge (falling) → SPI MODE1.

#define DRV8x06_SPI_FREQ    2000000UL
#define DRV8x06_SPI_MODE    SPI_MODE1

// MSB byte bit positions
#define SPI_RW_BIT      0x40u   // bit 6 of MSB byte = bit 14 of 16-bit frame
#define SPI_ADDR_MASK   0x3Fu   // bits 5:0 of MSB byte

// tWAKE: 1 ms is sufficient; datasheet says max 1 ms from nSLEEP rising
#define TWAKE_US        1250u

DRV8x06::DRV8x06(uint8_t csPin, uint8_t nSleepPin,
                 int8_t drvOffPin, int8_t nFaultPin,
                 uint8_t in1Pin, uint8_t in2Pin,
                 int8_t in3Pin, int8_t in4Pin,
                 SPIClass &spi)
    : _spi(spi),
      _csPin(csPin), _nSleepPin(nSleepPin),
      _drvOffPin(drvOffPin), _nFaultPin(nFaultPin),
      _in1Pin(in1Pin), _in2Pin(in2Pin),
      _in3Pin(in3Pin), _in4Pin(in4Pin),
      _sclkPin(-1), _misoPin(-1), _mosiPin(-1),
      _deviceID(DRV8x06_UNKNOWN),
      _isDRV871x(false), _isSVariant(false), _isDRV8718S(false),
      _sleeping(true)
{
    memset(&regs, 0, sizeof(regs));
}

void DRV8x06::begin(DRV8x06_DeviceID deviceID,
                    int8_t sclkPin, int8_t misoPin, int8_t mosiPin)
{
    _sclkPin = sclkPin;
    _misoPin = misoPin;
    _mosiPin = mosiPin;
    _deviceID = deviceID;

    switch (_deviceID) {
        case DRV8706S_Q1: case DRV8106S_Q1: case DRV8705S_Q1:
        case DRV8718S_Q1: case DRV8714S_Q1:
            _isSVariant = true;  break;
        default:
            _isSVariant = false; break;
    }
    _isDRV871x  = (_deviceID == DRV8718S_Q1 ||
                   _deviceID == DRV8714S_Q1 ||
                   _deviceID == DRV8714H_Q1);
    _isDRV8718S = (_deviceID == DRV8718S_Q1);

    // GPIO
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);

    pinMode(_nSleepPin, OUTPUT);
    digitalWrite(_nSleepPin, LOW);

    if (_drvOffPin >= 0) {
        pinMode(_drvOffPin, OUTPUT);
        digitalWrite(_drvOffPin, HIGH);  // outputs disabled until explicitly enabled
    }
    if (_nFaultPin >= 0)
        pinMode(_nFaultPin, INPUT);

    pinMode(_in1Pin, OUTPUT); digitalWrite(_in1Pin, LOW);
    pinMode(_in2Pin, OUTPUT); digitalWrite(_in2Pin, LOW);
    if (_in3Pin >= 0) { pinMode(_in3Pin, OUTPUT); digitalWrite(_in3Pin, LOW); }
    if (_in4Pin >= 0) { pinMode(_in4Pin, OUTPUT); digitalWrite(_in4Pin, LOW); }

    _spi.begin(_sclkPin, _misoPin, _mosiPin, -1);

    setSleep(false);

    if (_isSVariant)
        _regInit();
}

void DRV8x06::setSleep(bool sleep)
{
    _sleeping = sleep;
    digitalWrite(_nSleepPin, sleep ? LOW : HIGH);
    if (!sleep)
        delayMicroseconds(TWAKE_US);
}

void DRV8x06::setOutputEnable(bool enable)
{
    if (_drvOffPin >= 0)
        digitalWrite(_drvOffPin, enable ? LOW : HIGH);
}

bool DRV8x06::hasFault() const
{
    if (_nFaultPin < 0) return false;
    return digitalRead(_nFaultPin) == LOW;
}

void DRV8x06::clearFault()
{
    if (_isDRV871x) {
        regs.ic_ctrl1_reg |= CLR_FLT_MASK;
        writeRegister(IC_CTRL1_DRV871XX, regs.ic_ctrl1_reg);
        regs.ic_ctrl1_reg = readRegister(IC_CTRL1_DRV871XX);
    } else {
        regs.ic_ctrl_reg |= CLR_FLT_MASK;
        writeRegister(IC_CTRL, regs.ic_ctrl_reg);
        regs.ic_ctrl_reg = readRegister(IC_CTRL);
    }
}

void DRV8x06::_transfer(uint8_t msb, uint8_t lsb, uint8_t *rxMsb, uint8_t *rxLsb)
{
    SPISettings settings(DRV8x06_SPI_FREQ, MSBFIRST, DRV8x06_SPI_MODE);
    _spi.beginTransaction(settings);
    digitalWrite(_csPin, LOW);
    *rxMsb = _spi.transfer(msb);
    *rxLsb = _spi.transfer(lsb);
    digitalWrite(_csPin, HIGH);
    _spi.endTransaction();
    delayMicroseconds(1);
}

uint8_t DRV8x06::readRegister(uint8_t address)
{
    uint8_t rxMsb, rxLsb;
    _transfer(SPI_RW_BIT | (address & SPI_ADDR_MASK), 0x00, &rxMsb, &rxLsb);
    return rxLsb;  // data arrives in the second received byte
}

void DRV8x06::writeRegister(uint8_t address, uint8_t data)
{
    uint8_t rxMsb, rxLsb;
    _transfer(address & SPI_ADDR_MASK, data, &rxMsb, &rxLsb);
}

void DRV8x06::readAllRegisters()
{
    if (_isDRV871x) {
        regs.ic_stat1_reg    = readRegister(IC_STAT_1_DRV871XX);
        regs.vds_stat1_reg   = readRegister(VDS_STAT1_DRV871XX);
        regs.vds_stat2_reg   = readRegister(VDS_STAT2_DRV871XX);
        regs.vgs_stat1_reg   = readRegister(VGS_STAT1_DRV871XX);
        regs.vgs_stat2_reg   = readRegister(VGS_STAT2_DRV871XX);
        regs.ic_stat2_reg    = readRegister(IC_STAT2_DRV871XX);
        regs.ic_stat3_reg    = readRegister(IC_STAT3_DRV871XX);
        regs.ic_ctrl1_reg    = readRegister(IC_CTRL1_DRV871XX);
        regs.ic_ctrl2_reg    = readRegister(IC_CTRL2_DRV871XX);
        regs.brg_ctrl1_reg   = readRegister(BRG_CTRL1_DRV871XX);
        regs.brg_ctrl2_reg   = readRegister(BRG_CTRL2_DRV871XX);
        regs.pwm_ctrl1_reg   = readRegister(PWM_CTRL1_DRV871XX);
        regs.pwm_ctrl2_reg   = readRegister(PWM_CTRL2_DRV871XX);
        regs.pwm_ctrl3_reg   = readRegister(PWM_CTRL3_DRV871XX);
        regs.pwm_ctrl4_reg   = readRegister(PWM_CTRL4_DRV871XX);
        regs.idrv_ctrl1_reg  = readRegister(IDRV_CTRL1_DRV871XX);
        regs.idrv_ctrl2_reg  = readRegister(IDRV_CTRL2_DRV871XX);
        regs.idrv_ctrl3_reg  = readRegister(IDRV_CTRL3_DRV871XX);
        regs.idrv_ctrl4_reg  = readRegister(IDRV_CTRL4_DRV871XX);
        regs.idrv_ctrl5_reg  = readRegister(IDRV_CTRL5_DRV871XX);
        regs.idrv_ctrl6_reg  = readRegister(IDRV_CTRL6_DRV871XX);
        regs.idrv_ctrl7_reg  = readRegister(IDRV_CTRL7_DRV871XX);
        regs.idrv_ctrl8_reg  = readRegister(IDRV_CTRL8_DRV871XX);
        regs.idrv_ctrl9_reg  = readRegister(IDRV_CTRL9_DRV871XX);
        regs.drv_ctrl1_reg   = readRegister(DRV_CTRL1_DRV871XX);
        regs.drv_ctrl2_reg   = readRegister(DRV_CTRL2_DRV871XX);
        regs.drv_ctrl3_reg   = readRegister(DRV_CTRL3_DRV871XX);
        regs.drv_ctrl4_reg   = readRegister(DRV_CTRL4_DRV871XX);
        regs.drv_ctrl5_reg   = readRegister(DRV_CTRL5_DRV871XX);
        regs.drv_ctrl6_reg   = readRegister(DRV_CTRL6_DRV871XX);
        regs.drv_ctrl7_reg   = readRegister(DRV_CTRL7_DRV871XX);
        regs.vds_ctrl1_reg   = readRegister(VDS_CTRL1_DRV871XX);
        regs.vds_ctrl2_reg   = readRegister(VDS_CTRL2_DRV871XX);
        regs.vds_ctrl3_reg   = readRegister(VDS_CTRL3_DRV871XX);
        regs.vds_ctrl4_reg   = readRegister(VDS_CTRL4_DRV871XX);
        regs.olsc_ctrl1_reg  = readRegister(OLSC_CTRL1_DRV871XX);
        regs.olsc_ctrl2_reg  = readRegister(OLSC_CTRL2_DRV871XX);
        regs.uvov_ctrl_reg   = readRegister(UVOV_CTRL_DRV871XX);
        regs.csa_ctrl1_reg   = readRegister(CSA_CTRL1_DRV871XX);
        regs.csa_ctrl2_reg   = readRegister(CSA_CTRL2_DRV871XX);
        regs.csa_ctrl3_reg   = readRegister(CSA_CTRL3_DRV871XX);
        regs.agd_ctrl1_reg   = readRegister(AGD_CTRL1_DRV871XX);
        regs.pdr_ctrl1_reg   = readRegister(PDR_CTRL1_DRV871XX);
        regs.pdr_ctrl2_reg   = readRegister(PDR_CTRL2_DRV871XX);
        regs.pdr_ctrl3_reg   = readRegister(PDR_CTRL3_DRV871XX);
        regs.pdr_ctrl4_reg   = readRegister(PDR_CTRL4_DRV871XX);
        regs.pdr_ctrl5_reg   = readRegister(PDR_CTRL5_DRV871XX);
        regs.pdr_ctrl6_reg   = readRegister(PDR_CTRL6_DRV871XX);
        regs.pdr_ctrl7_reg   = readRegister(PDR_CTRL7_DRV871XX);
        regs.pdr_ctrl8_reg   = readRegister(PDR_CTRL8_DRV871XX);
        regs.pdr_ctrl9_reg   = readRegister(PDR_CTRL9_DRV871XX);
        regs.pdr_ctrl10_reg  = readRegister(PDR_CTRL10_DRV871XX);
        regs.stc_ctrl1_reg   = readRegister(STC_CTRL1_DRV871XX);
        regs.stc_ctrl2_reg   = readRegister(STC_CTRL2_DRV871XX);
        regs.stc_ctrl3_reg   = readRegister(STC_CTRL3_DRV871XX);
        regs.stc_ctrl4_reg   = readRegister(STC_CTRL4_DRV871XX);
        regs.dcc_ctrl1_reg   = readRegister(DCC_CTRL1_DRV871XX);
        regs.pst_ctrl1_reg   = readRegister(PST_CTRL1_DRV871XX);
        regs.pst_ctrl2_reg   = readRegister(PST_CTRL2_DRV871XX);
        regs.sgd_stat1_reg   = readRegister(SGD_STAT1_DRV871XX);
        regs.sgd_stat2_reg   = readRegister(SGD_STAT2_DRV871XX);
        regs.sgd_stat3_reg   = readRegister(SGD_STAT3_DRV871XX);
    } else {
        regs.ic_status_1_reg  = readRegister(IC_STAT_1);
        regs.vgs_vds_stat_reg = readRegister(VGS_VDS_STAT);
        regs.ic_status_2_reg  = readRegister(IC_STAT_2);
        regs.ic_ctrl_reg      = readRegister(IC_CTRL);
        regs.brg_ctrl_reg     = readRegister(BRG_CTRL);
        regs.drv_ctrl_1_reg   = readRegister(DRV_CTRL_1);
        regs.drv_ctrl_2_reg   = readRegister(DRV_CTRL_2);
        regs.drv_ctrl_3_reg   = readRegister(DRV_CTRL_3);
        regs.vds_ctrl_1_reg   = readRegister(VDS_CTRL_1);
        regs.vds_ctrl_2_reg   = readRegister(VDS_CTRL_2);
        regs.olsc_ctrl_reg    = readRegister(OLSC_CTRL);
        regs.uvov_ctrl_reg    = readRegister(UVOV_CTRL);
        regs.csa_ctrl_reg     = readRegister(CSA_CTRL);
    }
}

DRV8x06_PWMMode DRV8x06::getPWMMode()
{
    if (_isDRV871x) {
        uint8_t val = readRegister(IC_CTRL1_DRV871XX);
        return (DRV8x06_PWMMode)((val & IC_CTRL1_PWM_MODE_MASK) >> IC_CTRL1_PWM_MODE_SHIFT);
    }
    uint8_t val = readRegister(BRG_CTRL);
    return (DRV8x06_PWMMode)((val & BRG_CTRL_PWM_MODE_MASK) >> BRG_CTRL_PWM_MODE_SHIFT);
}

void DRV8x06::setPWMMode(DRV8x06_PWMMode mode)
{
    if (_isDRV871x) {
        regs.ic_ctrl1_reg = (regs.ic_ctrl1_reg & ~IC_CTRL1_PWM_MODE_MASK) |
                            ((mode << IC_CTRL1_PWM_MODE_SHIFT) & IC_CTRL1_PWM_MODE_MASK);
        writeRegister(IC_CTRL1_DRV871XX, regs.ic_ctrl1_reg);
    } else {
        regs.brg_ctrl_reg = (regs.brg_ctrl_reg & ~BRG_CTRL_PWM_MODE_MASK) |
                            ((mode << BRG_CTRL_PWM_MODE_SHIFT) & BRG_CTRL_PWM_MODE_MASK);
        writeRegister(BRG_CTRL, regs.brg_ctrl_reg);
    }
}

void DRV8x06::_pwmWrite(uint8_t pin, uint8_t duty0to100)
{
    if (duty0to100 == 0) {
        digitalWrite(pin, LOW);
    } else if (duty0to100 >= 100) {
        digitalWrite(pin, HIGH);
    } else {
        analogWrite(pin, (uint32_t)duty0to100 * 255 / 100);
    }
}

void DRV8x06::setSpeed(int16_t speed)
{
    if (speed >  100) speed =  100;
    if (speed < -100) speed = -100;

    if (speed >= 0) {
        digitalWrite(_in2Pin, HIGH);
        _pwmWrite(_in1Pin, (uint8_t)speed);
    } else {
        digitalWrite(_in2Pin, LOW);
        _pwmWrite(_in1Pin, (uint8_t)(-speed));
    }
}

void DRV8x06::setHB1Duty(uint8_t duty) { _pwmWrite(_in1Pin, duty); }
void DRV8x06::setHB2Duty(uint8_t duty) { _pwmWrite(_in2Pin, duty); }

void DRV8x06::setHB3Duty(uint8_t duty)
{
    if (_in3Pin >= 0) _pwmWrite((uint8_t)_in3Pin, duty);
}

void DRV8x06::setHB4Duty(uint8_t duty)
{
    if (_in4Pin >= 0) _pwmWrite((uint8_t)_in4Pin, duty);
}

void DRV8x06::_regInit()
{
    if (_isDRV871x) {
        writeRegister(IC_CTRL1_DRV871XX,  0x07);  // independent mode, drv enable, clr fault
        writeRegister(IDRV_CTRL1_DRV871XX, 0xEE); // IDRIVE peak = 48mA source / 48mA sink
        writeRegister(IDRV_CTRL2_DRV871XX, 0xEE);
        writeRegister(IDRV_CTRL3_DRV871XX, 0xEE);
        writeRegister(IDRV_CTRL4_DRV871XX, 0xEE);
        writeRegister(VDS_CTRL1_DRV871XX,  0xAA); // VDS OCP threshold 0.5 V
        writeRegister(VDS_CTRL2_DRV871XX,  0xAA);
        if (_isDRV8718S) {
            writeRegister(IDRV_CTRL5_DRV871XX, 0xEE);
            writeRegister(IDRV_CTRL6_DRV871XX, 0xEE);
            writeRegister(IDRV_CTRL7_DRV871XX, 0xEE);
            writeRegister(IDRV_CTRL8_DRV871XX, 0xEE);
            writeRegister(VDS_CTRL3_DRV871XX,  0xAA);
            writeRegister(VDS_CTRL4_DRV871XX,  0xAA);
        }
    } else {
        writeRegister(IC_CTRL,    0x87);  // drv enable, clr fault
        writeRegister(BRG_CTRL,   (_deviceID == DRV8106S_Q1) ? 0x00 : 0x20);
        writeRegister(DRV_CTRL_1, 0xEE); // IDRIVE 48mA/48mA
        writeRegister(DRV_CTRL_2, 0xEE);
        writeRegister(VDS_CTRL_2, 0xAA); // VDS OCP threshold 0.5 V
    }

    readAllRegisters();
}
