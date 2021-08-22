#include "slewratefilter.h"

#include <algorithm>

using namespace std;

/**
 * @brief Constructor.
 * @param slewRateLimit maximum slew rate of the filtered signal [s^-1].
 * @param initialValue intial value of the output of the filter.
 */
SlewRateFilter::SlewRateFilter(float slewRateLimit, float initialValue)
{
    this->slewRateLimit = slewRateLimit;
    output = initialValue;
}

/**
 * @brief Updates the filter output.
 * @param input new input sample from the signal to filter.
 * @param dt time elapsed since the last call to this function.
 * @return The new output of the filter.
 */
float SlewRateFilter::update(float input, float dt)
{
    if(input > output)
    {
        output += slewRateLimit * dt;

        if(output > input)
            output = input;
    }
    else
    {
        output -= slewRateLimit * dt;

        if(output < input)
            output = input;
    }

    return output;
}


/**
 * @brief Gets the last computed output of the filter.
 * @return the last computed output.
 */
const float &SlewRateFilter::get() const
{
    return output;
}

/**
 * @brief Resets the filter state to the desired value.
 * @param initialValue the value to set the filter output to.
 */
void SlewRateFilter::reset(float initialValue)
{
    output = initialValue;
}

/**
 * @brief Sets the maximum slew rate.
 * @param slewRateLimit slew rate limit [s^-1].
 */
void SlewRateFilter::setSlewRate(float slewRateLimit)
{
    this->slewRateLimit = slewRateLimit;
}

/**
 * @brief Gets the slew rate limit.
 * @return the slew rate limit [s^-1].
 */
float SlewRateFilter::getSlewRate() const
{
    return slewRateLimit;
}

/**
 * @brief Gets a reference to the slew rate limit.
 * This is useful to make it SyncVar-controlled.
 * @return a reference to the slew rate limit [s^-1].
 */
float &SlewRateFilter::getSlewRate()
{
    return slewRateLimit;
}
