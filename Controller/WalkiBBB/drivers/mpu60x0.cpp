#include "mpu60x0.h"

#include <limits>
#include <iostream>

#include "../lib/debugstream.h"

using namespace std;

//#define CONFIG_SPEED_HZ 1000000
//#define NORMAL_SPEED_HZ 20000000
#define CONFIG_SPEED_HZ 1000000
#define NORMAL_SPEED_HZ 1000000

#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_SMPLRT_DIV   0x19
#define REG_WHO_AM_I     0x75
#define REG_ACCEL_VALUES 0x3b
#define REG_GYRO_VALUES  0x43
#define REG_TEMP_VALUES  0x41

#define WHO_AM_I_VAL 0x68

#define GRAVITY_INTENSITY 9.81f

const float SINT16_MAX_F = (float)numeric_limits<int16_t>::max();

const float MPU_ACCEL_RANGE_REG_TO_CONV_FACTOR[] =
{
     2.0f / SINT16_MAX_F * GRAVITY_INTENSITY,
     4.0f / SINT16_MAX_F * GRAVITY_INTENSITY,
     8.0f / SINT16_MAX_F * GRAVITY_INTENSITY,
    16.0f / SINT16_MAX_F * GRAVITY_INTENSITY
};

const float MPU_GYRO_RANGE_REG_TO_CONV_FACTOR[] =
{
     250.0f / SINT16_MAX_F,
     500.0f / SINT16_MAX_F,
    1000.0f / SINT16_MAX_F,
    2000.0f / SINT16_MAX_F
};

/**
 * @brief Constructor.
 * @param accelRange accelerometer range.
 * @param gyroRange gyroscope range.
 * @param configFileName filename of the config file. It will not be loaded if
 * "" is given.
 */
Mpu60X0::Mpu60X0(Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
                 string configFileName) :
    Imu(configFileName)
{
    //
    this->accelConversionFactor = MPU_ACCEL_RANGE_REG_TO_CONV_FACTOR[accelRange];
    this->gyroConversionFactor= MPU_GYRO_RANGE_REG_TO_CONV_FACTOR[gyroRange];
}

/**
 * @brief Acquires all from the MPU-60X0.
 * Acquires the acceleration and the angular speed on the three axes, and the
 * temperature.
 */
void Mpu60X0::readAll()
{
    try
    {
        vector<uint8_t> arr(14);
        readRegisters(REG_ACCEL_VALUES, arr);

        // Interpret the acceleration.
        uint16_t axMsb = arr[0];
        uint16_t axLsb = arr[1];
        uint16_t ayMsb = arr[2];
        uint16_t ayLsb = arr[3];
        uint16_t azMsb = arr[4];
        uint16_t azLsb = arr[5];

        rawAcceleration.x = (float)(int16_t)((axMsb<<8) | axLsb);
        rawAcceleration.y = (float)(int16_t)((ayMsb<<8) | ayLsb);
        rawAcceleration.z = (float)(int16_t)((azMsb<<8) | azLsb);

        rawAcceleration *= accelConversionFactor;

        // Interpret the temperature.
        uint16_t tMsb = arr[6];
        uint16_t tLsb = arr[7];
        temperature = ((float)(int16_t)((tMsb<<8) | tLsb)) / 340.0f + 36.53f; // Coefs from the Register map spec.

        // Interpret the angular speed.
        uint16_t gxMsb = arr[8];
        uint16_t gxLsb = arr[9];
        uint16_t gyMsb = arr[10];
        uint16_t gyLsb = arr[11];
        uint16_t gzMsb = arr[12];
        uint16_t gzLsb = arr[13];

        rawAngularSpeed.x = (float)(int16_t)((gxMsb<<8) | gxLsb);
        rawAngularSpeed.y = (float)(int16_t)((gyMsb<<8) | gyLsb);
        rawAngularSpeed.z = (float)(int16_t)((gzMsb<<8) | gzLsb);

        rawAngularSpeed *= gyroConversionFactor;
    }
    catch(runtime_error&)
    {
        // TODO: count the frequency to set the sensor as FAULT when there are
        // too many errors.
        debug << "MPU-60X0: communication error." << endl;
    }
}

/**
 * @brief Initializes a MPU-60X0.
 * @param accelRange accelerometer range.
 * @param gyroRange gyroscope range.
 * @param bandwidth bandwidth of the low-pass filter.
 */
