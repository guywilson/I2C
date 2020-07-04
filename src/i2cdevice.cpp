#include <stdio.h>
#include <stdint.h>

#include <map>
#include <vector>
#include <string>

#include "i2c.h"

using namespace std;

I2CDevice::I2CDevice(const char * name, uint8_t busAddress)
{
    this->name.assign(name);
    this->address = busAddress;
}

void I2CDevice::addRegister(I2CRegister * reg)
{
    this->registers[reg->getName()] = reg;
}

void I2CDevice::removeRegister(const char * name)
{
    this->registers.erase(name);
}

uint8_t I2CDevice::readRegister8(const char * name)
{
    uint8_t     value;
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busRead(&value, 1);

    return value;
}

uint16_t I2CDevice::readRegister16(const char * name)
{
    uint16_t    value;
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busRead(&value, 2);

    return value;
}

uint32_t I2CDevice::readRegister32(const char * name)
{
    uint32_t    value;
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busRead(&value, 4);

    return value;
}

void I2CDevice::readBlock(uint8_t address, uint8_t * data, uint32_t datalength)
{
    bus->busWrite(&address, 1);
    bus->busRead(data, datalength);
}

void I2CDevice::writeRegister8(const char * name, uint8_t value)
{
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busWrite(&value, 1);
}

void I2CDevice::writeRegister16(const char * name, uint16_t value)
{
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busWrite(&value, 2);
}

void I2CDevice::writeRegister32(const char * name, uint32_t value)
{
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busWrite(&value, 4);
}

void I2CDevice::writeBlock(uint8_t address, uint8_t * data, uint32_t datalength)
{
    bus->busWrite(&address, 1);
    bus->busWrite(data, datalength);
}
