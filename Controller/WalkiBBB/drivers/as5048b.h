#ifndef DEF_DRIVERS_AS4048B_H
#define DEF_DRIVERS_AS4048B_H

#include "../lib/peripheral.h"
#include "i2c.h"

/**
 * @defgroup AS5048B AS5048B
 * @brief AS5048B I2C magnetic encoder.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief AS5048B I2C magnetic encoder.
 */
class As5048b : public Peripheral
{
public:
    As5048b(I2c &i2c, bool a1PinState, bool a2PinState);
    void update(float dt) override;
    const float &getAngle() const;

private:
    I2c &i2c; ///< I2C bus.
    uint8_t slaveAddress; ///< I2C peripheral slave address.
    float angle; ///< Last measured angle [deg].
};

/**
 * @}
 */

#endif
