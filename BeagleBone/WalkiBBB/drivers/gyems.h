#ifndef DEF_DRIVERS_GYEMS_H
#define DEF_DRIVERS_GYEMS_H

#include <vector>
#include <string>
#include <list>
#include <chrono>
#include <thread>
#include <cstdint>
#include <fstream>

#include "uart.h"
#include "../../MotorBoard/src/public_definitions.h"
#include "../lib/peripheral.h"
#include "../lib/filters/derivator.h"

#include "canbus.h"

#if __arm__
#define SIMULATED_ENCODERS 0
#else
#define SIMULATED_ENCODERS 1
#endif

#define MOTORBOARD_RX_DATA_BYTES_BUFFER_SIZE 1000

// * @defgroup Gyems Motor interface
// * @brief
// * @ingroup Drivers
// * @{
// */

/**
 * @brief Walki motorboard driver.
 */
class Gyems : public Peripheral
{

public:
    Gyems(CanBus* canbus, uint8_t id, int direction = 1, float offsetAngle = 0.0f);
    ~Gyems() override;

    void update(float dt) override;
    void setTorque(float torque);
    void setPosition(float pos);
    void setSpeed(int32_t speed);
    void setCurrent(float current);
    void setAngleOffset(float offset);
    void setCurrentAngleAsZero();

    double getPosition() const;
    float getShaftAngle() const;
    float getSpeed() const;
    float getCurrent() const;
    float getTorque() const;
    int getTemperature() const;

protected:
    void sendPacket(uint8_t messageType);
    bool processRxBytes();

private:
    CanBus* can;                        ///< pointer to the CanBus object
    int motorId;                        ///< CAN ID of the motor, set by the switches on the case
    int directionSign;                  ///< -1 to invert the direction, otherwise +1
    float softwareOffsetAngle;          ///< Offset angle to be added to the output shaft angle [deg]

    int temperature;
    float current;
    int16_t inputSpeed;                 ///< input shaft speed [deg/s]
    float speed;                        ///< output shaft speed [deg/s]
    float inputShaftAngle;              ///< input shaft angle [0-360 deg]
    uint16_t inputShaftEncoderOffset;   ///< input encoder offset, in pulses [16-bit]
    double multiTurnAngle;              ///< output shaft angle [deg]

    float targetCurrent;
    float targetSpeed;
    float targetPosition;               ///< target angle [deg]
    int controlMode;
};

#endif
