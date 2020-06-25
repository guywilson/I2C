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
    uint8_t         buf[12];
    uint32_t        pressure;
    uint32_t        temperature;
    uint16_t        humidity;

    I2CBus & bus = I2CBus::getInstance();

    bus.acquire();
    readBlock(0xF7, buf, 8);

    pressure = (buf[0] << 12) + (buf[1] << 4) + ((buf[2] >> 4) & 0x0F);
    temperature = (buf[3] << 12) + (buf[4] << 4) + ((buf[5] >> 4) & 0x0F);
    humidity = (buf[6] << 8) + (buf[7]);
}

float BME280::getCompensatedTemperature(int32_t adcInput)
{

}

float BME280::getCompensatedPressure(int32_t adcInput)
{

}

float BME280::getCompensatedHumidity(int32_t adcInput)
{

}
