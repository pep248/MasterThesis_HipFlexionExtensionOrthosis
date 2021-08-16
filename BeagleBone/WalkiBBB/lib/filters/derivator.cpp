#include "derivator.h"

#include <limits>

using namespace std;

/**
 * @brief Constructor.
 * @param filterTau cut-off period of the low-pass filter applied to the
 * computed derivative [s].
 */
Derivator::Derivator(float filterTau) : filteredDerivative(filterTau)
{
    previousInput = 0.0f;
    currentRawDerivative = 0.0f;
}

/**
 * @brief Updates the filter output.
 * @param newInput new input sample from the signal to derivate.
 * @param dt time elapsed since the last call to this function.
 * @return The new filtered derivative.
 */
float Derivator::update(float newInput, float dt)
{
    if(dt > numeric_limits<float>::epsilon())
    {
        currentRawDerivative = (newInput - previousInput) / dt;
        previousInput = newInput;
    }

    return filteredDerivative.update(currentRawDerivative, dt);
}

/**
 * @brief Gets the last computed raw derivative, without filtering.
 * @return the last computed derivative.
 */
float const & Derivator::getRaw() const
{
    return currentRawDerivative;
}

/**
 * @brief Gets the last computed filtered derivative.
 * @return the last computed filtered derivative.
 */
float const & Derivator::getFiltered() const
{
    return filteredDerivative.get();
}
