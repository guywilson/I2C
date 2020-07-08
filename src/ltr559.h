#include <stdint.h>

#include "i2c.h"

#ifndef _INCL_LTR559
#define _INCL_LTR559

#define LTR559_DEVICE_NAME              "LTR559_ALS"
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

class LTR559_ALS : public I2CDevice
{
private:
    I2CBus & bus = I2CBus::getInstance();

    I2CRegister8bit *   _regALSControl;
    I2CRegister8bit *   _regALSMeasureRate;
    I2CRegister16bit *  _regALSThresholdHi;
    I2CRegister16bit *  _regALSThresholdLo;
    I2CRegister16bit *  _regALSChannel0;
    I2CRegister16bit *  _regALSChannel1;

    double  getIntegrationTime();
    double  getMeasurementRate();
    int     getGain();

public:
enum ALS_gain {
        alsg_1 =        0b00000000,
        alsg_2 =        0b00000100,
        alsg_4 =        0b00001000,
        alsg_8 =        0b00001100,
        alsg_48 =       0b00011000,
        alsg_96 =       0b00011100
    };

    enum ALS_mode {
        mode_standby =  0b00000000,
        mode_active =   0b00000001
    };

    enum ALS_int_time {
        int_t_100 =     0b00000000,
        int_t_50 =      0b00000100,
        int_t_200 =     0b00001000,
        int_t_400 =     0b00001100,
        int_t_150 =     0b00010000,
        int_t_250 =     0b00010100,
        int_t_300 =     0b00011000,
        int_t_350 =     0b00011100
    };

    enum ALS_meas_rate {
        mr_50 =         0b00000000,
        mr_100 =        0b00000001,
        mr_200 =        0b00000010,
        mr_500 =        0b00000011,
        mr_1000 =       0b00000100,
        mr_2000 =       0b00000101
    };

    LTR559_ALS();
    LTR559_ALS(ALS_int_time t, ALS_meas_rate m, ALS_gain g);
    ~LTR559_ALS();

    virtual void    initialise();

    void            reset();
    bool            isReset();

    void            setALSGain(ALS_gain g);
    void            setALSMode(ALS_mode m);
    void            setALSIntegrationTime(ALS_int_time t);
    void            setALSMeasureRate(ALS_meas_rate r);

    double          readLux();

private:
    ALS_int_time    integrationTime;
    ALS_meas_rate   measurementRate;
    ALS_gain        gain;
};

#endif
