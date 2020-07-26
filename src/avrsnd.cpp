#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "i2c_error.h"
#include "i2c.h"
#include "avrsnd.h"

AVRSound::AVRSound() : I2CDevice(AVRSND_DEVICE_NAME, AVRSND_BUS_ADDRESS)
{
    _regRMSWindowSize = new I2CRegister16bit(this, AVRSND_REG_WSIZE_NAME, AVRSND_REG_WSIZE_ADDRESS);
    _regLoudness = new I2CRegister8bit(this, AVRSND_REG_LOUDNESS_NAME, AVRSND_REG_LOUDNESS_ADDRESS);
    _regReset = new I2CRegister8bit(this, AVRSND_REG_RESET_NAME, AVRSND_REG_RESET_ADDRESS);

    addRegister(_regRMSWindowSize);
    addRegister(_regLoudness);
    addRegister(_regReset);
}

AVRSound::AVRSound(uint16_t windowSize)
{
    this->windowSize = windowSize;
}

AVRSound::~AVRSound()
{
    delete _regReset;
    delete _regLoudness;
    delete _regRMSWindowSize;
}

void AVRSound::initialise()
{
    printf("Initialising AVRSound\n");

    resetDevice();

    setWindowSize(this->windowSize);
}

void AVRSound::resetDevice()
{
    _regReset->write(AVRSND_RESET_CMD);

    usleep(2000000L);
}

void AVRSound::setWindowSize(uint16_t windowSize)
{
    _regRMSWindowSize->write(windowSize);
}

int AVRSound::getLoudnessValue()
{
    uint8_t value = _regLoudness->read();

    return (int)value;
}

const char * AVRSound::getLoudnessDescription()
{
    const char * desc = loudnessDesc[getLoudnessValue() / 25];

    return desc;
}
