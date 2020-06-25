#include <iostream>
#include <string>

#include <stdio.h>

#include "i2c.h"
#include "testdevice.h"

using namespace std;

TestDevice::TestDevice() : I2CDevice("Temp Sensor", 0x75)
{
    I2CRegister tempValue("Temp Value", 0x7E);
    addRegister("Temp Value", tempValue);
}

float TestDevice::readTemperature()
{
    uint16_t temperature = readRegister16("Temp Value");

    return ((float)(temperature >> 8) + (float)((temperature & 0x00FF) / 100.0));
}

int main(void)
{
    I2CBus & bus = I2CBus::getInstance();

    bus.openBus("/dev/i2c-1");

    TestDevice device;

    bus.attachDevice("Temp Sensor", device);

    bus.acquire("Temp Sensor");

    device.readTemperature();

    bus.release("Temp Sensor");

    bus.closeBus();
}
