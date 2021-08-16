#include "hmc5883l.h"

#include <iostream>

using namespace std;

// 7-bit slave address.
#define SLAVE_ADDRESS (0x3c>>1)

// Register addresses.
#define REG_CONFIG_A 0
#define REG_CONFIG_B 1
#define REG_MODE 2
#define REG_OUT_X_MSB 3
#define REG_OUT_X_LSB 4
#define REG_OUT_Z_MSB 5
#define REG_OUT_Z_LSB 6
#define REG_OUT_Y_MSB 7
#define REG_OUT_Y_LSB 8
#define REG_STATUS 9
#define REG_ID_A 10
#define REG_ID_B 11
#define REG_ID_C 12

// Configuration register A constants.
#define CRA_AVERAGING_1 (0x0<<5)
#define CRA_AVERAGING_2 (0x1<<5)
#define CRA_AVERAGING_4 (0x2<<5)
#define CRA_AVERAGING_8 (0x3<<5)

#define CRA_OUTPUT_RATE_0_75HZ (0x0<<2)
#define CRA_OUTPUT_RATE_1_5HZ (0x1<<2)
#define CRA_OUTPUT_RATE_3HZ (0x2<<2)
#define CRA_OUTPUT_RATE_7_5HZ (0x3<<2)
#define CRA_OUTPUT_RATE_15HZ (0x4<<2)
#define CRA_OUTPUT_RATE_30HZ (0x5<<2)
#define CRA_OUTPUT_RATE_75HZ (0x6<<2)

#define CRA_MODE_NORMAL 0x0
#define CRA_MODE_POS_BIAS 0x1
#define CRA_MODE_NEG_BIAS 0x2

// Configuration register B constants.
#define CRB_GAIN_1370 (0x0<<5) // +- 0.88 G.
#define CRB_GAIN_1090 (0x1<<5) // +- 1.3 G.
#define CRB_GAIN_820 (0x2<<5) // +- 1.9 G.
#define CRB_GAIN_660 (0x3<<5) // +- 2.5 G.
#define CRB_GAIN_440 (0x4<<5) // +- 4.0 G.
#define CRB_GAIN_390 (0x5<<5) // +- 4.7 G.
#define CRB_GAIN_330 (0x6<<5) // +- 5.6 G.
#define CRB_GAIN_230 (0x7<<5) // +- 8.1 G.

// Mode register constants.
#define MR_HIGH_SPEED_I2C (1<<7)
#define MR_MODE_CONTINUOUS 0x0
#define MR_MODE_SINGLE 0x1
#define MR_MODE_IDLE 0x2

// Status register constants.
#define SR_DATA_READY (1<<0)
#define SR_LOCK (1<<1)

// Identification register constants.
#define IRA_VAL 'H'
#define IRB_VAL '4'
#define IRC_VAL '3'

// Gain register value to sensitivity table [G].
const float REG_TO_SENSITIVITY[8] = { 0.00073f, 0.00092f, 0.00122f, 0.00152f, 0.00227f, 0.00256f, 0.00303f, 0.00435f };

/**
 * @brief Constructor.
 * @param i2c I2C bus to communicate with the HMC5883L.
 */
Hmc5883l::Hmc5883l(I2c &i2c) : i2cBus(i2c)
{
    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("x", "G", lastSample.x, VarAccess::READ,
                                   false));
    syncVars.push_back(makeSyncVar("y", "G", lastSample.y, VarAccess::READ,
                                   false));
    syncVars.push_back(makeSyncVar("z", "G", lastSample.z, VarAccess::READ,
                                   false));

    //
    try
    {
        // Check that the HMC5883L is responding on the I2C bus, by checking its
        // ID registers.
        i2cBus.rawWrite(SLAVE_ADDRESS, {REG_ID_A});
        vector<uint8_t> id = i2cBus.rawRead(SLAVE_ADDRESS, 3);

        if((id[0] != IRA_VAL) || (id[1] != IRB_VAL) || (id[2] != IRC_VAL))
        {
            state = PeripheralState::FAULT;
            return;
        }

        // Setup - configuration register A.
        uint8_t crA = CRA_AVERAGING_8 | CRA_OUTPUT_RATE_15HZ | CRA_MODE_NORMAL;
        i2cBus.writeRegister(SLAVE_ADDRESS, REG_CONFIG_A, crA);

        vector<uint8_t> t = i2cBus.readRegister(SLAVE_ADDRESS, REG_CONFIG_A, 1);

        // Setup - configuration register B.
        // TODO: let the class user select the sensitivity.
        uint8_t crB = CRB_GAIN_1090;
        i2cBus.writeRegister(SLAVE_ADDRESS, REG_CONFIG_B, crB);
        sensitivity = REG_TO_SENSITIVITY[crB>>5];

        t = i2cBus.readRegister(SLAVE_ADDRESS, REG_CONFIG_B, 1);

        // Setup - mode register.
        uint8_t mr = MR_MODE_CONTINUOUS;
        i2cBus.writeRegister(SLAVE_ADDRESS, REG_MODE, mr);

        t = i2cBus.readRegister(SLAVE_ADDRESS, REG_MODE, 1);

        // Sensor ready.
        state = PeripheralState::ACTIVE;
    }
    catch(runtime_error&)
    {
        // Error with the I2C bus.
        state = PeripheralState::FAULT;
    }
}

/**
 * @brief Acquires a magnetic field vector from the HMC5883L.
 */
void Hmc5883l::update(float)
{
    if(state != PeripheralState::ACTIVE)
        return;

    try
    {
        // Are measurements ready ? Read the status register.
        vector<uint8_t> status = i2cBus.readRegister(SLAVE_ADDRESS, REG_STATUS,
                                                     1);

        if(status[0] & SR_DATA_READY) // Data is ready.
        {
            // Read the data output registers.
            vector<uint8_t> output = i2cBus.readRegister(SLAVE_ADDRESS,
                                                         REG_OUT_X_MSB, 6);

            int16_t rawX = (int16_t)((((uint16_t)output[0]) << 8) |
                                      ((uint16_t)output[1]));
            int16_t rawY = (int16_t)((((uint16_t)output[2]) << 8) |
                                      ((uint16_t)output[3]));
            int16_t rawZ = (int16_t)((((uint16_t)output[4]) << 8) |
                                      ((uint16_t)output[5]));

            // Convert to Gauss.
            lastSample.setValue((float)rawX, (float)rawY, (float)rawZ);
            lastSample *= sensitivity;
        }
    }
    catch(runtime_error&)
    {
        // Error with the I2C bus.
        state = PeripheralState::FAULT;
    }
}

/**
 * @brief Gets the last measured magnetic field vector.
 * @return The last measured magnetic field vector [G].
 * @note This function does not perfom the sensor acquisition, so it necessary
 * to call acquire() before.
 */
Vec3f Hmc5883l::get() const
{
    return lastSample;
}
