#ifndef DEF_DRIVERS_MOTORBOARD_H
#define DEF_DRIVERS_MOTORBOARD_H

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

#if __arm__
#define SIMULATED_ENCODERS 0
#else
#define SIMULATED_ENCODERS 1
#endif

#define MOTORBOARD_RX_DATA_BYTES_BUFFER_SIZE 1000

class Communication;

/**
 * @defgroup Motorboard Motorboard
 * @brief Walki dual motor controller.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Logging point when the motorboard is in "position logging" mode.
 */
struct PositionLogPoint
{
    // TODO: update.
    uint64_t timestamp; ///< Timestamp [us].
    float currentPosition, ///< Current motor position [deg].
          targetPosition; ///< Target motor position [deg].
    float command; ///< [V] or [N.m], depending on the control mode.
};

/**
 * @brief Motors enumeration.
 */
enum class MotorID
{
    A = MOTOR_A, ///< Motor connected on the channel A of the motorboard.
    B = MOTOR_B ///< Motor connected on the channel B of the motorboard.
};

/**
 * @brief Joints enumeration.
 */
enum class JointID
{
    A = JOINT_A, ///< Joint corresponding on the motor A.
    B = JOINT_B ///< Joint corresponding on the motor A.
};

/**
 * @brief Representation of a sine wave, used for Fourier decomposition.
 */
struct SineDef
{
    float a, ///< Amplitude of the cosine.
          b; ///< Amplitude of the sine.
};

/**
 * @brief Walki motorboard driver.
 */
class MotorBoard : public Peripheral
{
    // TODO: Create the class Motor (instanciated 4x), that has a reference on
    // one of the two motorboards.
public:
    MotorBoard(UartPort serialPort, float speedAccelFilteringTau,
               bool doPingTest = true);
    ~MotorBoard() override;

    void update(float dt) override;

    void resetBoard();
    void setTime();
    void setLed(bool enable);

    void armBridges();
    void disarmBridges();
    void coast(MotorJointID id);
    void setVoltage(MotorJointID id, float voltage);
    void setTorque(MotorJointID id, float torque);
    void setSpeed(MotorJointID id, float speed);
    void setPosition(MotorJointID id, float position, float duration);
    void setSineTrajectory(MotorJointID id, float period, float offset,
                           float a1, float a2, float a3,
                           float f1, float f2, float f3);
    void setFourierTrajectory(MotorJointID id, float period,
                              std::vector<SineDef> fourierCoefs);
    void setTrajectoryAmplitude(MotorJointID id, float amplitude);
    void setTrajectorySpeed(MotorJointID id, float speed);
    void setTrajectoryProgress(MotorJointID id, float progress);

    float getPosition(MotorJointID id) const;
    float getSpeed(MotorJointID id) const;
    float getAcceleration(MotorJointID id) const;
    float getCurrent(MotorJointID id) const;
    float getTargetCurrent(MotorJointID id) const;

    void setEncoderPosition(JointID id, float position);

    bool fault();
    bool overTemperatureWarning();
    void getTemperatures(float *tA, float *tB, float *tC);
    float getAbsoluteSensorPosition(JointID id) const;

    template <typename T> void setVar(uint8_t varID, T value);
    template <typename T>
    void setMotorJointVar(uint8_t motorJointId, uint8_t varID, T value);

    static uint32_t computeMessageCrc(uint8_t type, uint8_t const *data,
                                      uint32_t length);

protected:
    void sendPacket(uint8_t messageType,
                    const std::vector<uint8_t> &data = std::vector<uint8_t>());
    bool processRxBytes(const std::vector<uint8_t> &rxBuffer);
    bool ping();
    bool currentMessageIsValid(int expectedPayloadSize);

private:
#if !SIMULATED_ENCODERS
    Uart uart; ///< UART communication port to communicate with the motorboard.
#endif
    std::vector<uint8_t> txBuffer; ///< Pre-allocated buffer for sending data.
    uint8_t rxDataBytesBuffer[MOTORBOARD_RX_DATA_BYTES_BUFFER_SIZE]; ///< Pre-allocated buffer for building a single RX packet.
    bool pingtBack; ///< True if the motorboard is detected (ping request answered).
    unsigned int rxBytesCount, ///< Current number of bytes in the RX packet.
                 rxCurrentMessageType; ///< Type of the current RX packet.
    uint8_t firstHalfByte; ///< Temporary byte buffer to store the first half of the data byte.
    float timeWithoutRx; ///< Time elapsed since the last valid message received [s].
    bool ignoreCommErrors; ///< Indicates whether the serious communication errors should be silently ignored.

