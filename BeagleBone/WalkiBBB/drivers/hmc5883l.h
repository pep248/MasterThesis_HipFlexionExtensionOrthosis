#ifndef DEF_DRIVERS_HMC5883L_H
#define DEF_DRIVERS_HMC5883L_H

#include <stdexcept>
#include <chrono>
#include <thread>

#include "i2c.h"
#include "../lib/peripheral.h"
#include "../lib/vec3.h"

/**
 * @defgroup HMC5883L HMC5883L
 * @brief Honeywell HMC5883L 3-axis magnetometer.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Driver for the Honeywell HMC5883L 3-axis magnetometer.
 */
class Hmc5883l : public Peripheral
{
public:
    Hmc5883l(I2c &i2c);
    void update(float dt);
    Vec3f get() const;

private:
    Vec3f lastSample; ///< Last measured magnetic field vector [G].
    I2c &i2cBus; ///< I2C bus for communication.
    float sensitivity; ///< Factor for conversion from chip register to magnetic field [G].
};

/**
 * @}
 */

#endif
