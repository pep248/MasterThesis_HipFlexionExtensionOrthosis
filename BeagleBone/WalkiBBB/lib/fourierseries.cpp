#include "fourierseries.h"

#include "utils.h"

/**
 * @brief Constructor.
 * @param a coefficients vector, multiplying the cosines. The first element is
 * multiplying "cos(0)", so it represents actually the "offset" of the function.
 * @param b coefficients vector, multiplying the sines. The first element is
 * multiplying "sin(0)", so it is ignored.
 * @param period function period [s].
 * @throws runtime_error if a and b are not the same size.
 */
FourierSeries::FourierSeries(std::vector<float> a, std::vector<float> b,
                             float period) : size(a.size())
{
    if(a.size() != b.size())
        throw std::runtime_error("FourierSeries: a and b sizes not equal.");

    this->a = a;
    this->b = b;
    pulsation = 2.0f * PI / period;
}

/**
 * @brief Evaluates the function at the given time point.
 * @param time time point to evaluate the function [s].
 * @return the value of the function, evaluated at the given time point.
 */
float FourierSeries::get(float time)
{
    float result = a[0];

    for(int i=1; i < size; i++)
    {
        result += a[i] * cosf(((float)i) * pulsation * time)
                + b[i] * sinf(((float)i) * pulsation * time);
    }

    return result;
}


