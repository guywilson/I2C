#include "i2c.h"

#ifndef _INCL_BME280
#define _INCL_BME280

#define BME280_DEVICE_NAME          "BME280"
#define BME280_BUS_ADDRESS          0X76

#define BME280_CHIP_ID              0x60

#define BME280_RESET_CMD            0xB6

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
    int32_t                     temperatureCalibration = 0;

    I2CBus & bus = I2CBus::getInstance();

    I2CRegister8bit *               _regReset;
    I2CRegister8bit *               _regStatus;
    I2CRegister8bit *               _regConfig;
    I2CRegister8bit *               _regChipID;
    I2CRegister8bit *               _regCtrlMeasure;
    I2CRegister8bit *               _regCtrlHumidity;
    I2CRegisterBlock *              _regData;
    I2CRegisterBlock *              _regCompensation1;
    I2CRegisterBlock *              _regCompensation2;

    double getCompensatedTemperature(int32_t adcInput);
    double getCompensatedPressure(int32_t adcInput);
    double getCompensatedHumidity(int32_t adcInput);

    void resetDevice();

public:
    BME280();
    virtual ~BME280();

    enum power_mode {
        pow_sleep =     0b00000000,
        pow_forced =    0b00000001,
        pow_normal =    0b00000011
    };

    enum osrs_p {
        pos_skipped =   0b00000000,
        pos_1 =         0b00000100,
        pos_2 =         0b00001000,
        pos_4 =         0b00001100,
        pos_8 =         0b00010000,
        pos_16 =        0b00010100
    };

    enum osrs_t {
        tos_skipped =   0b00000000,
        tos_1 =         0b00100000,
        tos_2 =         0b01000000,
        tos_4 =         0b01100000,
        tos_8 =         0b10000000,
        tos_16 =        0b10100000
    };

    enum osrs_h {
        hos_skipped =   0b00000000,
        hos_1 =         0b00000001,
        hos_2 =         0b00000010,
        hos_4 =         0b00000011,
        hos_8 =         0b00000100,
        hos_16 =        0b00000101
    };

    enum t_sb {
        t_sb_0_5 =      0b00000000,
        t_sb_62_5 =     0b00100000,
        t_sb_125 =      0b01000000,
        t_sb_500 =      0b10000000,
        t_sb_1000 =     0b10100000,
        t_sb_10 =       0b11000000,
        t_sb_20 =       0b11100000
    };

    enum filter {
        filter_off =    0b00000000,
        filter_2 =      0b00000100,
        filter_4 =      0b00001000,
        filter_8 =      0b00001100,
        filter_16 =     0b00010000
    };

    virtual void    initialise();

    void            setMode(power_mode mode);
    void            setPressureOversampling(osrs_p pos);
    void            setTemperatureOversampling(osrs_t tos);
    void            setHumidityOversampling(osrs_h hos);
    void            setFilterCoefficient(filter f);
    void            setStandbyTime(t_sb tm);

    bool            isChipIDValid();
    bool            isNVMCopyInProgress();
    bool            isMeasuring();

    void            getData(BME280_TPH * tph);
};

#endif
