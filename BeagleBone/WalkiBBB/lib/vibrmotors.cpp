#include "vibrmotors.h"

/**
 * @brief Constructor initializing pair of vibrating motors and setting their duty cycle to zero
 * @param paradigm Any member of the Paradigm enum
 * @param leftMotor Any member of the Pwm_Device enum, representing the pwm pin of the BBB connected to the left motor
 * @param rightMotor Any member of the Pwm_Device enum, representing the pwm pins of the BBB connected to the right motor
 */
VibrMotors::VibrMotors(Paradigm paradigm, Pwm_Device leftMotor, Pwm_Device rightMotor):
    paradigm(paradigm), leftMotor(leftMotor), rightMotor(rightMotor)
{
    this->leftMotor.setDuty(0.0f);
    this->rightMotor.setDuty(0.0f);
}

/**
 * @brief Converts an input signal representing a certain quality of balance to the control signal for the vibrating motors
 * The current implementation uses the fact that setDuty limits its input argument to a value between 0 and 1, therefore
 * setting negative values to 0.
 * @param leftInpupt
 * @param rightInput
 */

void VibrMotors::setSignal(float leftsignal, float rightsignal)
{
    switch (paradigm)
    {
    case Paradigm::Intensity:
        leftMotor.setDuty(leftsignal);
        rightMotor.setDuty(rightsignal);
    }

}
