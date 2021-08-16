#ifndef DEF_LIB_STATEESTIMATOR_H
#define DEF_LIB_STATEESTIMATOR_H

#include "../lib/imu.h"
#include "../drivers/hmc5883l.h"
#include "vec3.h"
#include "syncvar/syncvar.h"

/**
 * @defgroup StateEstimator State estimator
 * @brief State estimator to compute the orientation from an IMU readings.
 * @ingroup Lib
 * @{
 */

/**
 * @brief IMU orientation enumerations.
 * @ingroup Lib
 */
enum class ImuOrientation
{
    BACKPACK_VERTICAL, ///< IMU located on a vertical mainboard, facing the front.
    BACKPACK_VERTICAL_REVERSED, ///< IMU located on a vertical mainboard, facing the back.
    BACKPACK_VERTICAL_LEFT, ///< IMU located on a vertical mainboard, pointing to the left, in the backpack.
    BACKPACK_HORIZONTAL, ///< IMU located on a horizontal mainboard, in the backpack.
    LEFT_FOOT, ///< IMU located under the left foot, horizontally, upside down.
    RIGHT_FOOT, ///< IMU located under the right foot, horizontally, upside down.
    ANKLE, ///< IMU located on the ankle, vertically, facing left.
};

/**
 * @brief State estimator to compute the orientation from an IMU readings.
 * State estimator, implemented with a simple low-pass filter.
 */
class StateEstimator
{
public:
    StateEstimator(ImuOrientation imuAxesOrientation,
                   Imu* imu, Hmc5883l* magneto=nullptr);
    void update(float dt);
    const Vec3f& getOrientation() const;
    SyncVarList getSyncVars() const;

private:
    Imu* imu; ///< Associated accelerometer + gyroscope.
    Hmc5883l* magneto;///< Optionnal magnetometer.
    ImuOrientation imuAxesOrientation; ///< IMU orientation.
    Vec3f orientation; ///< Orientation, in the order: roll, pitch, yaw [deg].
    SyncVarList syncVars; ///< SyncVars.
    float accelWeight, ///< Weight of the accelerometers-estimated orientation for fusion [0.0-1.0].
          gyroWeight, ///< Weight of the gyroscopes-estimated orientation for fusion [0.0-1.0].
          magnetoWeight;  ///< Weight of the magnetometers-estimated orientation for fusion [0.0-1.0].
};

/**
 * @}
 */

#endif
