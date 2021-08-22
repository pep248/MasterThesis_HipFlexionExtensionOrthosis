#ifndef DEF_LIB_BEEPER_H
#define DEF_LIB_BEEPER_H

#include "peripheral.h"
#include "../drivers/pwm.h"

#include <tuple>
#include <list>

/**
 * @defgroup Beeper Beeper
 * @brief Makes beeps with a PWM-driven loudspeaker.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Structure representing a beep sound.
 */
struct Beep
{
    float frequency, ///< Beep frequency [Hz].
          strength, ///< Beep strength [0.0-1.0].
          duration; ///< Beep duration [s].
};

/**
 * @brief Makes beeps with a PWM-driven loudspeaker.
 * @note Due to the method used to drive the loudspeaker, the volume (strength)
 * control is approximative and also influences the sound tone.
 * @ingroup Lib
 */
class Beeper : public Peripheral
{
public:
    Beeper(Pwm_Device pwmDevice);
    ~Beeper();
    void beep(float frequency, float strength, float duration);
    void playSequence(std::list<Beep> beepSequence);
    void update(float dt);

private:
    void setPwm(float frequency, float strength, float duration);

    Pwm pwm; ///< Pwm device to control the loudspeaker.
    float beepRemainingTime; ///< Remaining time before the beep ends [s].
    std::list<Beep> beepsQueue;
};

/**
 * @}
 */

#endif
