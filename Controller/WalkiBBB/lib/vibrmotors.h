#ifndef VIBRMOTORS_H
#define VIBRMOTORS_H

#include "../drivers/pwm.h"

/**
 * @brief The Paradigm enum lists haptic paradigms used to convert a numeric value
 * into a comprehensible haptic signal
 */
enum Paradigm
{
    Intensity = 0
};

/**
 * @brief The VibrMotors class sets up two vibrating motors and translates an input
 * signal describing the balance of gait into the control signal for these motors,
 * using a chosen paradigm.
 */
class VibrMotors
{
public:
    VibrMotors(Paradigm paradigm, Pwm_Device leftMotor, Pwm_Device rightMotor);
    void setSignal(float leftsignal, float rightsignal);

private:
    Paradigm paradigm; ///< Variable describing the paradigm
    Pwm leftMotor, ///< left vibrating motor
        rightMotor; ///< right vibrating motor


};

#endif // VIBRMOTORS_H
