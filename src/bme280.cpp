#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "i2c_error.h"
#include "i2c.h"
#include "bme280.h"

BME280::BME280() : I2CDevice(BME280_DEVICE_NAME, BME280_BUS_ADDRESS)
{
    I2CRegister temp("Temp", 0xFA);
    addRegister(temp.getName(), temp);

    memset(&compensationData, 0x00, sizeof(BME280_COMPENSATIONDATA));
}

void BME280::readTPH(BME280_TPH * tph)
{
    uint8_t                     buf[64];
    int32_t                     pressure;
    int32_t                     temperature;
    int32_t                     humidity;

    I2CBus & bus = I2CBus::getInstance();

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

    printf("Got raw pressure value %u\n", pressure);
    printf("Got raw temperature value %u\n", temperature);
    printf("Got raw humidity value %u\n", humidity);

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

    printf("**** START COMPENSATION DATA ****\n");
    for (uint32_t i = 0;i < sizeof(BME280_COMPENSATIONDATA);i++) {
        printf("[%02X]", buf[i]);
    }
    printf("\n**** END COMPENSATION DATA ****\n\n");

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

    printf("Temperature calibration = %u\n", temperatureCalibration);

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
    double var1;
    double var2;
    double var3;
    double var4;
    double var5;
    double var6;

    printf("Humidity compensation1 = 0x%02X\n", compensationData.humidity1);
    printf("Humidity compensation2 = 0x%04X [%d]\n", compensationData.humidity2, compensationData.humidity2);
    printf("Humidity compensation3 = 0x%02X\n", compensationData.humidity3);
    printf("Humidity compensation4 = 0x%04X [%d]\n", compensationData.humidity4, compensationData.humidity4);
    printf("Humidity compensation5 = 0x%04X [%d]\n", compensationData.humidity5, compensationData.humidity5);
    printf("Humidity compensation6 = 0x%02X [%d]\n", compensationData.humidity6, compensationData.humidity6);

    var1 = ((double)temperatureCalibration) - (double)76800.0;
    printf("var1 = %.2f\n", var1);

    var2 = (((double)compensationData.humidity4) * (double)64.0 + (((double)compensationData.humidity5) / (double)16384.0) * var1);
    printf("var2 = %.2f\n", var2);

    var3 = (double)adcInput - var2;
    printf("var3 = %.2f\n", var3);

    var4 = ((double)compensationData.humidity2) / (double)65536.0;
    printf("var4 = %.2f\n", var4);

    var5 = ((double)1.0 + (((double)compensationData.humidity3) / (double)67108864.0) * var1);
    printf("var5 = %.2f\n", var5);

    var6 = (double)1.0 + (((double)compensationData.humidity6) / (double)67108864.0) * var1 * var5;
    printf("var6 = %.2f\n", var6);
    var6 = var3 * var4 * (var5 * var6);
    printf("var6 = %.2f\n", var6);

    humidity = var6 * ((double)1.0 - ((double)compensationData.humidity1) * var6 / (double)524288.0);

    printf("Humidity value = %.2f\n", humidity);

    if (humidity > humidity_max) {
        humidity = humidity_max;
    }
    else if (humidity < humidity_min) {
        humidity = humidity_min;
    }

    return humidity;
}
