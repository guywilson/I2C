#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "i2c_error.h"
#include "i2c.h"
#include "ltr559.h"

LTR559::LTR559() : I2CDevice(LTR559_DEVICE_NAME, LTR559_BUS_ADDRESS)
{
    ALSControl = new I2CRegister(LTR559_REG_ALSCONTROL_NAME, LTR559_REG_ALSCONTROL_ADDRESS);
    ALSMeasureRate = new I2CRegister(LTR559_REG_ALSMEASURERT_NAME, LTR559_REG_ALSMEASURERT_ADDRESS);
    ALSThresholdHi = new I2CRegister(LTR559_REG_ALSTHRESHI_NAME, LTR559_REG_ALSTHRESHI_ADDRESS);
    ALSThresholdLo = new I2CRegister(LTR559_REG_ALSTHRESLO_NAME, LTR559_REG_ALSTHRESLO_ADDRESS);
    ALSChannel0 = new I2CRegister(LTR559_REG_ALSCHANNEL0_NAME, LTR559_REG_ALSCHANNEL0_ADDRESS);
    ALSChannel1 = new I2CRegister(LTR559_REG_ALSCHANNEL1_NAME, LTR559_REG_ALSCHANNEL1_ADDRESS);

    addRegister(ALSControl);
    addRegister(ALSMeasureRate);
    addRegister(ALSThresholdHi);
    addRegister(ALSThresholdLo);
    addRegister(ALSChannel0);
    addRegister(ALSChannel1);
}

LTR559::~LTR559()
{
    delete ALSChannel1;
    delete ALSChannel0;
    delete ALSThresholdLo;
    delete ALSThresholdHi;
    delete ALSMeasureRate;
    delete ALSControl;
}

void LTR559::initialise()
{
    uint8_t     isResetting = 1;

    bus.acquire(getName());
    writeRegister(LTR559_REG_ALSCONTROL_NAME, (uint8_t)0x01);
    bus.release(getName());

    while (isResetting) {
        usleep(1000L);

        bus.acquire(getName());
        isResetting = (readRegister8(LTR559_REG_ALSCONTROL_NAME) & 0x02);
        bus.release(getName());
    }

    bus.acquire(getName());
    writeRegister(LTR559_REG_ALSCONTROL_NAME, (uint8_t)0x09);
    bus.release(getName());

    bus.acquire(getName());
    writeRegister(LTR559_REG_ALSMEASURERT_NAME, (uint8_t)0x08);
    bus.release(getName());

    bus.acquire(getName());
    writeRegister(LTR559_REG_ALSTHRESHI_NAME, (uint16_t)0xFFFF);
    bus.release(getName());

    bus.acquire(getName());
    writeRegister(LTR559_REG_ALSTHRESLO_NAME, (uint16_t)0x0000);
    bus.release(getName());
}

int32_t LTR559::readLux()
{
    uint16_t        alsval_ch0;
    uint16_t        alsval_ch1;
    int32_t         lux;
    int             ch0_co;
    int             ch1_co;
    int             ratio;
    int             ch0_c[4] = {17743,42785,5926,0};
    int             ch1_c[4] = {-11059,19548,-1185,0};

    bus.acquire(getName());
    alsval_ch0 = readRegister16(LTR559_REG_ALSCHANNEL0_NAME);
    bus.release(getName());

    bus.acquire(getName());
    alsval_ch1 = readRegister16(LTR559_REG_ALSCHANNEL1_NAME);
    bus.release(getName());

    if ((alsval_ch0 + alsval_ch1) == 0) {
            ratio = 1000;
    }
    else {
        ratio = alsval_ch1 * 1000 / (alsval_ch1 + alsval_ch0);
    }
    if (ratio < 450) {
            ch0_co = ch0_c[0];
            ch1_co = ch1_c[0];
    }
    else if ((ratio >= 450) && (ratio < 640)) {
            ch0_co = ch0_c[1];
            ch1_co = ch1_c[1];
    }
    else if ((ratio >= 640) && (ratio < 850)) {
            ch0_co = ch0_c[2];
            ch1_co = ch1_c[2];
    }
    else if (ratio >= 850) {
            ch0_co = ch0_c[3];
            ch1_co = ch1_c[3];
    }
    else {
        throw i2c_error("Invalid lux calculation", __FILE__, __LINE__);
    }
    
    lux = (alsval_ch0 * ch0_co - alsval_ch1 * ch1_co) / 10000;

    return lux;
}
