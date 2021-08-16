#ifndef DEF_LIB_IMU_H
#define DEF_LIB_IMU_H

#include "peripheral.h"
#include "vec3.h"


/**
 * @defgroup IMU IMU
 * @brief Inertial measurement unit.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Abstract IMU class.
 * This class represents an IMU providing 3-axes acceleration, 3-axes angular
 * speed and temperature. It features the calibration of the 6 axes.
 */
class Imu : public Peripheral
{
public:
    void update(float dt) override;

    const Vec3f& getAcceleration() const;
    const Vec3f& getAngularSpeed() const;
    const float& getTemperature() const;

    void resetCalibration();

protected:
    Imu(std::string configFileName);
    virtual ~Imu();
    virtual void readAll() = 0;

    Vec3f rawAcceleration, ///< Last measured acceleration vector, raw [m/s^2].
          rawAngularSpeed; ///< Last measured angular speed vector, raw [deg/s].
    float temperature; ///< Last measured temperature [°C].

private:
    Vec3f acceleration, ///< Last measured acceleration vector, calibrated [m/s^2].
          angularSpeed, ///< Last measured angular speed vector, calibrated [deg/s].
          gyroOffset; ///< Gyrometer offsets, obtained at runtime when calibrationRequest = true [deg/s].

    Vec3f accelCalibGains; ///< Compensation gains of the accelerometer, determined by calibration [].
    Vec3f accelCalibOffsets; ///< Compensation offsets of the accelerometer, determined by calibration [m/s^2].
    Vec3f gyroCalibGains; ///< Compensation gains of the gyrometer, determined by calibration [].
    Vec3f gyroCalibOffsets; ///< Compensation offsets of the gyrometer, determined by calibration [m/s^2].

    bool calibrationRequest; ///< true if the calibration has been requested, false otherwise.
};

/**
 * @}
 */

#endif
