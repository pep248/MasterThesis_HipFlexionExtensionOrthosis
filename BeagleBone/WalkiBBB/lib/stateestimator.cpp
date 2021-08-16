#include "stateestimator.h"

#include "utils.h"

#define GRAVITY_INTENSITY 9.81f // [m/s^2].
#define ACCEL_MAGN_TOLERANCE 2.0f // [m/s^2].

#define GYRO_WEIGHT_WITHOUT_MAGNETO 0.9995f // []
#define GYRO_WEIGHT_WITH_MAGNETO 0.95f // []
#define MAGNETO_WEIGHT 0.02f // []

const Vec3f GRAVITY_DIRECTION(0.0f, 0.0f, -1.0f);

/**
 * @brief Constructor.
 * @param imuAxesOrientation IMU axes orientation.
 * @param imu IMU to read acceleration and angular speed from.
 * @param magneto magnetometer to read the magnetic field from.
 */
StateEstimator::StateEstimator(ImuOrientation imuAxesOrientation,
                               Imu *imu, Hmc5883l* magneto)
{
    this->imuAxesOrientation = imuAxesOrientation;
    this->imu = imu;
    this->magneto = magneto;

    // Compute the weighting coefficients.
    if(magneto == nullptr)
    {
        gyroWeight = GYRO_WEIGHT_WITHOUT_MAGNETO;
        magnetoWeight = 0.0f;
    }
    else
    {
        gyroWeight = GYRO_WEIGHT_WITH_MAGNETO;
        magnetoWeight = MAGNETO_WEIGHT;
    }

    accelWeight = 1.0f - gyroWeight - magnetoWeight;

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("x", "deg", orientation.x, VarAccess::READ,
                                   true));
    syncVars.push_back(makeSyncVar("y", "deg", orientation.y, VarAccess::READ,
                                   true));
    syncVars.push_back(makeSyncVar("z", "deg", orientation.z, VarAccess::READ,
                                   (magneto != nullptr)));
}

/**
 * @brief Acquires the sensors and compute the orientation.
 * @param dt time elapsed since the last call to this function [s].
 */
