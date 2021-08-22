#ifndef DEF_DRIVERS_I2C_H
#define DEF_DRIVERS_I2C_H

#include <vector>
#include <string>
#include <stdexcept>
#include <mutex>

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../lib/peripheral.h"

/**
 * @defgroup I2C I2C
 * @brief BeagleBone Black I2C bus.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief I2C buses enumeration.
 */
enum I2cBus
{
    I2C_BUS_SENSORS = 0,
    I2C_BUS_EEPROM
};

/**
 * @brief I2C bus manager.
 * C++ wrapper around the Linux userspace I2C driver.
 */
class I2c : public Peripheral
{
public:
    I2c(I2cBus bus);
    ~I2c();

    void rawWrite(uint8_t slaveAddress, const std::vector<uint8_t> &data);
    const std::vector<uint8_t>& rawRead(uint8_t slaveAddress, int length);

    void writeRegister(uint8_t slaveAddress, uint8_t registerAddress, uint8_t data);
    void writeRegister(uint8_t slaveAddress, uint8_t registerAddress, const std::vector<uint8_t> &data);
    uint8_t readRegister(uint8_t slaveAddress, uint8_t registerAddress);
    const std::vector<uint8_t>& readRegister(uint8_t slaveAddress, uint8_t registerAddress, int length);

private:
    std::string filename; ///< Filename of the I2C device file.
    int file; ///< C-style device file handler.
    std::vector<uint8_t> rxBuffer; ///< Pre-allocated buffer for receiving data.
    std::mutex deviceMutex; ///< Mutex to avoid that two threads try to control the device at the same time.
};

/**
 * @}
 */

#endif
