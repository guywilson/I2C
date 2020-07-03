#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <map>
#include <vector>
#include <string>

#include "i2c_error.h"
#include "i2c.h"

using namespace std;

I2CBus::~I2CBus()
{
    if (this->_busFd) {
        closeBus();
    }
}

void I2CBus::openBus(const char * pszDeviceName)
{
    int fd = open(pszDeviceName, O_RDWR);

    if (fd < 0) {
        throw i2c_error(i2c_error::buildMsg("Failed to open bus device %s: %s", pszDeviceName, strerror(errno)), __FILE__, __LINE__);
    }

    this->_busFd = fd;
}

void I2CBus::closeBus()
{
    if (this->_busFd) {
        close(this->_busFd);

        this->_busFd = 0;
    }
}

void I2CBus::attachDevice(I2CDevice * device)
{
    this->devices[device->getName()] = device;

    printf("Attached device %s\n", device->getName());

    device->setBus(this);
}

void I2CBus::detachDevice(const char * name)
{
    this->devices.erase(name);
}

void I2CBus::busWrite(void * data, uint32_t dataLength)
{
    uint32_t bytesWritten = write(this->_busFd, data, dataLength);

    if (bytesWritten < dataLength) {
        throw i2c_error(i2c_error::buildMsg("Failed to write %ld bytes to bus: %s", dataLength, strerror(errno)), __FILE__, __LINE__);
    }        
}

void I2CBus::busRead(void * data, uint32_t dataLength)
{
    uint32_t bytesRead = read(this->_busFd, data, dataLength);

    if (bytesRead < dataLength) {
        throw i2c_error(i2c_error::buildMsg("Failed to read %ld bytes from bus: %s", dataLength, strerror(errno)), __FILE__, __LINE__);
    }
}

void I2CBus::acquire(const char * deviceName)
{
    if (pthread_mutex_trylock(&mutex)) {
        throw i2c_error(i2c_error::buildMsg("Failed to acquire bus for device %s, it is already locked", deviceName), __FILE__, __LINE__);
    }
    printf("Acquired mutex lock\n");

    this->lock = 1;

    I2CDevice * dev = this->devices[deviceName];
    printf("Got device object for device %s\n", dev->getName());
    
    int err = ioctl(this->_busFd, I2C_WORKER, dev->getAddress());

    if (err) {
        throw i2c_error(i2c_error::buildMsg("Failed to acquire I2C bus access for device %s: %s", deviceName, strerror(errno)), __FILE__, __LINE__);
    }
}

void I2CBus::release(const char * deviceName)
{
    pthread_mutex_unlock(&mutex);
}

I2CDevice * I2CBus::getDevice(const char * name)
{
    return this->devices[name];
}
