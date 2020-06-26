#include <stdint.h>

#include "i2c.h"

#ifndef _INCL_LTR559
#define _INCL_LTR559

#define LTR559_DEVICE_NAME              "LTR559"
#define LTR559_BUS_ADDRESS              0x23

#define LTR559_ALS_CONTROL_NAME         "ALS Control Register"
#define LTR559_ALS_CONTROL_ADDRESS      0x80

#define LTR559_ALS_CHANNEL0_NAME        "ALS Channel 0"
#define LTR559_ALS_CHANNEL0_ADDRESS     0x8A

#define LTR559_ALS_CHANNEL1_NAME        "ALS Channel 1"
#define LTR559_ALS_CHANNEL1_ADDRESS     0x88

class LTR559 : public I2CDevice
{
private:
    I2CBus & bus = I2CBus::getInstance();

public:
    LTR559();

    int32_t readLux();
};

#endif
