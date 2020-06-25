#include <stdint.h>

#include "i2c.h"
#include "bme280.h"

BME280::BME280() : I2CDevice("BME280", 0x76)
{
    I2CRegister temp("Temp", 0xFA);
    addRegister(temp.getName(), temp);
}

void BME280::readTPH(BME_TPH tph)
{
    
}