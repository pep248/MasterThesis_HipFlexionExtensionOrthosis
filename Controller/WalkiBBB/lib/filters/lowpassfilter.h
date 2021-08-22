#ifndef DEF_LIB_FILTERS_LOWPASSFILTER_H
#define DEF_LIB_FILTERS_LOWPASSFILTER_H

/**
 * @brief First-order low-pass filter.
 * @ingroup Filters
 */
class LowPassFilter
{
public:
    LowPassFilter(float tau, float initialValue = 0.0f);
    float update(float input, float dt);
    float const & get() const;
    void reset(float initialValue);
    void setTau(float tau);
    float getTau() const;
    float& getTau();

private:
    float tau; ///< Cut-off period [s].
    float output; ///< Last computed output value.
};

#endif
