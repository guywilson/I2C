#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "i2c_error.h"
#include "i2c.h"
#include "ltr559.h"

LTR559::LTR559() : I2CDevice(LTR559_DEVICE_NAME, LTR559_BUS_ADDRESS)
{
    _regALSControl = new I2CRegister8bit(this, LTR559_REG_ALSCONTROL_NAME, LTR559_REG_ALSCONTROL_ADDRESS);
    _regALSMeasureRate = new I2CRegister8bit(this, LTR559_REG_ALSMEASURERT_NAME, LTR559_REG_ALSMEASURERT_ADDRESS);
    _regALSThresholdHi = new I2CRegister16bit(this, LTR559_REG_ALSTHRESHI_NAME, LTR559_REG_ALSTHRESHI_ADDRESS);
    _regALSThresholdLo = new I2CRegister16bit(this, LTR559_REG_ALSTHRESLO_NAME, LTR559_REG_ALSTHRESLO_ADDRESS);
    _regALSChannel0 = new I2CRegister16bit(this, LTR559_REG_ALSCHANNEL0_NAME, LTR559_REG_ALSCHANNEL0_ADDRESS);
    _regALSChannel1 = new I2CRegister16bit(this, LTR559_REG_ALSCHANNEL1_NAME, LTR559_REG_ALSCHANNEL1_ADDRESS);

    addRegister(_regALSControl);
    addRegister(_regALSMeasureRate);
    addRegister(_regALSThresholdHi);
    addRegister(_regALSThresholdLo);
    addRegister(_regALSChannel0);
    addRegister(_regALSChannel1);
}

LTR559::~LTR559()
{
    delete _regALSChannel1;
    delete _regALSChannel0;
    delete _regALSThresholdLo;
    delete _regALSThresholdHi;
    delete _regALSMeasureRate;
    delete _regALSControl;
}

void LTR559::initialise()
{
    uint8_t     isResetting = 1;

    _regALSControl->write(0x02);

    while (isResetting) {
        usleep(1000L);

        isResetting = (_regALSControl->read() & 0x02);
    }

    _regALSControl->write(0x09);

    _regALSMeasureRate->write(0x08);

    _regALSThresholdHi->write(0xFFFF);

    _regALSThresholdLo->write(0x0000);
}

double LTR559::readLux()
{
    uint16_t        alsval_ch0;
    uint16_t        alsval_ch1;
    double          lux;
    int             ch0_co;
    int             ch1_co;
    int             ratio;
    int             ch0_c[4] = {17743,42785,5926,0};
    int             ch1_c[4] = {-11059,19548,-1185,0};

    alsval_ch0 = _regALSChannel0->read();
    alsval_ch1 = _regALSChannel1->read();

    if ((alsval_ch0 + alsval_ch1) == 0) {
            ratio = 101;
    }
    else {
        ratio = alsval_ch1 * 100 / (alsval_ch1 + alsval_ch0);
    }
    if (ratio < 45) {
            ch0_co = ch0_c[0];
            ch1_co = ch1_c[0];
    }
    else if ((ratio >= 45) && (ratio < 64)) {
            ch0_co = ch0_c[1];
            ch1_co = ch1_c[1];
    }
    else if ((ratio >= 64) && (ratio < 85)) {
            ch0_co = ch0_c[2];
            ch1_co = ch1_c[2];
    }
    else if (ratio >= 85) {
            ch0_co = ch0_c[3];
            ch1_co = ch1_c[3];
    }
    else {
        throw i2c_error("Invalid lux calculation", __FILE__, __LINE__);
    }

    lux = ((double)alsval_ch0 * (double)ch0_co) - ((double)alsval_ch1 * (double)ch1_co);
    lux /= ((double)50.0 / (double)100.0);
    lux /= (double)4.0;
    lux /= (double)10000.0;

    //lux = (alsval_ch0 * ch0_co - alsval_ch1 * ch1_co) / 10000;

    return lux;
}
