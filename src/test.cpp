#include <iostream>
#include <string>

#include <stdio.h>

#include "i2c.h"
#include "bme280.h"

using namespace std;

BME280_COMPENSATIONDATA     compensationData;
int32_t                     temperatureCalibration;

double _getCompensatedHumidity(int32_t adcInput)
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

    humidity = var6 * var6 * ((double)1.0 - (double)compensationData.humidity1) / (double)524288.0;

    printf("Humidity value = %.2f\n", humidity);

    if (humidity > humidity_max) {
        humidity = humidity_max;
    }
    else if (humidity < humidity_min) {
        humidity = humidity_min;
    }

    return humidity;
}

/* int main(void)
{
    compensationData.humidity1 = 0x00;
    compensationData.humidity2 = 0x017E;
    compensationData.humidity3 = 0x00;
    compensationData.humidity4 = 0x2111;
    compensationData.humidity5 = 0x1E03;
    compensationData.humidity6 = 0x00;

    temperatureCalibration = 202637;

    double humidity = _getCompensatedHumidity(20795);

    printf("Calculated humidity = %.2f\n", humidity);
}
 */

int main(void)
{
    BME280_TPH      tph;

    I2CBus & bus = I2CBus::getInstance();

    bus.openBus("/dev/i2c-1");

    BME280 bme280;

    bus.attachDevice(BME280_DEVICE_NAME, bme280);

    bme280.readTPH(&tph);

    printf("Temperature: %.2f\n", tph.temperature);
    printf("Pressure: %.2f\n", tph.pressure);
    printf("Humidity: %.2f\n", tph.humidity);

    bus.closeBus();

    return 0;
}
