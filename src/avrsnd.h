#include <stdint.h>

#include "i2c.h"

#ifndef _INCL_AVRSND
#define _INCL_AVRSND

#define AVRSND_DEVICE_NAME              "AVRSound"
#define AVRSND_BUS_ADDRESS              0x18

#define AVRSND_REG_WSIZE_NAME           "Window Size"        
#define AVRSND_REG_WSIZE_ADDRESS        0xF0

#define AVRSND_REG_LOUDNESS_NAME        "Loudness"
#define AVRSND_REG_LOUDNESS_ADDRESS     0xF2

#define AVRSND_REG_RESET_NAME           "Reset"
#define AVRSND_REG_RESET_ADDRESS        0xF3

#define AVRSND_RESET_CMD                0xB6

#define AVRSND_THRESHOLD_SILENT         10
#define AVRSND_THRESHOLD_VQUIET         20
#define AVRSND_THRESHOLD_QUIET          50
#define AVRSND_THRESHOLD_LOW            80
#define AVRSND_THRESHOLD_MLOW           110
#define AVRSND_THRESHOLD_MEDIUM         140
#define AVRSND_THRESHOLD_MHIGH          170
#define AVRSND_THRESHOLD_HIGH           200
#define AVRSND_THRESHOLD_LOUD           220

class AVRSound : public I2CDevice
{
private:
    I2CBus & bus = I2CBus::getInstance();

    uint16_t                windowSize;

    I2CRegister16bit *      _regRMSWindowSize;
    I2CRegister8bit *       _regLoudness;
    I2CRegister8bit *       _regReset;

    const char * loudnessDesc[10] = {
        "Silent",
        "Very quiet",
        "Quiet",
        "Low",
        "Medium low",
        "Medium",
        "Medium high",
        "High",
        "Loud",
        "Very loud"
    };

    AVRSound();

    void resetDevice();

public:
    AVRSound(uint16_t windowSize);
    virtual ~AVRSound();

    virtual void initialise();

    void        setWindowSize(uint16_t windowSize);
    uint16_t    getWindowSize() {
        return this->windowSize;
    }

    int                 getLoudnessValue();
    const char *        getLoudnessDescription();
    const char *        getLoudnessDescription(int value);
};

#endif