void StateEstimator::update(float dt)
{
    // Acquire the sensors.
    Vec3f sensorAcceleration = imu->getAcceleration();
    Vec3f sensorAngularSpeed = imu->getAngularSpeed();

    Vec3f sensorMagneticDirection;
    if(magneto != nullptr)
    {
        magneto->update(dt);
        sensorMagneticDirection = magneto->get();
        sensorMagneticDirection.normalize();
    }

    // Change coordinates system.
    Vec3f acceleration; // [m/s^2].
    Vec3f angularSpeed; // [deg/s].
    Vec3f magneticDirection; // [].

    if(imuAxesOrientation == ImuOrientation::BACKPACK_VERTICAL)
    {
        acceleration.x = sensorAcceleration.z;
        acceleration.y = sensorAcceleration.x;
        acceleration.z = sensorAcceleration.y;

        angularSpeed.x = sensorAngularSpeed.z;
        angularSpeed.y = sensorAngularSpeed.x;
        angularSpeed.z = sensorAngularSpeed.y;

        if(magneto != nullptr)
        {
            magneticDirection.x = sensorMagneticDirection.x;
            magneticDirection.y = sensorMagneticDirection.y;
            magneticDirection.z = sensorMagneticDirection.z;
        }
    }
    else if(imuAxesOrientation == ImuOrientation::BACKPACK_VERTICAL_REVERSED)
    {
        acceleration.x = -sensorAcceleration.z;
        acceleration.y = -sensorAcceleration.x;
        acceleration.z = sensorAcceleration.y;

        angularSpeed.x = -sensorAngularSpeed.z;
        angularSpeed.y = -sensorAngularSpeed.x;
        angularSpeed.z = sensorAngularSpeed.y;

        if(magneto != nullptr)
        {
            magneticDirection.x = sensorMagneticDirection.x;
            magneticDirection.y = sensorMagneticDirection.y;
            magneticDirection.z = sensorMagneticDirection.z;
        }
    }
    else if(imuAxesOrientation == ImuOrientation::BACKPACK_VERTICAL_LEFT)
    {
        acceleration.x = -sensorAcceleration.z;
        acceleration.y = sensorAcceleration.y;
        acceleration.z = sensorAcceleration.x;

        angularSpeed.x = -sensorAngularSpeed.z;
        angularSpeed.y = sensorAngularSpeed.y;
        angularSpeed.z = sensorAngularSpeed.x;

        if(magneto != nullptr)
        {
            magneticDirection.x = sensorMagneticDirection.x;
            magneticDirection.y = sensorMagneticDirection.y;
            magneticDirection.z = sensorMagneticDirection.z;
        }
    }
    else if(imuAxesOrientation == ImuOrientation::BACKPACK_HORIZONTAL)
    {
        acceleration.x = -sensorAcceleration.x;
        acceleration.y = -sensorAcceleration.y;
        acceleration.z = sensorAcceleration.z;

        angularSpeed.x = -sensorAngularSpeed.x;
        angularSpeed.y = -sensorAngularSpeed.y;
        angularSpeed.z = sensorAngularSpeed.z;

        if(magneto != nullptr)
        {
            magneticDirection.x = sensorMagneticDirection.x;
            magneticDirection.y = sensorMagneticDirection.y;
            magneticDirection.z = sensorMagneticDirection.z;
        }
    }
    else if(imuAxesOrientation == ImuOrientation::LEFT_FOOT)
    {
        acceleration.x = sensorAcceleration.y;
        acceleration.y = sensorAcceleration.x;
        acceleration.z = -sensorAcceleration.z;

        angularSpeed.x = sensorAngularSpeed.y;
        angularSpeed.y = sensorAngularSpeed.x;
        angularSpeed.z = -sensorAngularSpeed.z;
    }
    else if(imuAxesOrientation == ImuOrientation::RIGHT_FOOT)
    {
        acceleration.x = -sensorAcceleration.y;
        acceleration.y = -sensorAcceleration.x;
        acceleration.z = -sensorAcceleration.z;

        angularSpeed.x = -sensorAngularSpeed.y;
        angularSpeed.y = -sensorAngularSpeed.x;
        angularSpeed.z = -sensorAngularSpeed.z;
    }
    else if(imuAxesOrientation == ImuOrientation::ANKLE)
    {
        acceleration.x = sensorAcceleration.y;
        acceleration.y = sensorAcceleration.z;
        acceleration.z = sensorAcceleration.x;

        angularSpeed.x = sensorAngularSpeed.y;
        angularSpeed.y = sensorAngularSpeed.z;
        angularSpeed.z = sensorAngularSpeed.x;
    }

    // Complementary filter: gyroscope contribution.
    angularSpeed.rotateAroundZ(orientation.z);
    angularSpeed.rotateAroundY(orientation.y);
    angularSpeed.rotateAroundX(orientation.x);

    Vec3f gyroOrientation = orientation + angularSpeed * dt;

    // Complementary filter: accelerometer contribution.
    float accelMagnitude = acceleration.normalize();
    Vec3f accelOrientation;

    if(fabsf(GRAVITY_INTENSITY-accelMagnitude) < ACCEL_MAGN_TOLERANCE)
    {
        // TODO: handle the case where acceleration.z is close to zero, that
        // happens when pitch or roll is close to 90°.
        accelOrientation.x = RAD_TO_DEG(atan2f(acceleration.y, acceleration.z));
        accelOrientation.y = RAD_TO_DEG(atan2f(-acceleration.x, acceleration.z));
        accelOrientation.z = orientation.z;
    }
    else
        accelOrientation = orientation;

    // Complementary filter: magnetometer contribution.
    Vec3f magnetoOrientation;

    if(magneto != nullptr)
    {
        // TODO: implement state estimation using the magnetometer.
    }

    // Fusion.
    orientation = gyroOrientation * gyroWeight +
                  accelOrientation * accelWeight +
                  magnetoOrientation * magnetoWeight;

    if(magneto == nullptr)
        orientation.z = 0.0f;
}

/**
 * @brief Gets the last computed orientation.
 * @return the orientation angles [deg] in a Vec3, in this order: roll, pitch,
 * yaw.
 * @note This function does not compute the orientation, so update() should be
 * called before.
 */
const Vec3f& StateEstimator::getOrientation() const
{
    return orientation;
}

/**
 * @brief Gets the SyncVars.
 * @return the list of SyncVars of the estimator.
 */
SyncVarList StateEstimator::getSyncVars() const
{
    return syncVars;
}
