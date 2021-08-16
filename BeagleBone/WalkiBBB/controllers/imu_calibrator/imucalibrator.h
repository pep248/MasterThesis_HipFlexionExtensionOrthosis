#ifndef DEF_CONTROLLERS_IMU_CALIBRATOR_IMUCALIBRATOR_H
#define DEF_CONTROLLERS_IMU_CALIBRATOR_IMUCALIBRATOR_H

#include "../controller.h"
#include "../../drivers/motorboard.h"
#include "../../drivers/mpu60x0.h"

/**
 * @defgroup ImuCalibrator IMU calibrator
 * @brief Controller of the IMU calibration machine.
 * @ingroup Controllers
 * @{
 */

#define MAIN_LOOP_PERIOD 0.010f ///< Main loop period [s].

/**
 * @brief Controller of the IMU calibration machine.
 * @ingroup Controllers
 */
class ImuCalibrator : public Controller
{
public:
    ImuCalibrator(PeripheralsSet peripherals);
    ~ImuCalibrator();

    void update(float dt) override;

private:
    /**
     * @brief Enumeration of the IMU calibration states.
     */
    enum class State
    {
        IDLE, ///< The gimbal is not moving.
        ACCELERATING, ///< The gimbal is accelerating gradually to reach the starting speeds smoothly.
        RUNNING, ///< The gimbal is spinning continuously. The data acquired will be used for the offline optimization process.
        DECELERATING ///< The gimbal is decelerating gradually to stop smoothly.
    };

    void setSpeeds();

    MotorBoard motorboard; ///< The motorboard controlling the two gimbal motors.
    Mpu60X0 *imu; ///< The IMU being calibrated.
    float calibrationTime; ///< Time elapsed since the start of the RUNNING phase [s].
    float speedRatio; ///< Gimbal speed ratio, used to control the overall speed, and control the acceleration/deceleration [0.0-1.0].
    State state; ///< Current state.
};

typedef ImuCalibrator SelectedController; ///< Sets the application controller as ImuCalibrator.

/**
 * @}
 */

#endif
