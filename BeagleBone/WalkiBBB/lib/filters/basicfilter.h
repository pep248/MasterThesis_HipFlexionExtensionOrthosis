#ifndef DEF_LIB_FILTERS_BASICFILTER_H
#define DEF_LIB_FILTERS_BASICFILTER_H

/**
 * @brief Simplest smoothing filter.
 * @ingroup Filters
 */
class BasicFilter
{
public:
    BasicFilter(float strength, float initialValue = 0.0f);
    float update(float input);
    float const& get() const;
    void reset(float initialValue);

private:
    float strength; ///< Filtering strength (0.0-1.0).
    float output; ///< Last computed output value.
};

#endif
