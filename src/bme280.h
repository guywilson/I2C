#include "i2c.h"

#ifndef _INCL_BME280
#define _INCL_BME280

#define BME280_DEVICE_NAME          "BME280"
#define BME280_BUS_ADDRESS          0X76

typedef struct
{
    double      temperature;
    double      pressure;
    double      humidity;
}
BME280_TPH;

typedef struct
{
    uint16_t    temperature1;
    int16_t     temperature2;
    int16_t     temperature3;

    uint16_t    pressure1;
    int16_t     pressure2;
    int16_t     pressure3;
    int16_t     pressure4;
    int16_t     pressure5;
    int16_t     pressure6;
    int16_t     pressure7;
    int16_t     pressure8;
    int16_t     pressure9;

    uint8_t     humidity1;
    int16_t     humidity2;
    uint8_t     humidity3;
    int16_t     humidity4;
    int16_t     humidity5;
    int8_t      humidity6;
}
BME280_COMPENSATIONDATA;

class BME280 : public I2CDevice
{
private:
    BME280_COMPENSATIONDATA     compensationData;
    int32_t                     temperatureCalibration;

    double getCompensatedTemperature(int32_t adcInput);
    double getCompensatedPressure(int32_t adcInput);
    double getCompensatedHumidity(int32_t adcInput);

public:
    BME280();

    void readTPH(BME280_TPH * tph);
};

#endif