    float currentA, currentB; // [A].
    float currentTargetA, currentTargetB; // [A].
    float boardVoltage; // [V].
    float boardCurrent; // [A].
    float angles[4]; // [deg].
    float targetAngleA, targetAngleB; // [deg].
    float potentiometerA, potentiometerB; // [deg].
    Derivator speeds[4]; // [deg/s].
    Derivator accelerations[4]; // [deg/s^2].

    uint64_t motorboardTimestamp, previousMotorboardTimestamp; // [us].

    int8_t temperatureA, temperatureB, temperatureC; // [°C].

    bool gotFault;

    std::chrono::steady_clock::time_point resetTime; ///< Time at which the motorboard was reset.

#if SIMULATED_ENCODERS
    std::ifstream encodersFile;
#endif
};

/**
 * @brief Remotely sets the value of a variable of the motorboard firmware.
 * @param varID Identifier of the variable to set.
 * @param value New value of the selected variable. The raw bytes of the given
 * value will be copied. No cast will be performed, so value should be the same
 * type as the actual variable on the motroboard.
 */
template <typename T> void MotorBoard::setVar(uint8_t varID, T value) {
    static_assert(sizeof(T) == MOTORBOARD_COMM_VAR_SIZE,
                  "value is not the right size.");

    std::vector<uint8_t> data(sizeof(uint8_t) + MOTORBOARD_COMM_VAR_SIZE);

    data[0] = varID;
    memcpy(&data[1], &value, MOTORBOARD_COMM_VAR_SIZE);

    sendPacket(PC_MESSAGE_SET_VAR, data);
}

/**
 * @brief Remotely sets the value of a variable of the motorboard firmware.
 * @param varID Identifier of the variable to set.
 * @param value New value of the selected variable. The raw bytes of the given
 * value will be copied. No cast will be performed, so value should be the same
 * type as the actual variable on the motroboard.
 */
template <typename T>
void MotorBoard::setMotorJointVar(uint8_t motorJointId, uint8_t varID,
                                  T value) {
    static_assert(sizeof(T) == MOTORBOARD_COMM_VAR_SIZE,
                  "value is not the right size.");

    std::vector<uint8_t> data(sizeof(uint8_t) + sizeof(uint8_t) +
                              MOTORBOARD_COMM_VAR_SIZE);

    data[0] = motorJointId;
    data[1] = varID;
    memcpy(&data[2], &value, MOTORBOARD_COMM_VAR_SIZE);

    sendPacket(PC_MESSAGE_SET_MOTOR_JOINT_VAR, data);
}

/**
 * @brief Helper class to interact with a single axis of a motorboard.
 */
class MotorBoardAxis {
public:
    MotorBoardAxis(MotorBoard &motorboard, MotorJointID axis,
                   float angularOffset);

    void coast();
    void setVoltage(float voltage);
    void setTorque(float torque);
    void setSpeed(float speed);
    void setPosition(float position, float duration = 0.0f);

    float getPosition() const;
    float getSpeed() const;
    float getAcceleration() const;
    float getCurrent() const;
    float getTargetCurrent() const;

    void setAngularOffset(float offset);
    float getAngularOffset() const;

private:
    MotorBoard &motorboard; ///< Motorboard which controls the selected motor/joint.
    const MotorJointID axis; ///< Selected axis (motor or joint).
    float angularOffset; ///< Angular offset [deg], to be subtracted from the measured position, and added to the target position.
};

/**
 * @}
 */

#endif
