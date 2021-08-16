#include "as5048b.h"

#include "../lib/debugstream.h"

using namespace std;

/**
 * @brief Constructor.
 * @param i2c I2C bus to communicate with the AS5048B.
 * @param a1PinState true if the A1 pin is in high state, false otherwise. This
 * is used to compute the I2C slave address.
 * @param a2PinStatetrue if the A2 pin is in high state, false otherwise. This
 * is used to compute the I2C slave address.
 */
As5048b::As5048b(I2c &i2c, bool a1PinState, bool a2PinState) : i2c(i2c)
{
    slaveAddress = (1<<6) | (((uint8_t)a2PinState) << 1) | ((uint8_t)a1PinState);

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("angle", "deg", angle, VarAccess::READ,
                                   false));

    // Check that the chip is working correctly.
    try
    {
        uint8_t addressRegValue = i2c.readRegister(slaveAddress, 0x15);

        debug << "Expected slave address: " << (int)slaveAddress
              << ". Read slave address:" << (int)addressRegValue << endl;

        state = PeripheralState::ACTIVE;
    }
    catch(runtime_error &e)
    {
        debug << "Could not communicate with the AS5048B: " << e.what() << endl;
        state = PeripheralState::DISABLED;
    }
}

/**
 * @brief Reads the angle measured by the sensor.
 * @param dt time elapsed since the last call to this method [s].
 */
void As5048b::update(float)
{
    if(state != PeripheralState::ACTIVE)
        return;

    try
    {
        vector<uint8_t> rawData = i2c.readRegister(slaveAddress, 0xfe, 2);
        angle = ((float)((rawData[1]<<6) | rawData[0])) / 16383.0f * 360.0f;
    }
    catch(runtime_error &err)
    {
        //debug << "AS5048B error: " << err.what() << endl;
    }
}

/**
 * @brief Gets the last angle read from the sensor.
 * @return the measured angle [deg].
 */
const float &As5048b::getAngle() const
{
    return angle;
}
