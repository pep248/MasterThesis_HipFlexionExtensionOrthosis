#include "binarysignalfilter.h"

/**
 * @brief Constructor.
 * @param tau cut-off period of the analog signal filter [s].
 */
BinarySignalFilter::BinarySignalFilter(float tau) : signalFilter(tau, 0.0f)
{
    filteredSignalValue = false;
    riseEvent = false;
    fallEvent = false;
}

/**
 * @brief Updates the filtered signal with a new raw sample.
 * @param inputSample input raw sample.
 * @param dt time elapsed since the last call to this method.
 * @return the updated filtered signal value.
 */
bool BinarySignalFilter::update(bool inputSample, float dt)
{
    signalFilter.update(inputSample ? 1.0f : 0.0f, dt);

    if(filteredSignalValue && signalFilter.get() < 0.33f)
    {
        filteredSignalValue = false;
        fallEvent = true;
    }
    else if(!filteredSignalValue && signalFilter.get() > 0.66f)
    {
        filteredSignalValue = true;
        riseEvent = true;
    }

    return filteredSignalValue;
}

/**
 * @brief Forces the filter state to the given one.
 * @param newState new state of the filter.
 */
void BinarySignalFilter::reset(bool newState)
{
    signalFilter.reset(newState ? 1.0f : 0.0f);

    filteredSignalValue = newState;
    riseEvent = false;
    fallEvent = false;
}

/**
 * @brief Gets the filtered signal value.
 * @return a const reference to the filtered signal value.
 */
const bool &BinarySignalFilter::isHigh() const
{
    return filteredSignalValue;
}

/**
 * @brief Gets if a rising edge event occured.
 * @return true if a rising edge event occured since the last call to this
 * method, false otherwise.
 */
bool BinarySignalFilter::risingEdgeEventOccured()
{
    if(riseEvent)
    {
        riseEvent = false;
        return true;
    }
    else
        return false;
}

/**
 * @brief Gets if a falling edge event occured.
 * @return true if a falling edge event occured since the last call to this
 * method, false otherwise.
 */
bool BinarySignalFilter::fallingEdgeEventOccured()
{
    if(fallEvent)
    {
        fallEvent = false;
        return true;
    }
    else
        return false;
}
