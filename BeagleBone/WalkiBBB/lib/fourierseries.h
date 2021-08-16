#ifndef DEF_LIB_FOURIERSERIES_H
#define DEF_LIB_FOURIERSERIES_H

#include <vector>

/**
 * @brief Evaluates a periodic function defined by a Fourier series.
 */
class FourierSeries
{
public:
    FourierSeries(std::vector<float> a, std::vector<float> b, float period);
    float get(float time);

private:
    std::vector<float> a; ///< First coefficients vector (multiplying the cosines). The first element is multiplying "cos(0)", so it represents the "offset".
    std::vector<float> b; ///< Second coefficients vector (multiplying the sines). The first element is unused, since it is multiplying "sin(0)".
    float pulsation; ///< Pulsation [rad/s].
    const int size; ///< Coefficients vectors size (for performance).
};

#endif
