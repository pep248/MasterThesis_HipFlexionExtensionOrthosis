#include "basicfilter.h"

/**
 * @brief Constructor.
 * @param strength filtering strength (0.0-1.0). 0.0 corresponds to no filtering
 * at all, while values close to 1 filter strongly.
 * @param initialValue intial value of the output of the filter.
 */
BasicFilter::BasicFilter(float strength, float initialValue)
{
    this->strength = strength;
    output = initialValue;
}

/**
 * @brief Updates the filter output.
 * @param input new input sample from the signal to filter.
 * @return The new output of the filter.
 */
float BasicFilter::update(float input)
{
    output = strength * output + (1.0f-strength) * input;

    return output;
}

/**
 * @brief Gets the last computed output of the filter.
 * @return the last computed output.
 */
float const& BasicFilter::get() const
{
     return output;
}

/**
 * @brief Resets the filter state to the desired value.
 * @param initialValue the value to set the filter output to.
 */
void BasicFilter::reset(float initialValue)
{
    output = initialValue;
}
