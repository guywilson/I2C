#include <iostream>
#include <string>

#include <stdio.h>

#include "i2c.h"
#include "bme280.h"

using namespace std;

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
