#include "lowpassfilter.h"

/**
 * @brief Constructor.
 * @param tau cut-off period [s]. If zero or negative, the filter will have no
 * effect.
 * @param initialValue intial value of the output of the filter.
 */
LowPassFilter::LowPassFilter(float tau, float initialValue)
{
    this->tau = tau;
    output = initialValue;
}

/**
 * @brief Updates the filter output.
 * @param input new input sample from the signal to filter.
 * @param dt time elapsed since the last call to this function.
 * @return The new output of the filter.
 */
float LowPassFilter::update(float input, float dt)
{
    if(tau > 0.0f && dt < tau)
        output += (input - output) / tau * dt;
    else
        output = input;

    return output;
}

/**
 * @brief Gets the last computed output of the filter.
 * @return the last computed output.
 */
float const &LowPassFilter::get() const
{
    return output;
}

/**
 * @brief Resets the filter state to the desired value.
 * @param initialValue the value to set the filter output to.
 */
void LowPassFilter::reset(float initialValue)
{
    output = initialValue;
}

/**
 * @brief Sets the cut-off period.
 * @param tau cut-off period [s].
 */
void LowPassFilter::setTau(float tau)
{
    this->tau = tau;
}

/**
 * @brief Gets the cut-off period.
 * @return the cut-off period [s].
 */
float LowPassFilter::getTau() const
{
    return tau;
}

/**
 * @brief Gets a reference to the cut-off period.
 * This is useful to make it SyncVar-controlled.
 * @return a reference to the cut-off period.
 */
float &LowPassFilter::getTau()
{
    return tau;
}
