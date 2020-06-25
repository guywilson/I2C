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

public:
    BME280();

    void readTPH(BME_TPH tph);
};

#endif
