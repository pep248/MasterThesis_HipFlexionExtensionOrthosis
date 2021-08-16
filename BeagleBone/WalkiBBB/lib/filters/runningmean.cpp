#include "runningmean.h"

#include <algorithm>

using namespace std;

/**
 * @brief Constructor
 * @param nSamples size of the running mean, in samples. The nSamples last given
 * samples will be stored, and averaged to get the output.
 * @param initialValue initial output value of the filter.
 */
RunningMeanFilter::RunningMeanFilter(int nSamples, float initialValue) :
    samples(nSamples, initialValue), nSamples(nSamples)
{
    currentIndex = 0;
}

/**
 * @brief Updates the filter output.
 * @param input new input sample from the signal to filter.
 * @return The new output of the filter.
 */
float RunningMeanFilter::update(float input)
{
    // Add the new sample at the right location in the array.
    samples[currentIndex] = input;
    currentIndex++;

    if(currentIndex >= nSamples)
        currentIndex = 0;

    // Compute the mean.
    float sum = 0.0f;

    for(int i=0; i<nSamples; i++)
        sum += samples[i];

    output = sum / (float)nSamples;

    return output;
}

/**
 * @brief Gets the last computed output of the filter.
 * @return the last computed output.
 */
float const& RunningMeanFilter::get() const
{
    return output;
}

/**
 * @brief Sets all the filter samples to the desired value.
 * @param initialValue the value to set the filter output to.
 */
void RunningMeanFilter::reset(float initialValue)
{
    fill(samples.begin(), samples.end(), initialValue);
    output = initialValue;
}

