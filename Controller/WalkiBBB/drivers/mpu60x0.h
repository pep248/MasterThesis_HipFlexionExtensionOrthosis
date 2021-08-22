#ifndef DEF_DRIVERS_MPU60X0_H
#define DEF_DRIVERS_MPU60X0_H

#include <chrono>
#include <thread>

#include "i2c.h"
#include "spi.h"
#include "../lib/imu.h"

/**
 * @defgroup MPU-6050 MPU-6050
 * @brief Invensense MPU-60X0 6-axis IMU.
 * @ingroup Drivers
 * @{
 */

#define MPU_I2C_ADDRESS_LOW 0x68 ///< I2C slave address of the MPU-6050, if the AD0 pin is driven low.
#define MPU_I2C_ADDRESS_HIGH 0x69 ///< I2C slave address of the MPU-6050, if the AD0 pin is driven high.

/**
 * @brief MPU-60X0 accelerometer ranges enumeration.
 */
enum Mpu_AccelRange
{
    MPU_ACCEL_RANGE_2G  = 0,
    MPU_ACCEL_RANGE_4G  = 1,
    MPU_ACCEL_RANGE_8G  = 2,
    MPU_ACCEL_RANGE_16G = 3
};

/**
 * @brief MPU-60X0 gyroscope ranges enumeration.
 */
enum Mpu_GyroRange
{
    MPU_GYRO_RANGE_250DPS  = 0,
    MPU_GYRO_RANGE_500DPS  = 1,
    MPU_GYRO_RANGE_1000DPS = 2,
    MPU_GYRO_RANGE_2000DPS = 3
};

/**
 * @brief MPU-60X0 low-pass filter bandwidthes enumeration.
 */
enum Mpu_Bandwidth
{
    MPU_DLPF_BW_256HZ = 0,
    MPU_DLPF_BW_188HZ = 1,
    MPU_DLPF_BW_98HZ  = 2,
    MPU_DLPF_BW_42HZ  = 3,
    MPU_DLPF_BW_20HZ  = 4,
    MPU_DLPF_BW_10HZ  = 5,
    MPU_DLPF_BW_5HZ   = 6
};

/**
 * @brief Driver for the Invensense MPU-60X0 6-axis IMU.
 * This abstract class can represent either a MPU-6000 or a MPU-6050, which are
 * behaving the same way, except that the first one has a SPI interface, and the
 * second one has a I2C interface. The operation and the registers are the same.
 *
 * Each Mpu60X0 object has its own updating thread, so the get*() methods return
 * immediately, and no call to an update() function is needed. However, every
 * class inheriting Mpu60X0 must call stopAutoUpdate() at the first line of its
 * destructor, to avoid a segmentation fault.
 */
class Mpu60X0 : public Imu
{
public:
    Mpu60X0(Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
            std::string configFileName);

protected:
    void readAll() override;

    /**
     * @brief Reads a 8-bit register.
     * @param address register address.
     * @return the value read from the register.
     */
    virtual uint8_t readRegister(uint8_t address) = 0;

    /**
     * @brief Writes a 8-bit register.
     * @param address register address.
     * @param newValue the value to write to the register.
     */
    virtual void writeRegister(uint8_t address, uint8_t newValue) = 0;

    /**
     * @brief Reads a n-bits register.
     * @param address register address.
     * @param rxData buffer to write the read bytes to. The number of bytes read
     * is equal to the size of rxData.
     */
    virtual void readRegisters(uint8_t address,
                               std::vector<uint8_t>& rxData) = 0;

    virtual void init(Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
                      Mpu_Bandwidth bandwidth);
    void writeBit(uint8_t address, bool newValue, uint8_t bitIndex);
    bool readBit(uint8_t address, uint8_t bitIndex);
    void writeBits(uint8_t address, uint8_t newValue,
                   uint8_t startBitIndex, uint8_t nBits);
    uint8_t readBits(uint8_t address, uint8_t startBitIndex, uint8_t nBits);

    float accelConversionFactor; ///< Accelerometer registers to acceleration conversion factor [m/s^2].
    float gyroConversionFactor;  ///< Gyroscope registers to angular speed conversion factor [deg/s].
};

/**
 * @brief Driver for the Invensense MPU-6000 6-axis IMU, with SPI interface.
 */
class Mpu6000 : public Mpu60X0
{
public:
    Mpu6000(SpiChannel &spi, Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
            Mpu_Bandwidth bandwidth, std::string configFileName = "");
    virtual ~Mpu6000();

protected:
    virtual void init(Mpu_AccelRange accelRange, Mpu_GyroRange gyroRange,
                      Mpu_Bandwidth bandwidth);
    virtual uint8_t readRegister(uint8_t address);
    virtual void writeRegister(uint8_t address, uint8_t newValue);
    virtual void readRegisters(uint8_t address, std::vector<uint8_t>& rxData);

private:
    SpiChannel &spi; ///< SPI bus to communicate with the MPU-6000.
};

/**
 * @brief Driver for the Invensense MPU-6000 6-axis IMU, with I2C interface.
 */
class Mpu6050 : public Mpu60X0
{
public:
    Mpu6050(I2c &i2c, uint8_t slaveAddress, Mpu_AccelRange accelRange,
            Mpu_GyroRange gyroRange, Mpu_Bandwidth bandwidth,
            std::string configFileName = "");
    virtual ~Mpu6050();

protected:
    virtual uint8_t readRegister(uint8_t address);
    virtual void writeRegister(uint8_t address, uint8_t newValue);
    virtual void readRegisters(uint8_t address, std::vector<uint8_t>& rxData);

private:
    I2c &i2c; ///< I2C bus to communicate with the MPU-6050.
    uint8_t slaveAddress; ///< I2C slave address of the MPU-6050.
};

/**
 * @}
 */

#endif
