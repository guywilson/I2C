#include <stdint.h>

#include "i2c.h"

#ifndef _INCL_LTR559
#define _INCL_LTR559

#define LTR559_DEVICE_NAME              "LTR559"
#define LTR559_BUS_ADDRESS              0x23

#define LTR559_REG_ALSCONTROL_NAME      "ALS Control Register"
#define LTR559_REG_ALSCONTROL_ADDRESS   0x80

#define LTR559_REG_ALSMEASURERT_NAME    "ALS Measure Rate"
#define LTR559_REG_ALSMEASURERT_ADDRESS 0x84

#define LTR559_REG_ALSTHRESLO_NAME     "ALS Threshold Low"
#define LTR559_REG_ALSTHRESLO_ADDRESS   0x99

#define LTR559_REG_ALSTHRESHI_NAME     "ALS Threshold High"
#define LTR559_REG_ALSTHRESHI_ADDRESS   0x97

#define LTR559_REG_ALSCHANNEL0_NAME     "ALS Channel 0"
#define LTR559_REG_ALSCHANNEL0_ADDRESS  0x8A

#define LTR559_REG_ALSCHANNEL1_NAME     "ALS Channel 1"
#define LTR559_REG_ALSCHANNEL1_ADDRESS  0x88

class LTR559 : public I2CDevice
{
private:
    I2CBus & bus = I2CBus::getInstance();

    I2CRegister8bit *   ALSControl;
    I2CRegister8bit *   ALSMeasureRate;
    I2CRegister16bit *  ALSThresholdHi;
    I2CRegister16bit *  ALSThresholdLo;
    I2CRegister16bit *  ALSChannel0;
    I2CRegister16bit *  ALSChannel1;

public:
    LTR559();
    ~LTR559();

    virtual void initialise();

    double readLux();
};

#endif
