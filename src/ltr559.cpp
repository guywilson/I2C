#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "i2c_error.h"
#include "i2c.h"
#include "ltr559.h"

LTR559::LTR559() : I2CDevice(LTR559_DEVICE_NAME, LTR559_BUS_ADDRESS)
{
    I2CRegister * ALSControl = new I2CRegister(LTR559_ALS_CONTROL_NAME, LTR559_ALS_CONTROL_ADDRESS);
    addRegister(ALSControl->getName(), ALSControl);
    printf("Added ALS Control reg\n");

    I2CRegister * ALSChannel0 = new I2CRegister(LTR559_ALS_CHANNEL0_NAME, LTR559_ALS_CHANNEL0_ADDRESS);
    addRegister(ALSChannel0->getName(), ALSChannel0);
    printf("Added ALS Channel0 reg\n");

    I2CRegister * ALSChannel1 = new I2CRegister(LTR559_ALS_CHANNEL1_NAME, LTR559_ALS_CHANNEL1_ADDRESS);
    addRegister(ALSChannel1->getName(), ALSChannel1);
    printf("Added ALS Channel1 reg\n");
}

void LTR559::enableALS()
{
    bus.acquire(getName());
    writeRegister(LTR559_ALS_CONTROL_NAME, (uint8_t)0x01);
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
    alsval_ch0 = readRegister16(LTR559_ALS_CHANNEL0_NAME);
    bus.release(getName());

    bus.acquire(getName());
    alsval_ch1 = readRegister16(LTR559_ALS_CHANNEL1_NAME);
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
