#ifndef DEF_DRIVERS_LEDSTATUSINDICATOR_H
#define DEF_DRIVERS_LEDSTATUSINDICATOR_H

#include "../lib/peripheral.h"
#include "gpio.h"

/**
 * @defgroup LedStatusIndicator LED status indicator
 * @brief Driver to interact with the RGB LED PCB.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Structure describing a RGB LED pulse.
 */
struct LedPulse
{
    bool red, ///< Red LED enabled.
         green, ///< Green LED enabled.
         blue; ///< Blue LED enabled.
    float duration; ///< Duration [s].
};

/**
 * @brief The LedStatusIndicator class
 */
class LedStatusIndicator : public Peripheral
{
public:
    LedStatusIndicator();
    void update(float dt) override;

    void enableControl();
    void disableControl();

    void setNormalColor(bool red, bool green, bool blue);
    void playSequence(std::vector<LedPulse> pulses);

private:
    void setColor(bool red, bool green, bool blue);

    Gpio redGpio, greenGpio, blueGpio, enableGpio; ///< GPIOs that control the RGB LED.
    bool normalRedState, normalGreenState, normalBlueState; ///< "Normal" state of the RGB LED, set by setColor(). After a sequence is played, the color will revert to this "normal" state.
    std::vector<LedPulse> remainingPulses; ///< List of color pulses to play.
};

/**
 * @}
 */

#endif
