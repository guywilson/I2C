#include <iostream>
#include <string>

#include <stdio.h>

#include "i2c.h"
#include "bme280.h"
#include "ltr559.h"

using namespace std;

#define TEMP_COMPENSATION_FACTOR            2.25
#define I2C_DEVICE_NAME                     "/dev/i2c-1"

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

    try {
        bus.openBus(I2C_DEVICE_NAME);

        printf("Opened bus...\n");

        BME280 *    bme280 = new BME280();
        LTR559 *    ltr559 = new LTR559();

        bus.attachDevice(bme280);
        bus.attachDevice(ltr559);

        bme280->initialise();
        ltr559->initialise();

        bme280->readTPH(&tph);

        double lightLevel = ltr559->readLux();

        printf("Temperature: %.2f\n", (tph.temperature - ((getCPUTemp() - tph.temperature) / TEMP_COMPENSATION_FACTOR)));
        printf("Pressure: %.2f\n", tph.pressure);
        printf("Humidity: %.2f\n", tph.humidity);
        printf("Light level: %.2f\n\n", lightLevel);

        delete ltr559;
        delete bme280;
        
        bus.closeBus();
    }
    catch (i2c_error & e) {
        fprintf(stderr, "Caught exception: %s", e.what());
        exit(-1);
    }

    return 0;
}
