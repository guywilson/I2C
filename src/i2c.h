#include <string>
#include <map>
#include <vector>

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

using namespace std;

#include "i2c_error.h"

#ifndef _INCL_I2CBUS
#define _INCL_I2CBUS

#define I2C_WORKER          0x0703

class I2CBus;
class I2CDevice;
class I2CRegister;

class I2CDevice
{
private:
    string                      name;
    uint8_t                     address;
    I2CBus *                    bus;
    map<string, I2CRegister*>   registers;

protected:
    void            addRegister(I2CRegister * reg);
    void            removeRegister(const char * name);

public:
    I2CDevice() {}
    I2CDevice(const char * name, uint8_t busAddress);

    virtual ~I2CDevice() {}

    uint8_t         getAddress() {
        return this->address;
    }

    const char *    getName() {
        return this->name.c_str();
    }

    I2CBus * getBus() {
        return this->bus;
    }
    void setBus(I2CBus * bus) {
        this->bus = bus;
    }

    virtual void    initialise() = 0;

    uint8_t         readRegister8(const char * name);
    uint16_t        readRegister16(const char * name);
    uint32_t        readRegister32(const char * name);
    void            readBlock(uint8_t address, uint8_t * data, uint32_t datalength);

    void            writeRegister8(const char * name, uint8_t value);
    void            writeRegister16(const char * name, uint16_t value);
    void            writeRegister32(const char * name, uint32_t value);
    void            writeBlock(uint8_t address, uint8_t * data, uint32_t datalength);
};

class I2CBus
{
friend class I2CDevice;

public:
    static I2CBus & getInstance() {
        static I2CBus instance;
        return instance;
    }

private:
    I2CBus() {}

    int                         _busFd = 0;
    pthread_mutex_t             mutex;
    map<string, I2CDevice*>     devices;
    uint8_t                     lock = 0;

protected:
    void busWrite(void * data, uint32_t dataLength);
    void busRead(void * data, uint32_t dataLength);

    int getBusFileDesc() {
        return this->_busFd;
    }

public:
    ~I2CBus();

    void openBus(const char * pszDeviceName);
    void closeBus();

    void attachDevice(I2CDevice * device);
    void detachDevice(const char * name);

    void acquire(const char * deviceName);
    void release(const char * deviceName);

    I2CDevice * getDevice(const char * name);
};

class I2CRegister
{
private:
    I2CDevice *     device;
    uint8_t         address;
    string          name;

protected:
    void setDevice(I2CDevice * device) {
        this->device = device;
    }

    I2CDevice * getDevice() {
        return this->device;
    }

    void acquireBus() {
        getDevice()->getBus()->acquire(getDevice()->getName());
    }

    void releaseBus() {
        getDevice()->getBus()->release(getDevice()->getName());
    }

public:
    I2CRegister() {}
    I2CRegister(const char * name, uint8_t address) {
        this->name.assign(name);
        this->address = address;
    }

    uint8_t getAddress() {
        return this->address;
    }

    const char * getName() {
        return this->name.c_str();
    }
};

class I2CRegisterBlock : public I2CRegister
{
public:
    I2CRegisterBlock(I2CDevice * device) : I2CRegister() {
        setDevice(device);
    }
    I2CRegisterBlock(I2CDevice * device, const char * name, uint8_t address) : I2CRegister(name, address) {
        setDevice(device);
    }

    void read(uint8_t * data, uint32_t dataLength) {
        acquireBus();
        getDevice()->readBlock(getAddress(), data, dataLength);
        releaseBus();
    }
    void write(uint8_t * data, uint32_t dataLength) {
        acquireBus();
        getDevice()->writeBlock(getAddress(), data, dataLength);
        releaseBus();
    }
};

class I2CRegister32bit : public I2CRegister
{
public:
    I2CRegister32bit(I2CDevice * device) : I2CRegister() {
        setDevice(device);
    }
    I2CRegister32bit(I2CDevice * device, const char * name, uint8_t address) : I2CRegister(name, address) {
        setDevice(device);
    }

    uint32_t    read() {
        uint32_t        value;

        acquireBus();
        value = getDevice()->readRegister32(getName());
        releaseBus();

        return value;
    }
    void        write(uint32_t data) {
        acquireBus();
        getDevice()->writeRegister32(getName(), data);
        releaseBus();
    }
};

class I2CRegister16bit : public I2CRegister
{
public:
    I2CRegister16bit(I2CDevice * device) : I2CRegister() {
        setDevice(device);
    }
    I2CRegister16bit(I2CDevice * device, const char * name, uint8_t address) : I2CRegister(name, address) {
        setDevice(device);
    }

    uint16_t    read() {
        uint16_t        value;

        acquireBus();
        value = getDevice()->readRegister16(getName());
        releaseBus();

        return value;
    }
    void        write(uint16_t data) {
        acquireBus();
        getDevice()->writeRegister16(getName(), data);
        releaseBus();
    }
};

class I2CRegister8bit : public I2CRegister
{
public:
    I2CRegister8bit(I2CDevice * device) : I2CRegister() {
        setDevice(device);
    }
    I2CRegister8bit(I2CDevice * device, const char * name, uint8_t address) : I2CRegister(name, address) {
        setDevice(device);
    }

    uint8_t    read() {
        uint8_t        value;

        acquireBus();
        value = getDevice()->readRegister8(getName());
        releaseBus();

        return value;
    }
    void        write(uint8_t data) {
        acquireBus();
        getDevice()->writeRegister8(getName(), data);
        releaseBus();
    }
};

#endif
