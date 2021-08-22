#include "ledstatusindicator.h"

/**
 * @brief Constructor.
 */
LedStatusIndicator::LedStatusIndicator() : redGpio(GPIO2_3), greenGpio(GPIO2_5),
    blueGpio(GPIO2_4), enableGpio(GPIO2_2)
{
    redGpio.setupAsOutput();
    greenGpio.setupAsOutput();
    blueGpio.setupAsOutput();

    redGpio.setPinState(false);
    greenGpio.setPinState(false);
    blueGpio.setPinState(false);

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("red", "", normalRedState,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("green", "", normalGreenState,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("blue", "", normalBlueState,
                                   VarAccess::READWRITE, false));
}

/**
 * @brief Updates the module.
 * This method should be called often, otherwise the LED PCB will go red (stall
 * state).
 * @param dt time elapsed since the last call to this method [s].
 */
void LedStatusIndicator::update(float dt)
{
    // Toogle the enable pin.
    enableGpio.setPinState(!enableGpio.getPinState());

    // Update the color, in case a pulses sequence is playing.
    if(remainingPulses.empty())
        setColor(normalRedState, normalGreenState, normalBlueState);
    else
    {
        LedPulse &currentPulse = remainingPulses.front();
        setColor(currentPulse.red, currentPulse.green, currentPulse.blue);
        currentPulse.duration -= dt;

        if(currentPulse.duration <= 0.0f)
            remainingPulses.erase(remainingPulses.begin());
    }
}

/**
 * @brief Enables the color control of the LED.
 */
void LedStatusIndicator::enableControl()
{
    enableGpio.setupAsOutput();
}

/**
 * @brief Disables the control of the LED.
 * Disables the color control of the LED, so that it does not turn red when
 * the update() function is not called periodically.
 * Typically, this method should be called before the normal exit of the
 * program, to avoid that the LED turns red (error).
 */
void LedStatusIndicator::disableControl()
{
    enableGpio.setupAsInput();
}

/**
 * @brief Sets the "normal" color of the LED.
 * The color will be set permanently until this method is called again or if a
 * pulses sequence (initiated by playSequence()) is being played. After the
 * sequence is played, the LED color will turn back to the last "normal" value.
 * @param red true if the red LED should be ON, false otherwise.
 * @param green true if the green LED should be ON, false otherwise.
 * @param blue true if the blue LED should be ON, false otherwise.
 * @remark If the color control is not enabled, the LED color will not change.
 * enableControl() should be called before.
 */
void LedStatusIndicator::setNormalColor(bool red, bool green, bool blue)
{
    normalRedState = red;
    normalGreenState = green;
    normalBlueState = blue;

    setColor(red, green, blue);
}

/**
 * @brief Plays a sequence of LED pulses.
 * After the sequence is played, the LED color will turn back to the last
 * "normal" value, previously set by setColor().
 * @param pulses vector containing a sequence of pulses to play sequentially.
 */
void LedStatusIndicator::playSequence(std::vector<LedPulse> pulses)
{
    remainingPulses = pulses;
}

/**
 * @brief Directly sets the color the RGB LED.
 * @param red true to enable the red LED, false otherwise.
 * @param green true to enable the green LED, false otherwise.
 * @param blue true to enable the blue LED, false otherwise.
 */
void LedStatusIndicator::setColor(bool red, bool green, bool blue)
{
    redGpio.setPinState(red);
    greenGpio.setPinState(green);
    blueGpio.setPinState(blue);
}
