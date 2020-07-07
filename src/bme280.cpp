#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "i2c_error.h"
#include "i2c.h"
#include "bme280.h"

BME280::BME280() : I2CDevice(BME280_DEVICE_NAME, BME280_BUS_ADDRESS)
{
    memset(&compensationData, 0x00, sizeof(BME280_COMPENSATIONDATA));

    _regReset = new I2CRegister8bit(this, BME280_REG_RESET_NAME, BME280_REG_RESET_ADDRESS);
    _regChipID = new I2CRegister8bit(this, BME280_REG_CHIPID_NAME, BME280_REG_CHIPID_ADDRESS);
    _regStatus = new I2CRegister8bit(this, BME280_REG_STATUS_NAME, BME280_REG_STATUS_ADDRESS);
    _regConfig = new I2CRegister8bit(this, BME280_REG_CONFIG_NAME, BME280_REG_CONFIG_ADDRESS);
    _regCtrlMeasure = new I2CRegister8bit(this, BME280_REG_CTRLMEAS_NAME, BME280_REG_CTRLMEAS_ADDRESS);
    _regCtrlHumidity = new I2CRegister8bit(this, BME280_REG_CTRLHUM_NAME, BME280_REG_CTRLHUM_ADDRESS);
    _regData = new I2CRegisterBlock(this, BME280_REG_DATA_NAME, BME280_REG_DATA_ADDRESS);
    _regCompensation1 = new I2CRegisterBlock(this, BME280_REG_COMP1_NAME, BME280_REG_COMP1_ADDRESS);
    _regCompensation2 = new I2CRegisterBlock(this, BME280_REG_COMP2_NAME, BME280_REG_COMP2_ADDRESS);

    addRegister(_regReset);
    addRegister(_regChipID);
    addRegister(_regStatus);
    addRegister(_regConfig);
    addRegister(_regCtrlMeasure);
    addRegister(_regCtrlHumidity);
    addRegister(_regData);
    addRegister(_regCompensation1);
    addRegister(_regCompensation2);
}

BME280::~BME280()
{
    delete _regCompensation2;
    delete _regCompensation1;
    delete _regData;
    delete _regCtrlHumidity;
    delete _regCtrlMeasure;
    delete _regConfig;
    delete _regStatus;
    delete _regChipID;
    delete _regReset;
}

void BME280::resetDevice()
{
    _regReset->write(BME280_RESET_CMD);

    while (isNVMCopyInProgress()) {
        usleep(2000L);
    }
}

void BME280::initialise()
{
    uint8_t                     buf[64];
    int                         i = 0;

    printf("Initialising BME280\n");

    while (i < 5) {
        if (isChipIDValid()) {
            /*
            ** Reset the device...
            */
            resetDevice();

            /*
            ** Read compensation data, part 1 & 2...
            */
            _regCompensation1->read(buf, 25);
            _regCompensation2->read(&buf[25], 8);

            memcpy(&compensationData, buf, sizeof(BME280_COMPENSATIONDATA));

            break;
        }

        usleep(1000L);
        i++;
    }

    if (i == 5) {
        throw i2c_error("Could not find BME280 on the I2C bus", __FILE__, __LINE__);
    }

    setFilterCoefficient(filter_4);

    setHumidityOversampling(hos_1);
    setPressureOversampling(pos_1);
    setTemperatureOversampling(tos_1);
}

void BME280::setMode(power_mode mode)
{
    _regCtrlMeasure->setBits(0x03, mode);
}

void BME280::setPressureOversampling(osrs_p pos)
{
    _regCtrlMeasure->setBits(0xC1, pos);
}

void BME280::setTemperatureOversampling(osrs_t tos)
{
    _regCtrlMeasure->setBits(0xE0, tos);
}

void BME280::setHumidityOversampling(osrs_h hos)
{
    _regCtrlHumidity->setBits(0x07, hos);
}

void BME280::setStandbyTime(t_sb tm)
{
    _regConfig->setBits(0xE0, tm);
}

void BME280::setFilterCoefficient(filter f)
{
    _regConfig->setBits(0x1C, f);
}

bool BME280::isChipIDValid()
{
    return (_regChipID->read() == BME280_CHIP_ID);
}

bool BME280::isNVMCopyInProgress()
{
    return ((_regStatus->read() & 0x01) == 0x01);
}

bool BME280::isMeasuring()
{
    return ((_regStatus->read() & 0x08) == 0x08);
}

