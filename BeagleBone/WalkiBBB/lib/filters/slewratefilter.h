#ifndef DEF_LIB_FILTERS_RISETIMEFILTER_H
#define DEF_LIB_FILTERS_RISETIMEFILTER_H

/**
 * @brief Slew-rate-limited low-pass filter.
 * @ingroup Filters
 */
class SlewRateFilter
{
public:
    SlewRateFilter(float slewRateLimit, float initialValue = 0.0f);
    float update(float input, float dt);
    float const & get() const;
    void reset(float initialValue);
    void setSlewRate(float slewRateLimit);
    float getSlewRate() const;
    float& getSlewRate();

private:
    float slewRateLimit; ///< [s^-1].
    float output; ///< Last computed output value.
};

#endif
