#include "i2c.h"

#ifndef _INCL_BME280
#define _INCL_BME280

typedef struct
{
    double      temperature;
    double      pressure;
    double      humidity;
}
BME_TPH;

class BME280 : public I2CDevice
{
private:
    float getCompensatedTemperature(int32_t adcInput);
    float getCompensatedPressure(int32_t adcInput);
    float getCompensatedHumidity(int32_t adcInput);
    
public:
    BME280();

    void readTPH(BME_TPH tph);
};

#endif
