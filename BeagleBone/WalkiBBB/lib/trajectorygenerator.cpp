#include "trajectorygenerator.h"

#include "utils.h"

using namespace std;
using namespace Utils;

/**
 * @brief Constructor.
 */
TrajectoryGenerator::TrajectoryGenerator()
{
    clear();
}

/**
 * @brief Sets the movement parameters.
 * @param startPosition start position.
 * @param endPosition end position.
 * @param duration time to reach the end position [s].
 * @param smoothInterpolation true to generate a smooth trajectory such that the
 * the derivative is zero at the beginning and the end, false to perform the
 * linear interpolation.
 */
void TrajectoryGenerator::setup(float startPosition, float endPosition,
                                float duration, bool smoothInterpolation)
{
    currentTime = 0.0f;
    finalTime = duration;

    // Compute the polynomial coefficients.
    if(duration > 0.0f)
    {
        if(smoothInterpolation)
        {
            a = 2.0f * (startPosition - endPosition) / (duration*duration*duration);
            b = -3.0f * (startPosition - endPosition) / (duration*duration);
            c = 0.0f;
            d = startPosition;
        }
        else
        {
            a = 0.0f;
            b = 0.0f;
            c = (endPosition - startPosition) / duration;
            d = startPosition;
        }

        currentTargetPosition = startPosition;
    }
    else
        currentTargetPosition = endPosition;
}

/**
 * @brief Computes the new output.
 * @param dt time elapsed since the last call to this method [s].
 */
void TrajectoryGenerator::update(float dt)
{
    currentTime += dt;

    clamp(currentTime, 0.0f, finalTime);

    if(finalTime > 0.0f && currentTime <= finalTime)
    {
        currentTargetPosition = a * (currentTime*currentTime*currentTime)
                                + b * (currentTime*currentTime)
                                + c * currentTime
                                + d;
    }
}

/**
 * @brief Indicates if the trajectory has ended.
 * @return true if the end position was reached, false otherwise.
 */
bool TrajectoryGenerator::finished() const
{
    return currentTime >= finalTime;
}

/**
 * @brief Gets the current target position.
 * @return the last target position, computed by update().
 */
float TrajectoryGenerator::getCurrentTargetPosition() const
{
    return currentTargetPosition;
}

/**
 * @brief Resets the configuration.
 * Resets the configuration so that it becomes invalid. isSetup() will return
 * false.
 */
void TrajectoryGenerator::clear()
{
    currentTime = 0.0f;
    finalTime = 0.0f;
    currentTargetPosition = 0.0f;
}

/**
 * @brief Indicates if the trajectory has been set.
 * @return true if the trajectory was set, false otherwise.
 */
bool TrajectoryGenerator::isSetup() const
{
    return finalTime == 0.0f;
}