void Mpu60X0::init(Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
                   Mpu_Bandwidth bandwidth)
{
    // Reset the MPU-60X0.
    writeRegister(0x6B, (1<<7)|(1<<6)); // Device reset.
    this_thread::sleep_for(chrono::milliseconds(100));

    // Test the communication with the MPU-60X0.
    uint16_t id = readRegister(REG_WHO_AM_I);

    if(id != WHO_AM_I_VAL)
    {
        state = PeripheralState::FAULT;
        return;
    }

    // Reset the MPU-60X0.
    writeRegister(0x6B, (1<<7)|(1<<6)); // Device reset.
    this_thread::sleep_for(chrono::milliseconds(100));

    writeRegister(0x68, (1<<2)|(1<<1)|(1<<0)); // Signal path reset.
    this_thread::sleep_for(chrono::milliseconds(100));

    // Setup the MPU-60X0 registers.
    writeBit(0x6a, 1, 4); // Disable the I2C slave interface.
    writeBit(0x6b, 0, 6); // Disable sleep.

    writeBits(0x6b, 3, 0, 3); // PLL with Z axis gyro ref.

    writeBit(0x6b, false, 3); // Enable temperature sensor.

    writeBits(REG_GYRO_CONFIG, gyroRange, 3, 2); // Gyro range.
    writeBits(REG_ACCEL_CONFIG, accelRange, 3, 2); // Accelero range.

    writeBits(REG_CONFIG, bandwidth, 2, 3); // Low-pass filter bandwidth.
    writeRegister(REG_SMPLRT_DIV, 0); // Set the sampling clock divider to 1 kHz. TODO: change.

    // Sensor ready.
    state = PeripheralState::ACTIVE;
}

/**
 * @brief Writes a single bit in the specified register.
 * @param address the address of the register to write.
 * @param newValue the new bit value.
 * @param bitIndex the bit index.
 */
void Mpu60X0::writeBit(uint8_t address, bool newValue, uint8_t bitIndex)
{
    uint8_t registerValue = readRegister(address);

    if(newValue)
        registerValue |= (1<<bitIndex);
    else
        registerValue &= ~(1<<bitIndex);

    writeRegister(address, registerValue);
}

/**
 * @brief Reads a single bit in the specified register.
 * @param address the address of the register to write.
 * @param bitIndex the bit index.
 * @return the bit value.
 */
bool Mpu60X0::readBit(uint8_t address, uint8_t bitIndex)
{
    uint8_t registerValue = readRegister(address);

    return (registerValue & (1<<bitIndex));
}

/**
 * @brief Writes a group of contiguous bits in the specified register.
 * @param address register address.
 * @param newValue new value of the bits group.
 * @param startBitIndex index of the first bit to modify.
 * @param nBits size of the bits group (number of bits to modify).
 */
void Mpu60X0::writeBits(uint8_t address, uint8_t newValue,
                        uint8_t startBitIndex, uint8_t nBits)
{
    uint8_t registerValue = readRegister(address);
    uint8_t endBitIndex = startBitIndex + nBits - 1;

    newValue =  (newValue << startBitIndex);

    for(int i=startBitIndex; i<=endBitIndex; i++)
    {
        if(newValue & (1<<i))
            registerValue |= (1<<i);
        else
            registerValue &= ~(1<<i);
    }

    writeRegister(address, registerValue);
}

/**
 * @brief Reads a group of contiguous bits in the specified register.
 * @param address register address.
 * @param startBitIndex index of the first bit to read.
 * @param nBits size of the bits group (number of bits to modify).
 * @return
 */
uint8_t Mpu60X0::readBits(uint8_t address, uint8_t startBitIndex, uint8_t nBits)
{
    uint8_t registerValue = readRegister(address);

    return ((registerValue >> startBitIndex) & ~(0xff<<nBits));
}

/**
 * @brief Constructor.
 * @param spi SPI bus to communicate with the MPU-6000.
 * @param chipselect SPI chipselect pin corresponding to the MPU-6000.
 * @param accelRange accelerometer range.
 * @param gyroRange gyroscope range.
 * @param bandwidth bandwidth of the low-pass filter.
 * @param configFileName filename of the config file. It will not be loaded if
 * "" is given.
 */
