#include <stdint.h>

#include "i2c.h"

#ifndef _INCL_TESTDEVICE
#define _INCL_TESTDEVICE

class TestDevice : public I2CDevice
{
public:
    TestDevice();

    float readTemperature();
};

#endif
