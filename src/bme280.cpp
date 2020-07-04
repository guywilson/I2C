#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "i2c_error.h"
#include "i2c.h"
#include "bme280.h"

BME280::BME280() : I2CDevice(BME280_DEVICE_NAME, BME280_BUS_ADDRESS)
{
    memset(&compensationData, 0x00, sizeof(BME280_COMPENSATIONDATA));

    reset = new I2CRegister(BME280_REG_RESET_NAME, BME280_REG_RESET_ADDRESS);
    chipID = new I2CRegister(BME280_REG_CHIPID_NAME, BME280_REG_CHIPID_ADDRESS);
    status = new I2CRegister(BME280_REG_STATUS_NAME, BME280_REG_STATUS_ADDRESS);
    config = new I2CRegister(BME280_REG_CONFIG_NAME, BME280_REG_CONFIG_ADDRESS);
    ctrlMeasure = new I2CRegister(BME280_REG_CTRLMEAS_NAME, BME280_REG_CTRLMEAS_ADDRESS);
    ctrlHumidity = new I2CRegister(BME280_REG_CTRLHUM_NAME, BME280_REG_CTRLHUM_ADDRESS);

    addRegister(reset);
    addRegister(chipID);
    addRegister(status);
    addRegister(config);
    addRegister(ctrlMeasure);
    addRegister(ctrlHumidity);
}

BME280::~BME280()
{
    delete ctrlHumidity;
    delete ctrlMeasure;
    delete config;
    delete status;
    delete chipID;
    delete reset;
}

void BME280::initialise()
{
    /*
    ** Reset the device...
    */
    bus.acquire(getName());
    writeRegister(BME280_REG_RESET_NAME, (uint8_t)0xB6);
    bus.release(getName());

    usleep(100000L);

    bus.acquire(getName());
    writeRegister(BME280_REG_CTRLHUM_NAME, (uint8_t)0x05);
    bus.release(getName());

    bus.acquire(getName());
    writeRegister(BME280_REG_CTRLMEAS_NAME, (uint8_t)0xB7);
    bus.release(getName());

    bus.acquire(getName());
    writeRegister(BME280_REG_CONFIG_NAME, (uint8_t)0x88);
    bus.release(getName());
}

void BME280::readTPH(BME280_TPH * tph)
{
    uint8_t                     buf[64];
    int32_t                     pressure;
    int32_t                     temperature;
    int32_t                     humidity;

    /*
    ** Read raw temperature, pressure and humidity...
    */
    bus.acquire(this->getName());
    readBlock(0xF7, buf, 8);
    bus.release(this->getName());

    /*
    ** Get raw values...
    */
    pressure = (buf[0] << 12) + (buf[1] << 4) + ((buf[2] >> 4) & 0x0F);
    temperature = (buf[3] << 12) + (buf[4] << 4) + ((buf[5] >> 4) & 0x0F);
    humidity = (buf[6] << 8) + (buf[7]);

    /*
    ** Read compensation data, part 1...
    */
    bus.acquire(this->getName());
    readBlock(0x88, buf, 25);
    bus.release(this->getName());

    /*
    ** Read compensation data, part 2...
    */
    bus.acquire(this->getName());
    readBlock(0xE1, &buf[25], 8);
    bus.release(this->getName());

    memcpy(&compensationData, buf, sizeof(BME280_COMPENSATIONDATA));

    tph->temperature = getCompensatedTemperature(temperature);
    tph->pressure = getCompensatedPressure(pressure);
    tph->humidity = getCompensatedHumidity(humidity);
}

double BME280::getCompensatedTemperature(int32_t adcInput)
{
    double var1;
    double var2;
    double temperature;
    double temperature_min = -40;
    double temperature_max = 85;

    var1 = ((double)adcInput) / 16384.0 - ((double)compensationData.temperature1) / 1024.0;
    var1 = var1 * ((double)compensationData.temperature2);

    var2 = (((double)adcInput) / 131072.0 - ((double)compensationData.temperature1) / 8192.0);
    var2 = (var2 * var2) * ((double)compensationData.temperature3);

    temperatureCalibration = (int32_t)(var1 + var2);

    temperature = (var1 + var2) / 5120.0;

    if (temperature < temperature_min) {
        temperature = temperature_min;
    }
    else if (temperature > temperature_max) {
        temperature = temperature_max;
    }

    return temperature;
}

double BME280::getCompensatedPressure(int32_t adcInput)
{
    double var1;
    double var2;
    double var3;
    double pressure;
    double pressure_min = 30000.0;
    double pressure_max = 110000.0;

    var1 = ((double)temperatureCalibration / 2.0) - 64000.0;

    var2 = var1 * var1 * ((double)compensationData.pressure6) / 32768.0;
    var2 = var2 + var1 * ((double)compensationData.pressure5) * 2.0;
    var2 = (var2 / 4.0) + (((double)compensationData.pressure4) * 65536.0);

    var3 = ((double)compensationData.pressure3) * var1 * var1 / 524288.0;

    var1 = (var3 + ((double)compensationData.pressure2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)compensationData.pressure1);

    /* avoid exception caused by division by zero */
    if (var1 > (0.0)) {
        pressure = 1048576.0 - (double)adcInput;
        pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;

        var1 = ((double)compensationData.pressure9) * pressure * pressure / 2147483648.0;

        var2 = pressure * ((double)compensationData.pressure8) / 32768.0;

        pressure = pressure + (var1 + var2 + ((double)compensationData.pressure7)) / 16.0;

        if (pressure < pressure_min) {
            pressure = pressure_min;
        }
        else if (pressure > pressure_max) {
            pressure = pressure_max;
        }
    }
    else {
        throw i2c_error("Invalid pressure reading, cannot compute", __FILE__, __LINE__);
    }

    return pressure / 100;
}

double BME280::getCompensatedHumidity(int32_t adcInput)
{
    double humidity;
    double humidity_min = 0.0;
    double humidity_max = 100.0;
    // double var1;
    // double var2;
    // double var3;
    // double var4;
    // double var5;
    // double var6;

    // var1 = ((double)temperatureCalibration) - (double)76800.0;

    // var2 = (((double)compensationData.humidity4) * (double)64.0 + (((double)compensationData.humidity5) / (double)16384.0) * var1);

    // var3 = (double)adcInput - var2;

    // var4 = ((double)compensationData.humidity2) / (double)65536.0;

    // var5 = ((double)1.0 + (((double)compensationData.humidity3) / (double)67108864.0) * var1);

    // var6 = (double)1.0 + (((double)compensationData.humidity6) / (double)67108864.0) * var1 * var5;
    // var6 = var3 * var4 * (var5 * var6);

    // humidity = var6 * var6 * ((double)1.0 - (double)compensationData.humidity1) / (double)524288.0;

    humidity = ((double)temperatureCalibration) - (double)76800.0;
    humidity = 
        ((double)adcInput - 
        ((double)compensationData.humidity4 * 
        (double)64.0 + 
        (double)compensationData.humidity5 / 
        (double)16384.0 * 
        humidity)) * 
        ((double)compensationData.humidity2 / 
        (double)65536.0 * 
        ((double)1.0 + 
        (double)compensationData.humidity6 / 
        (double)67108864.0 * 
        humidity * 
        ((double)1.0 + 
        (double)compensationData.humidity3 / 
        (double)67108864.0 * 
        humidity)));
    humidity = humidity * ((double)1.0 - (double)compensationData.humidity1 * humidity / (double)524288.0);

    if (humidity > humidity_max) {
        humidity = humidity_max;
    }
    else if (humidity < humidity_min) {
        humidity = humidity_min;
    }

    return humidity;
}
