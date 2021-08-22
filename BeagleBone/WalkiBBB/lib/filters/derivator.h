#ifndef DEF_LIB_FILTERS_DERIVATOR_H
#define DEF_LIB_FILTERS_DERIVATOR_H

#include "lowpassfilter.h"

/**
 * @brief Derivative filter with optional smoothing low-pass filter.
 * @ingroup Filters
 */
class Derivator
{
public:
    Derivator(float filterTau);
    float update(float newInput, float dt);
    float const& getRaw() const;
    float const& getFiltered() const;

private:
    float previousInput; ///< Previous input sample.
    float currentRawDerivative; ///< Last computed raw derivative, without filtering.
    LowPassFilter filteredDerivative; ///< Low-pass filter to smooth the output.
};

#endif
