#include "i2c.h"

#ifndef _INCL_BME280
#define _INCL_BME280

#define BME280_DEVICE_NAME          "BME280"
#define BME280_BUS_ADDRESS          0X76

#define BME280_REG_RESET_NAME       "Reset"
#define BME280_REG_RESET_ADDRESS    0xE0

#define BME280_REG_CHIPID_NAME      "Chip ID"
#define BME280_REG_CHIPID_ADDRESS   0xD0

#define BME280_REG_CTRLMEAS_NAME    "Ctrl Measure"
#define BME280_REG_CTRLMEAS_ADDRESS 0xF4

#define BME280_REG_CTRLHUM_NAME     "Ctrl Humidity"
#define BME280_REG_CTRLHUM_ADDRESS  0xF2

#define BME280_REG_CONFIG_NAME      "Config"
#define BME280_REG_CONFIG_ADDRESS   0xF5

#define BME280_REG_STATUS_NAME      "Status"
#define BME280_REG_STATUS_ADDRESS   0xF3

#define BME280_REG_DATA_NAME        "Data"
#define BME280_REG_DATA_ADDRESS     0xF7

#define BME280_REG_COMP1_NAME       "Compensation 1"
#define BME280_REG_COMP1_ADDRESS    0x88

#define BME280_REG_COMP2_NAME       "Compensation 2"
#define BME280_REG_COMP2_ADDRESS    0xE1

typedef struct
{
    double      temperature;
    double      pressure;
    double      humidity;
}
BME280_TPH;

typedef struct __attribute__((__packed__)) 
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

    I2CBus & bus = I2CBus::getInstance();

    I2CRegister8bit *               reset;
    I2CRegister8bit *               status;
    I2CRegister8bit *               config;
    I2CRegister8bit *               chipID;
    I2CRegister8bit *               ctrlMeasure;
    I2CRegister8bit *               ctrlHumidity;
    I2CRegisterBlock *              data;
    I2CRegisterBlock *              compensation1;
    I2CRegisterBlock *              compensation2;

    double getCompensatedTemperature(int32_t adcInput);
    double getCompensatedPressure(int32_t adcInput);
    double getCompensatedHumidity(int32_t adcInput);

public:
    BME280();
    ~BME280();

    void initialise();

    void readTPH(BME280_TPH * tph);
};

#endif
