#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "i2c_error.h"
#include "i2c.h"
#include "ltr559.h"

LTR559_ALS::LTR559_ALS() : I2CDevice(LTR559_DEVICE_NAME, LTR559_BUS_ADDRESS)
{
    _regALSControl =        new I2CRegister8bit(this, LTR559_REG_ALSCONTROL_NAME, LTR559_REG_ALSCONTROL_ADDRESS);
    _regALSMeasureRate =    new I2CRegister8bit(this, LTR559_REG_ALSMEASURERT_NAME, LTR559_REG_ALSMEASURERT_ADDRESS);
    _regALSThresholdHi =    new I2CRegister16bit(this, LTR559_REG_ALSTHRESHI_NAME, LTR559_REG_ALSTHRESHI_ADDRESS);
    _regALSThresholdLo =    new I2CRegister16bit(this, LTR559_REG_ALSTHRESLO_NAME, LTR559_REG_ALSTHRESLO_ADDRESS);
    _regALSData =           new I2CRegisterBlock(this, LTR559_REG_ALSDATA_NAME, LTR559_REG_ALSDATA_ADDRESS);

    addRegister(_regALSControl);
    addRegister(_regALSMeasureRate);
    addRegister(_regALSThresholdHi);
    addRegister(_regALSThresholdLo);
    addRegister(_regALSData);
}

LTR559_ALS::LTR559_ALS(ALS_int_time t, ALS_meas_rate m, ALS_gain g) : LTR559_ALS()
{
    this->integrationTime = t;
    this->measurementRate = m;
    this->gain = g;
}

LTR559_ALS::~LTR559_ALS()
{
    delete _regALSData;
    delete _regALSThresholdLo;
    delete _regALSThresholdHi;
    delete _regALSMeasureRate;
    delete _regALSControl;
}

void LTR559_ALS::initialise()
{
    printf("Initialising LTR559\n");

    reset();

    setALSGain(gain);

    setALSIntegrationTime(integrationTime);
    setALSMeasureRate(measurementRate);

    _regALSThresholdHi->write(0xFFFF);
    _regALSThresholdLo->write(0x0000);

    setALSMode(mode_active);

    usleep(10000L);

    printf("Completed initialisation of LTR559\n");
}

void LTR559_ALS::reset()
{
    _regALSControl->setBits(0x02, 0x02);

    while (!isReset()) {
        usleep(1000L);
    }
}

bool LTR559_ALS::isReset()
{
    return ((_regALSControl->read() & 0x02) == 0);
}

void LTR559_ALS::setALSGain(ALS_gain g)
{
    this->gain = g;
    _regALSControl->setBits(0x1C, g);
}

void LTR559_ALS::setALSMode(ALS_mode m)
{
    _regALSControl->setBits(0x01, m);
}

void LTR559_ALS::setALSIntegrationTime(ALS_int_time t)
{
    this->integrationTime = t;
    _regALSMeasureRate->setBits(0x38, t);
}

void LTR559_ALS::setALSMeasureRate(ALS_meas_rate r)
{
    this->measurementRate = r;
    _regALSMeasureRate->setBits(0x07, r);
}

double LTR559_ALS::getIntegrationTime()
{
    double int_t;

    switch (this->integrationTime) {
        case int_t_50:
            int_t = 50.0;
            break;

        case int_t_100:
            int_t = 100.0;
            break;

        case int_t_150:
            int_t = 150.0;
            break;

        case int_t_200:
            int_t = 200.0;
            break;

        case int_t_250:
            int_t = 250.0;
            break;

        case int_t_300:
            int_t = 300.0;
            break;

        case int_t_350:
            int_t = 350.0;
            break;

        case int_t_400:
            int_t = 400.0;
            break;
    }

    return int_t;
}

double LTR559_ALS::getMeasurementRate()
{
    double mr;

    switch (this->measurementRate) {
        case mr_50:
            mr = 50.0;
            break;

        case mr_100:
            mr = 100.0;
            break;

        case mr_200:
            mr = 200.0;
            break;

        case mr_500:
            mr = 500.0;
            break;

        case mr_1000:
            mr = 1000.0;
            break;

        case mr_2000:
            mr = 2000.0;
            break;
    }

    return mr;
}

int LTR559_ALS::getGain()
{
    int g;

    switch (this->gain) {
        case alsg_1:
            g = 1;
            break;

        case alsg_2:
            g = 2;
            break;

        case alsg_4:
            g = 4;
            break;

        case alsg_8:
            g = 8;
            break;

        case alsg_48:
            g = 48;
            break;

        case alsg_96:
            g = 96;
            break; 
    }

    return g;
}

double LTR559_ALS::readLux()
{
    uint16_t        alsval_ch0 = 0x0000;
    uint16_t        alsval_ch1 = 0x0000;
    double          lux;
    int             ch0_co;
    int             ch1_co;
    int             ratio;
    int             ch0_c[4] = {17743,42785,5926,0};
    int             ch1_c[4] = {-11059,19548,-1185,0};
    uint8_t         alsData[4];

    usleep((useconds_t)(getMeasurementRate() * 1000.0) + (useconds_t)(getIntegrationTime() * 1000.0) + 100L);

    _regALSData->read(alsData, sizeof(alsData));

    alsval_ch1 |= alsData[0];
    alsval_ch1 |= (alsData[1] << 8);

    alsval_ch0 |= alsData[2];
    alsval_ch0 |= (alsData[3] << 8);

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
    lux /= (getIntegrationTime() / (double)100.0);
    lux /= (double)getGain();
    lux /= (double)10000.0;

    //lux = (alsval_ch0 * ch0_co - alsval_ch1 * ch1_co) / 10000;

    return lux;
}
