#ifndef DEF_LIB_BINARYSIGNALFILTER_H
#define DEF_LIB_BINARYSIGNALFILTER_H

#include "filters/lowpassfilter.h"

/**
 * @brief Filters a binary signal and detects rising and falling edge events.
 * Filters a binary signal to avoid high-frequency pulses, and detects the
 * rising and falling edges events. This is useful to debounce a button and
 * detecting when it is pressed, for example.
 * Create one object per signal to process. Then, call the update() method
 * every time a new sample is available for the input signal. To get the
 * filtered value of the signal, call isPressed().
 */
class BinarySignalFilter
{
public:
    BinarySignalFilter(float tau);

    bool update(bool inputSample, float dt);
    void reset(bool newState);

    const bool& isHigh() const;
    bool risingEdgeEventOccured();
    bool fallingEdgeEventOccured();

private:
    bool filteredSignalValue; ///< Current filtered signal value.
    bool riseEvent; ///< true if a rising edge event occured since the last call to risingEdgeEventOccured(), false otherwise.
    bool fallEvent; ///< true if a falling edge event occured since the last call to falllingEdgeEventOccured(), false otherwise.
    LowPassFilter signalFilter; ///< Analog signal low-pass filter.
};

#endif
