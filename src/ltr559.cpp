#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "i2c_error.h"
#include "i2c.h"
#include "ltr559.h"

LTR559::LTR559() : I2CDevice(LTR559_DEVICE_NAME, LTR559_BUS_ADDRESS)
{
    I2CRegister ALSControl(LTR559_ALS_CONTROL_NAME, LTR559_ALS_CONTROL_ADDRESS);
    addRegister(ALSControl.getName(), ALSControl);

    I2CRegister ALSChannel0(LTR559_ALS_CHANNEL0_NAME, LTR559_ALS_CHANNEL0_ADDRESS);
    addRegister(ALSChannel0.getName(), ALSChannel0);

    I2CRegister ALSChannel1(LTR559_ALS_CHANNEL1_NAME, LTR559_ALS_CHANNEL1_ADDRESS);
    addRegister(ALSChannel1.getName(), ALSChannel1);

    /*
    ** Enable ALS...
    */
    bus.acquire(getName());
    writeRegister(LTR559_ALS_CONTROL_NAME, (uint8_t)0x01);
    bus.release(getName());
}

uint16_t LTR559::readALS0()
{
    uint16_t        value;

    bus.acquire(getName());
    value = readRegister16(LTR559_ALS_CHANNEL0_NAME);
    bus.release(getName());

    return value;
}

uint16_t LTR559::readALS1()
{
    uint16_t        value;

    bus.acquire(getName());
    value = readRegister16(LTR559_ALS_CHANNEL1_NAME);
    bus.release(getName());

    return value;
}