Mpu6000::Mpu6000(SpiChannel &spi, Mpu_AccelRange accelRange,
                 Mpu_GyroRange gyroRange, Mpu_Bandwidth bandwidth,
                 string configFileName) :
    Mpu60X0(accelRange, gyroRange, configFileName), spi(spi)
{
    // Initialize the MPU-6000, if the SPI bus is OK.
    if(spi.getBus().getState() == PeripheralState::ACTIVE)
    {
        spi.setSpeed(CONFIG_SPEED_HZ);
        spi.setMode(SPI_MODE_3);
        init(accelRange, gyroRange, bandwidth);
    }
}

/**
 * @brief Mpu6000 destructor.
 */
Mpu6000::~Mpu6000()
{
    stopAutoUpdate();
}

/**
 * @brief Initializes the MPU-6000.
 * @param accelRange accelerometer range.
 * @param gyroRange gyroscope range.
 * @param bandwidth bandwidth of the low-pass filter.
 */
void Mpu6000::init(Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
                   Mpu_Bandwidth bandwidth)
{
    spi.setSpeed(CONFIG_SPEED_HZ);
    Mpu60X0::init(accelRange, gyroRange, bandwidth);
    spi.setSpeed(NORMAL_SPEED_HZ);
}

/**
 * @brief Reads a register.
 * @param address register address.
 * @return register value.
 */
uint8_t Mpu6000::readRegister(uint8_t address)
{
    return spi.readRegister(address);
}

/**
 * @brief Writes to a register.
 * @param address register address.
 * @param newValue value to write to the register.
 */
void Mpu6000::writeRegister(uint8_t address, uint8_t newValue)
{
    spi.writeRegister(address, newValue);
}

/**
 * @brief Reads a n-bits register.
 * @param address register address.
 * @param rxData buffer to write the read bytes to. The number of bytes read
 * is equal to the size of rxData.
 */
void Mpu6000::readRegisters(uint8_t address, std::vector<uint8_t> &rxData)
{
    rxData = spi.readRegister(address, rxData.size());
}

/**
 * @brief Constructor.
 * @param i2c I2C bus to communicate with the MPU-6050.
 * @param slaveAddress slave address of the MPU-6050.
 * @param accelRange accelerometer range.
 * @param gyroRange gyroscope range.
 * @param bandwidth bandwidth of the low-pass filter.
 * @param configFileName filename of the config file. It will not be loaded if
 * "" is given.
 */
Mpu6050::Mpu6050(I2c &i2c, uint8_t slaveAddress, Mpu_AccelRange accelRange,
                 Mpu_GyroRange gyroRange, Mpu_Bandwidth bandwidth,
                 string configFileName) :
    Mpu60X0(accelRange, gyroRange, configFileName), i2c(i2c)
{
    this->slaveAddress = slaveAddress;

    // Initialize the MPU-6050, if the I2C bus is OK.
    try
    {
        if(i2c.getState() == PeripheralState::ACTIVE)
        {
            init(accelRange, gyroRange, bandwidth);
            state = PeripheralState::ACTIVE;
        }
        else
            state = PeripheralState::DISABLED;
    }
    catch(runtime_error&)
    {
        state = PeripheralState::DISABLED;
    }
}

/**
 * @brief Mpu6050 destructor.
 */
Mpu6050::~Mpu6050()
{
    stopAutoUpdate();
}

/**
 * @brief Reads a register.
 * @param address register address.
 * @return register value.
 */
uint8_t Mpu6050::readRegister(uint8_t address)
{
    try
    {
        return i2c.readRegister(slaveAddress, address);
    }
    catch(runtime_error&)
    {
        throw runtime_error("MPU-6050: I2C error.");
    }
}

/**
 * @brief Writes a register.
 * @param address register address.
 * @param newValue value to write.
 */
void Mpu6050::writeRegister(uint8_t address, uint8_t newValue)
{ 
    try
    {
        i2c.writeRegister(slaveAddress, address, newValue);
    }
    catch(runtime_error&)
    {
        throw runtime_error("MPU-6050: I2C error.");
    }
}


/**
 * @brief Reads a n-bits register.
 * @param address register address.
 * @param rxData buffer to write the read bytes to. The number of bytes read
 * is equal to the size of rxData.
 */
void Mpu6050::readRegisters(uint8_t address, std::vector<uint8_t> &rxData)
{
    rxData = i2c.readRegister(slaveAddress, address, rxData.size());
}
