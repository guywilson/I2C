#include <iostream>
#include <string>

#include <stdio.h>

#include "i2c.h"
#include "bme280.h"
#include "ltr559.h"
#include "avrsnd.h"

using namespace std;

#define TEMP_COMPENSATION_FACTOR            2.25
#define I2C_DEVICE_NAME                     "/dev/i2c-1"

bool loop = true;

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
    int             i = 0;
    //BME280_TPH      tph;

    I2CBus & bus = I2CBus::getInstance();

    try {
        bus.openBus(I2C_DEVICE_NAME);

        printf("Opened bus...\n");

        // BME280::operation_mode opMode = BME280::mode_weather_monitoring;

        // LTR559_ALS::ALS_int_time  t = LTR559_ALS::int_t_100;
        // LTR559_ALS::ALS_meas_rate m = LTR559_ALS::mr_200;
        // LTR559_ALS::ALS_gain g = LTR559_ALS::alsg_4;

        // BME280 *        bme280 = new BME280(opMode);
        // LTR559_ALS *    ltr559 = new LTR559_ALS(t, m, g);
        AVRSound * avrsnd = new AVRSound(256);

        // bus.attachDevice(bme280);
        // bus.attachDevice(ltr559);
        bus.attachDevice(avrsnd);

        while (loop) {
            // bme280->getData(&tph);

            // double lightLevel = ltr559->readLux();

            // double correctedTemperature = (tph.temperature - ((getCPUTemp() - tph.temperature) / TEMP_COMPENSATION_FACTOR));

            // double dewPoint = tph.temperature - (((double)100.0 - tph.humidity) / (double)5.0);
            // double humidity = (double)100.0 - ((double)5.0 * (correctedTemperature - dewPoint));

            // printf("\nTemperature: %.2f\n", correctedTemperature);
            // printf("Pressure: %.2f\n", tph.pressure);
            // printf("Humidity: %.2f\n", humidity);
            // printf("Light level: %.2f\n\n", lightLevel);
            printf("Sound level = %s\n", avrsnd->getLoudnessDescription());

            usleep(5000000L);

            i++;

            if (i == 12) {
                loop = false;
            }
        }

        delete avrsnd;
        // delete ltr559;
        // delete bme280;
        
        bus.closeBus();
    }
    catch (i2c_error & e) {
        fprintf(stderr, "Caught exception: %s", e.what());
        exit(-1);
    }

    return 0;
}
