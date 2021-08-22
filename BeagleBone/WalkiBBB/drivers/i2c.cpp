#include "i2c.h"
#include "../config/config.h"
#include "../lib/utils.h"
#include "../lib/bytebuffer.h"

using namespace std;

#define BUFFERS_CAPACITY 32

const string I2C_OVERLAYS[] =
{
    "BB-I2C1",
    "BB-I2C2"
};

const string I2C_SCL_PINS[] =
{
    "P9.17",
    "P9.19"
};

const string I2C_SDA_PINS[] =
{
    "P9.18",
    "P9.20"
};

const string I2C_PATHS[] =
{
#if DEBIAN_VERSION == 7
    "/dev/i2c-2",
    "/dev/i2c-3"
#elif DEBIAN_VERSION == 8
    "/dev/i2c-1",
    "/dev/i2c-2"
#endif
};

/**
 * @brief Constructor.
 * @param bus the I2C bus to use.
 */
I2c::I2c(I2cBus bus)
{
#ifdef __arm__
    // Set the pins muxing.
    try
    {
        Utils::loadOverlay(I2C_OVERLAYS[bus], I2C_PATHS[bus]);
    }
    catch(runtime_error)
    {
        state = PeripheralState::FAULT;
        return;
    }

    // Open the I2C bus.
    filename = I2C_PATHS[bus];

    file = open(filename.c_str(), O_RDWR);

    if(file < 0) // Error.
    {
        state = PeripheralState::FAULT;
        return;
    }

    state = PeripheralState::ACTIVE;

    //
    rxBuffer.reserve(BUFFERS_CAPACITY);
#else
    unused(bus);
    file = 0;
    state = PeripheralState::DISABLED;
#endif
}

/**
 * @brief I2c destructor.
 */
I2c::~I2c()
{
    close(file);
}

/**
 * @brief Directly writes the given bytes to a slave device.
 * @param slaveAddress 7-bit slave address (the read/write bit will be added).
 * @param data the bytes to write to the slave.
 */
void I2c::rawWrite(uint8_t slaveAddress, const std::vector<uint8_t> &data)
{
    // Wait until the I2C device is available.
    lock_guard<mutex> deviceLocker(deviceMutex);

    // Set slave address.
    int status = ioctl(file, I2C_SLAVE, slaveAddress);

    if(status < 0)
        throw runtime_error(string("I2C: error while setting slave address.\n"));

    // Start the transfer.
    status = ::write(file, data.data(), data.size());

    if(status < 0)
        throw runtime_error(string("I2C: error while writing.\n"));
}

/**
 * @brief Directly reads bytes from a slave device.
 * @param slaveAddress 7-bit slave address (the read/write bit will be added).
 * @param length number of bytes to read.
 * @return The bytes read.
 */
const std::vector<uint8_t> &I2c::rawRead(uint8_t slaveAddress, int length)
{
    rxBuffer.resize(length);

    // Wait until the I2C device is available.
    lock_guard<mutex> deviceLocker(deviceMutex);

    // Set slave address.
    int status = ioctl(file, I2C_SLAVE, slaveAddress);

    if(status < 0)
        throw runtime_error(string("I2C: error while setting slave address.\n"));

    // Start the transfer.
    status = ::read(file, rxBuffer.data(), length);

    if(status < 0)
        throw runtime_error(string("I2C: error while reading.\n"));

    return rxBuffer;
}

/**
 * @brief Writes the given data byte to a 8-bit register.
 * @param slaveAddress 7-bit slave address (the read/write bit will be added).
 * @param registerAddress 8-bit register address.
 * @param data data byte to write.
 */
void I2c::writeRegister(uint8_t slaveAddress, uint8_t registerAddress,
                        uint8_t data)
{
    vector<uint8_t> txBuffer;
    txBuffer.push_back(registerAddress);
    txBuffer.push_back(data);

    rawWrite(slaveAddress, txBuffer);
}

/**
 * @brief Writes the given data bytes to a register.
 * @param slaveAddress 7-bit slave address (the read/write bit will be added).
 * @param registerAddress 8-bit register address.
 * @param data data bytes to write.
 */
void I2c::writeRegister(uint8_t slaveAddress, uint8_t registerAddress,
                        const vector<uint8_t> &data)
{
    ByteBuffer txBuffer;
    txBuffer << registerAddress << data;

    rawWrite(slaveAddress, txBuffer);
}

/**
 * @brief Read the content of a simple 8-bit register.
 * @param slaveAddress 7-bit slave address (the read/write bit will be added).
 * @param registerAddress 8-bit register address.
 * @return The byte read.
 */
uint8_t I2c::readRegister(uint8_t slaveAddress, uint8_t registerAddress)
{
    // TODO: maybe need to implement "repeated start condition" instead.

    // Write the register address.
    rawWrite(slaveAddress, {registerAddress});

    // Read the data bytes from the slave.
    return rawRead(slaveAddress, 1).front();
}

/**
 * @brief Read the content of a multi-bytes register.
 * @param slaveAddress: 7-bit slave address (the read/write bit will be added).
 * @param registerAddress: 8-bit register address.
 * @param length number of bytes to read.
 * @return The bytes read.
 */
const std::vector<uint8_t> &I2c::readRegister(uint8_t slaveAddress,
                                              uint8_t registerAddress,
                                              int length)
{
    // TODO: maybe need to implement "repeated start condition" instead.

    // Write the register address.
    rawWrite(slaveAddress, {registerAddress});

    // Read the data bytes from the slave.
    return rawRead(slaveAddress, length);
}
