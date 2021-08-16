#ifndef DEF_LIB_VIBRATOR_H
#define DEF_LIB_VIBRATOR_H

#include "peripheral.h"
#include "../drivers/gpio.h"
#include "filters/lowpassfilter.h"

/**
 * @defgroup Vibrator Vibrator
 * @brief Manages a PWM-driven vibrator.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Class to manage a PWM-driven vibrator.
 * The Vibrator class provides methods to manage vibrators easily, such as
 * open-loop speed control (speed to PWM conversion with an approximate model)
 * and startup from zero speed.
 */
class Vibrator : public Peripheral
{
public:
    Vibrator(Gpio_Id gpioDevice);
    ~Vibrator();
    void update(float dt) override;
    void setSpeed(float targetPwmDuty);

private:
    Gpio gpio; ///< GPIO device.
    float remainingStartingTime; ///< Remaining starting time [s].
    float targetSpeed; ///< Target vibrator speed [0.0-1.0].
    LowPassFilter estimatedSpeed; ///< Estimated vibrator speed [0.0-1.0].
    std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdateTime; ///< Time of the last call to update(), to compute the dt.
    bool command; ///< Vibrator command (gpio state): false=OFF, true=ON.
};

/**
 * @}
 */

#endif
