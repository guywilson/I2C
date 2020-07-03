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

void I2CDevice::setBus(I2CBus * bus)
{
    this->bus = bus;
}

void I2CDevice::addRegister(const char * name, I2CRegister * reg)
{
    this->registers[name] = reg;
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

void I2CDevice::readBlock(uint8_t address, uint8_t * data, uint32_t datalength)
{
    bus->busWrite(&address, 1);
    bus->busRead(data, datalength);
}

void I2CDevice::writeRegister(const char * name, uint8_t value)
{
    uint8_t     address;

    I2CRegister * r = this->registers[name];
    printf("Got register object with name %s\n", r->getName());

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busWrite(&value, 1);
}

void I2CDevice::writeRegister(const char * name, uint16_t value)
{
    uint8_t     address;

    I2CRegister * r = this->registers[name];

    address = r->getAddress();

    bus->busWrite(&address, 1);
    bus->busWrite(&value, 2);
}

void I2CDevice::writeBlock(uint8_t address, uint8_t * data, uint32_t datalength)
{
    bus->busWrite(&address, 1);
    bus->busWrite(data, datalength);
}
