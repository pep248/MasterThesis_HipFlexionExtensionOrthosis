#ifndef DEF_LIB_FILTERS_RUNNINGMEAN_H
#define DEF_LIB_FILTERS_RUNNINGMEAN_H

#include <vector>

/**
 * @brief Running mean filter.
 * @ingroup Filters
 */
class RunningMeanFilter
{
public:
    RunningMeanFilter(int nSamples, float initialValue = 0.0f);
    float update(float input);
    float const& get() const;
    void reset(float initialValue);

private:
    std::vector<float> samples;
    const int nSamples;
    float output; ///< Last computed output value.
    int currentIndex;
};

#endif
