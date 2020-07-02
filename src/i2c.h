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

class I2CRegister
{
private:
    uint8_t         address;
    string          name;
    
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

class I2CDevice
{
private:
    string                      name;
    uint8_t                     address;
    I2CBus *                    bus;
    map<string, I2CRegister*>   registers;

protected:
    void            addRegister(const char * name, I2CRegister * reg);
    void            removeRegister(const char * name);

public:
    I2CDevice() {}
    I2CDevice(const char * name, uint8_t busAddress);

    uint8_t         getAddress() {
        return this->address;
    }

    const char *    getName() {
        return this->name.c_str();
    }

    void setBus(I2CBus * bus);

    uint8_t         readRegister8(const char * name);
    uint16_t        readRegister16(const char * name);
    void            readBlock(uint8_t address, uint8_t * data, uint32_t datalength);

    void            writeRegister(const char * name, uint8_t value);
    void            writeRegister(const char * name, uint16_t value);
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

#endif
