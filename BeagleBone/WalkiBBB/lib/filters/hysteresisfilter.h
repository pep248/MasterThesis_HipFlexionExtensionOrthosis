#ifndef DEF_LIB_FILTERS_HYSTERESISFILTER_H
#define DEF_LIB_FILTERS_HYSTERESISFILTER_H

/**
 * @brief Hysteresis filter.
 * @ingroup Filters
 */
class HysteresisFilter
{
public:
    HysteresisFilter(float lowThreshold, float highThreshold,
                     bool initialState = false);
    bool update(float input);
    const bool &get() const;
    void reset(bool initialState);
    void setThreshold(float threshold);
    float getThreshold() const;
    void setHysteresis(float hysteresis);
    float getHysteresis() const;

private:
    float hysteresis;
    float threshold;
    float lowThreshold;
    float highThreshold;
    bool state; ///< Current state of the filter (false=low, true=high).
};

#endif
