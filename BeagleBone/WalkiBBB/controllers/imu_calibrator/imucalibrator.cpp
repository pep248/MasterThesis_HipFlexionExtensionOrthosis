#include "imucalibrator.h"

#define PITCH_JOINT JOINT_A
#define ROLL_JOINT JOINT_B

#define ROTATION_SPEED 120.0f ///< Rotation speed [deg/s].
#define CALIBRATION_DURATION 60.0f ///< Calibration duration [s].
#define ACCEL_DURATION 3.0f ///< Acceleration/deceleration duration [s].

using namespace std;
using namespace chrono;

/**
 * @brief Constructor.
 * @param peripherals peripherals initialized by main().
 */
ImuCalibrator::ImuCalibrator(PeripheralsSet peripherals) :
    Controller("IMU calibrator", peripherals),
    motorboard(UART_PORT_A, 0.050f, false)
{
    state = State::IDLE;
    speedRatio = 0.0f;
    calibrationTime = 0.0f;

    imu = peripherals.backImu; // Change to match the IMU you wish to calibrate.
    imu->resetCalibration();

    SyncVarList imuVars = imu->getVars();
    imuVars.getFromName("ax")->setLogToFile(true);
    imuVars.getFromName("ay")->setLogToFile(true);
    imuVars.getFromName("az")->setLogToFile(true);
    imuVars.getFromName("gx")->setLogToFile(true);
    imuVars.getFromName("gy")->setLogToFile(true);
    imuVars.getFromName("gz")->setLogToFile(true);
    syncVars.add("imu/", imuVars);

    syncVars.push_back(makeSyncVar<bool>("arm_motorboard", "", nullptr,
                                         [=](bool enable)
                                         {
                                             if(enable)
                                                 motorboard.armBridges();
                                             else
                                                 motorboard.disarmBridges();
                                         }, false));
    syncVars.push_back(makeSyncVar<bool>("run", "", nullptr,
                                         [=](bool enable)
                                         {
                                             if(enable)
                                                 state = State::ACCELERATING;
                                             else
                                                 state = State::DECELERATING;
                                         },
                                         true));
    syncVars.push_back(makeSyncVar<bool>("calibrating", "",
                                         [=]()
                                         {
                                             return state == State::RUNNING;
                                         },
                                         nullptr, true));
}

/**
 * @brief Destructor.
 */
ImuCalibrator::~ImuCalibrator()
{

}

/**
 * @brief Updates the controller.
 * @param dt timestep (time elapsed since the last call to this method) [s].
 */
void ImuCalibrator::update(float dt)
{
    //
    imu->update(dt);

    //
    switch(state)
    {
    case State::IDLE:
        motorboard.coast(PITCH_JOINT);
        motorboard.coast(ROLL_JOINT);
        speedRatio = 0.0f;
        break;

    case State::ACCELERATING:
        speedRatio += dt / ACCEL_DURATION;
        calibrationTime = 0.0f;

        if(speedRatio >= 1.0f)
        {
            speedRatio = 1.0f;
            state = State::RUNNING;
        }

        setSpeeds();
        break;

    case State::RUNNING:
        calibrationTime += dt;
        setSpeeds();

        if(calibrationTime > CALIBRATION_DURATION)
            state = State::DECELERATING;
        break;

    case State::DECELERATING:
        speedRatio -= dt / ACCEL_DURATION;

        if(speedRatio <= 0.0f)
        {
            speedRatio = 0.0f;
            state = State::IDLE;
        }

        setSpeeds();
        break;
    }
}

/**
 * @brief Sets the speed of both gimbal motors.
 * This function sets the speed of both motors, using speedRatio and
 * calibrationTime. The speed profile is two sine/cosine wave, such that the
 * norm of the pitch and roll rates vector is speedRatio * ROTATION_SPEED.
 */
void ImuCalibrator::setSpeeds()
{
    float pitchSpeed = speedRatio * ROTATION_SPEED *
                       sin(2.0f*PI * calibrationTime / CALIBRATION_DURATION);
    float rollSpeed = speedRatio * ROTATION_SPEED *
                      cos(2.0f*PI * calibrationTime / CALIBRATION_DURATION);

    motorboard.setSpeed(PITCH_JOINT, pitchSpeed);
    motorboard.setSpeed(ROLL_JOINT, rollSpeed);
}
