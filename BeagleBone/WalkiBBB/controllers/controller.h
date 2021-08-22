#ifndef DEF_CONTROLLERS_CONTROLLER_H
#define DEF_CONTROLLERS_CONTROLLER_H

#include "../lib/syncvar/syncvar.h"

class SpiBus;
class I2c;
class PolarHRS;
class Mpu6050;
class Pwm;
class BatteryMonitor;
class LedStatusIndicator;

/**
 * @brief Structure to group the peripherals initialized by main().
 */
struct PeripheralsSet
{
    SpiBus *spiBus; ///< SPI bus.
    I2c *i2cBus; ///< I2C bus.
    PolarHRS *heartbeatSensor; ///< Heartbeat rate sensor.
    Mpu6050 *backImu; ///< IMU located on the mainboard.
    Pwm *caseFan; ///< PWM to control the case fan speed.
    BatteryMonitor *battery; ///< Battery monitor.
    LedStatusIndicator *statusLed; ///< Status LED.
};

class Controller
{
public:
    Controller(std::string name, PeripheralsSet peripherals);
    virtual ~Controller();

    /**
     * @brief Updates the controller state.
     * @param dt time elapsed since the last call to this method [s].
     */
    virtual void update(float dt) = 0;

    std::string getName() const;
    SyncVarList getVars();

protected:
    const std::string name; ///< Short description of the controller.
    SyncVarList syncVars; ///< List of the SyncVars, in order to monitor, log the states and control the behavior.
    PeripheralsSet peripherals; ///< Peripherals initialized in main().
};

#endif