void BME280::getData(BME280_TPH * tph)
{
    uint8_t                     buf[64];
    int32_t                     pressure;
    int32_t                     temperature;
    int32_t                     humidity;

    setMode(pow_forced);

    usleep(200000L);

    /*
    ** Read raw temperature, pressure and humidity...
    */
    _regData->read(buf, 8);

    /*
    ** Reset the filter...
    */
    setFilterCoefficient(filter_4);

    /*
    ** Get raw values...
    */
    pressure = (buf[0] << 12) + (buf[1] << 4) + ((buf[2] >> 4) & 0x0F);
    temperature = (buf[3] << 12) + (buf[4] << 4) + ((buf[5] >> 4) & 0x0F);
    humidity = (buf[6] << 8) + (buf[7]);

    tph->temperature = getCompensatedTemperature(temperature);
    tph->pressure = getCompensatedPressure(pressure);
    tph->humidity = getCompensatedHumidity(humidity);

    printf("Temperature: %d, 0x%08X, %.2f\n", temperature, temperature, tph->temperature);
    printf("Pressure: %d, 0x%08X, %.2f\n", pressure, pressure, tph->pressure);
    printf("Humidity: %d, 0x%04X, %.2f\n", humidity, humidity, tph->humidity);
}

double BME280::getCompensatedTemperature(int32_t adcInput)
{
    double var1;
    double var2;
    double temperature;
    double temperature_min = -40;
    double temperature_max = 85;

    var1 = ((double)adcInput) / 16384.0 - ((double)compensationData.temperature1) / 1024.0;
    var1 = var1 * ((double)compensationData.temperature2);

    var2 = (((double)adcInput) / 131072.0 - ((double)compensationData.temperature1) / 8192.0);
    var2 = (var2 * var2) * ((double)compensationData.temperature3);

    temperatureCalibration = (int32_t)(var1 + var2);

    temperature = (var1 + var2) / 5120.0;

    if (temperature < temperature_min) {
        temperature = temperature_min;
    }
    else if (temperature > temperature_max) {
        temperature = temperature_max;
    }

    return temperature;
}

double BME280::getCompensatedPressure(int32_t adcInput)
{
    double var1;
    double var2;
    double var3;
    double pressure;
    double pressure_min = 30000.0;
    double pressure_max = 110000.0;

    var1 = ((double)temperatureCalibration / 2.0) - 64000.0;

    var2 = var1 * var1 * ((double)compensationData.pressure6) / 32768.0;
    var2 = var2 + var1 * ((double)compensationData.pressure5) * 2.0;
    var2 = (var2 / 4.0) + (((double)compensationData.pressure4) * 65536.0);

    var3 = ((double)compensationData.pressure3) * var1 * var1 / 524288.0;

    var1 = (var3 + ((double)compensationData.pressure2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)compensationData.pressure1);

    /* avoid exception caused by division by zero */
    if (var1 > (0.0)) {
        pressure = 1048576.0 - (double)adcInput;
        pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;

        var1 = ((double)compensationData.pressure9) * pressure * pressure / 2147483648.0;

        var2 = pressure * ((double)compensationData.pressure8) / 32768.0;

        pressure = pressure + (var1 + var2 + ((double)compensationData.pressure7)) / 16.0;

        if (pressure < pressure_min) {
            pressure = pressure_min;
        }
        else if (pressure > pressure_max) {
            pressure = pressure_max;
        }
    }
    else {
        throw i2c_error("Invalid pressure reading, cannot compute", __FILE__, __LINE__);
    }

    return pressure / 100;
}

double BME280::getCompensatedHumidity(int32_t adcInput)
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

    var1 = ((double)temperatureCalibration) - (double)76800.0;
    var2 = (((double)compensationData.humidity4) * (double)64.0 + (((double)compensationData.humidity5) / (double)16384.0) * var1);
    var3 = (double)adcInput - var2;
    var4 = ((double)compensationData.humidity2) / (double)65536.0;
    var5 = ((double)1.0 + (((double)compensationData.humidity3) / (double)67108864.0) * var1);
    var6 = (double)1.0 + (((double)compensationData.humidity6) / (double)67108864.0) * var1 * var5;
    var6 = var3 * var4 * (var5 * var6);

    humidity = var6 * var6 * ((double)1.0 - (double)compensationData.humidity1) / (double)524288.0;

    if (humidity > humidity_max) {
        humidity = humidity_max;
    }
    else if (humidity < humidity_min) {
        humidity = humidity_min;
    }

    return humidity;
}
