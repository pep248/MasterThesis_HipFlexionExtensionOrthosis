#ifndef DEF_DRIVERS_SPI_H
#define DEF_DRIVERS_SPI_H

#include <vector>
#include <string>
#include <stdexcept>
#include <mutex>

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gpio.h"
#include "../lib/peripheral.h"

/**
 * @defgroup SPI SPI
 * @brief Classes to control the SPI bus.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief SPI bus manager.
 * C++ wrapper around the Linux Spidev userspace SPI driver.
 */
class SpiBus : public Peripheral
{
public:
    const static Gpio_Id CS_LEFT_ADC, /// GPIO connected to the left foot ADC chipselect.
                         CS_RIGHT_ADC, ///< GPIO connected to the right foot ADC chipselect.
                         CS_LEFT_FOOT_MPU, ///< GPIO connected to the left foot IMU chipselect.
                         CS_RIGHT_FOOT_MPU, ///< GPIO connected to the right foot IMU chipselect.
                         CS_EXT_1, ///< Additional CS.
                         CS_EXT_2, ///< Additional CS.
                         CS_EXT_3, ///< Additional CS.
                         CS_EXT_4; ///< Additional CS.

    SpiBus();
    ~SpiBus();

    void rawTransfer(const Gpio &chipSelect,
                     const std::vector<uint8_t> txData,
                     std::vector<uint8_t> &rxData, int speed, uint8_t mode);

private:
    bool setSpiMode(uint8_t mode);

    int file; ///< C-style device file handle to control the SPI bus.
    std::mutex deviceMutex; ///< Mutex to avoid that two threads try to control the device at the same time.
    uint8_t currentSpiMode; ///< Current SPI mode (0, 1, 2 or 3).
};

/**
 * @brief Convenience class to use a single channel of the SPI bus.
 * @remark All the SpiChannels corresponding to each device connected to the SPI
 * bus must be constructed, even if they are unused. Otherwise, the
 * corresponding chipselect pins of the BeagleBone will remain in the high-Z
 * state, which will result in a low (active) state after the differential
 * transducers. This will cause the unused devices to interfere during
 * communication with the others.
 */
class SpiChannel
{
public:
    SpiChannel(SpiBus& spiBus, Gpio_Id chipselect,
               int speed = 1000000, uint8_t spiMode = SPI_MODE_0);

    SpiBus& getBus();
    void setSpeed(int speed);
    void setMode(uint8_t spiMode);

    void rawTransfer(const std::vector<uint8_t> txData,
                     std::vector<uint8_t> &rxData);
    void writeRegister(uint8_t registerAddress, uint8_t data);
    void writeRegister(uint8_t registerAddress,
                       const std::vector<uint8_t> &data);
    uint8_t readRegister(uint8_t registerAddress);
    const std::vector<uint8_t>& readRegister(uint8_t registerAddress,
                                             int length);

private:
    SpiBus& spiBus;
    Gpio chipselect;
    int speed;
    uint8_t spiMode;
    std::vector<uint8_t> txBuffer, ///< Pre-allocated buffer to send data.
                         rxBuffer; ///< Pre-allocated buffer to receive data.
};

/**
 * @}
 */

#endif
