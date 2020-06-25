#include <iostream>
#include <string>

#include <stdio.h>

#include "i2c.h"
#include "bme280.h"

using namespace std;

#define TEMP_COMPENSATION_FACTOR            2.25

double getCPUTemp()
{
    double      cpuTemp = -299.0;
    FILE *      fptr;
    char        szTemp[8];
    int         i = 0;

    fptr = fopen("/sys/class/thermal/thermal_zone0/temp", "rt");

    if (fptr == NULL) {
        printf("Could not open cpu temperature file");
        exit(-1);
    }

    while (!feof(fptr)) {
        szTemp[i++] = (char)fgetc(fptr);
    }
    
    szTemp[i] = 0;

    fclose(fptr);

    cpuTemp = atof(szTemp) / 1000.0;

    return cpuTemp;
}

int main(void)
{
    BME280_TPH      tph;

    I2CBus & bus = I2CBus::getInstance();

    bus.openBus("/dev/i2c-1");

    BME280 bme280;

    bus.attachDevice(BME280_DEVICE_NAME, bme280);

    bme280.readTPH(&tph);

    printf("Temperature: %.2f\n", tph.temperature - ((getCPUTemp() - tph.temperature) / TEMP_COMPENSATION_FACTOR));
    printf("Pressure: %.2f\n", tph.pressure);
    printf("Humidity: %.2f\n", tph.humidity);

    bus.closeBus();

    return 0;
}
