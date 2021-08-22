#include "imu.h"

#include "configfile.h"

using namespace std;

/**
 * @brief Reads and calibrates the sensors readings.
 */
void Imu::update(float)
{
    if(state == PeripheralState::ACTIVE)
    {
        readAll();

        // Calibrate the gyro if requested.
        if(calibrationRequest)
        {
            gyroOffset = rawAngularSpeed / gyroCalibGains - gyroCalibOffsets;

            calibrationRequest = false;
        }

        // Correct the raw samples with the calibration values.
        acceleration = rawAcceleration / accelCalibGains;
        acceleration -= accelCalibOffsets;

        angularSpeed = rawAngularSpeed / gyroCalibGains;
        angularSpeed -= gyroCalibOffsets;

        angularSpeed -= gyroOffset;
    }
}

/**
 * @brief Gets the last measured acceleration.
 * @return The last measured acceleration vector [m/s^2].
 * @note This function does not perform the acquisition, so it necessary to call
 * acquire() or readAcceleration() before, in order to get up-to-date values.
 */
const Vec3f& Imu::getAcceleration() const
{
    return acceleration;
}

/**
 * @brief Gets the last measured angular speed.
 * @return The last measured angular speed vector [deg/s].
 * @note This function does not perform the acquisition, so it necessary to call
 * acquire() or readAngularSpeed() before, in order to get up-to-date values.
 */
const Vec3f& Imu::getAngularSpeed() const
{
    return angularSpeed;
}

/**
 * @brief Gets the last measured temperature.
 * @return The last measured temperature [deg].
 * @note This function does not perform the acquisition, so it necessary to call
 * acquire() or readTemperature() before, in order to get up-to-date values.
 */
const float& Imu::getTemperature() const
{
    return temperature;
}

/**
 * @brief Resets the offsets and gains of the accelerometer and gyrometer.
 * Resets the offsets and gains, such that the raw acceleration and angular
 * speed can be obtained. This is useful for calibration purpose.
 */
void Imu::resetCalibration()
{
    accelCalibGains = Vec3f(1.0f, 1.0f, 1.0f);
    accelCalibOffsets = Vec3f(0.0f, 0.0f, 0.0f);
    gyroCalibGains = Vec3f(1.0f, 1.0f, 1.0f);
    gyroCalibOffsets = Vec3f(0.0f, 0.0f, 0.0f);
}

/**
 * @brief Constructor.
 * @param configFileName filename of the calibration file, or "" if unused.
 */
Imu::Imu(string configFileName)
{
    //
    calibrationRequest = false;

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("ax", "m/s^2", acceleration.x,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("ay", "m/s^2", acceleration.y,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("az", "m/s^2", acceleration.z,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("gx", "deg/s", angularSpeed.x,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("gy", "deg/s", angularSpeed.y,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("gz", "deg/s", angularSpeed.z,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("temperature", "celsius", temperature,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("calibrate_gyro", "", calibrationRequest,
                                   VarAccess::WRITE, false));

    /*syncVars.push_back(makeSyncVar("ax_gain", "", accelCalibGains.x,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("ax_offset", "m/s^2", accelCalibOffsets.x,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("ay_gain", "", accelCalibGains.y,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("ay_offset", "m/s^2", accelCalibOffsets.y,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("az_gain", "", accelCalibGains.z,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("az_offset", "m/s^2", accelCalibOffsets.z,
                                   VarAccess::READWRITE, false));

    syncVars.push_back(makeSyncVar("gx_gain", "", gyroCalibGains.x,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("gx_offset", "deg/s", gyroCalibOffsets.x,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("gy_gain", "", gyroCalibGains.y,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("gy_offset", "deg/s", gyroCalibOffsets.y,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("gz_gain", "", gyroCalibGains.z,
                                   VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("gz_offset", "deg/s", gyroCalibOffsets.z,
                                   VarAccess::READWRITE, false));*/

    // Load the calibration values, if available.
    ConfigFile configFile(configFileName);

    accelCalibGains = configFile.get("accel_gains",
                                     VecN<float,3>({1.0f, 1.0f, 1.0f}));
    accelCalibOffsets = configFile.get("accel_offsets",
                                       VecN<float,3>({0.0f, 0.0f, 0.0f}));
    gyroCalibGains = configFile.get("gyro_gains",
                                    VecN<float,3>({1.0f, 1.0f, 1.0f}));
    gyroCalibOffsets = configFile.get("gyro_offsets",
                                      VecN<float,3>({0.0f, 0.0f, 0.0f}));
}

/**
 * @brief Destructor.
 */
Imu::~Imu()
{

}
