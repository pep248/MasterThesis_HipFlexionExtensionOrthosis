#include "hysteresisfilter.h"

HysteresisFilter::HysteresisFilter(float threshold, float hysteresis,
                                   bool initialState)
{
    this->threshold = threshold;
    setHysteresis(hysteresis);
    state = initialState;
}

bool HysteresisFilter::update(float input)
{
    if(state && input < lowThreshold)
        state = false;
    else if(!state && input > highThreshold)
        state = true;

    return state;
}

const bool &HysteresisFilter::get() const
{
    return state;
}

void HysteresisFilter::reset(bool initialState)
{
    state = initialState;
}

void HysteresisFilter::setThreshold(float threshold)
{
    this->threshold = threshold;

    lowThreshold = threshold - hysteresis / 2.0f;
    highThreshold = threshold + hysteresis / 2.0f;
}

float HysteresisFilter::getThreshold() const
{
    return threshold;
}

void HysteresisFilter::setHysteresis(float hysteresis)
{
    this->hysteresis = hysteresis;

    lowThreshold = threshold - hysteresis / 2.0f;
    highThreshold = threshold + hysteresis / 2.0f;
}

float HysteresisFilter::getHysteresis() const
{
    return hysteresis;
}
