#ifndef DRIVERS_PWM_H
#define DRIVERS_PWM_H

#include <string>
#include <fstream>

#include "../lib/peripheral.h"

/**
 * @defgroup PWM PWM
 * @brief BeagleBone Black PWM.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief PWM channels enumeration.
 */
enum Pwm_Device
{
    PWM_1A = 0,
    PWM_1B,
    PWM_2A,
    PWM_2B
};

/**
 * @brief PWM channel manager.
 * C++ wrapper around the Linux userspace PWM driver.
 */
class Pwm : public Peripheral
{
public:
    Pwm(Pwm_Device device);
    ~Pwm();
    void setFrequency(float frequency);
    void setDuty(float dutyRatio);
    float getDuty() const;

private:
    Pwm_Device pwmDevice; ///< Associated PWM device.
    std::ofstream dutyFile; ///< Device file to control the PWM duty.
    std::ofstream periodFile; ///< Device file to control the PWM period.
    float dutyRatio; ///< Current PWM duty [0.0-1.0].
    int period; ///< Current PWM period [ns].
};

/**
 * @}
 */

#endif
