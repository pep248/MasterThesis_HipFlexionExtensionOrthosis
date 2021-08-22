#ifndef DEF_LIB_TRAJECTORYGENERATOR_H
#define DEF_LIB_TRAJECTORYGENERATOR_H

/**
 * @defgroup TrajectoryGenerator Trajectory generator
 * @brief Generates interpolated points between two positions.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Generates interpolated points between two positions.
 */
class TrajectoryGenerator
{
public:
    TrajectoryGenerator();
    void setup(float startPosition, float endPosition, float duration,
               bool smoothInterpolation);
    void update(float dt);
    bool finished() const;
    float getCurrentTargetPosition() const;
    void clear();
    bool isSetup() const;

private:
    float currentTime, finalTime; ///< [s].
    float a, b, c, d; ///< [].
    float currentTargetPosition; ///< Last target position computed by update().
};

/**
 * @}
 */

#endif
