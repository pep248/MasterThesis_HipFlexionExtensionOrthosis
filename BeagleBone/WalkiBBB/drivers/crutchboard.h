#ifndef DEF_DRIVERS_CRUTCHBOARD_H
#define DEF_DRIVERS_CRUTCHBOARD_H

#include "uart.h"
#include "motorboard.h"
#include "../lib/imu.h"
#include "../lib/stateestimator.h"
#include "../lib/configfile.h"

#include "../../CrutchBoard/source/public_definitions.h"

/**
 * @defgroup CrutchBoard CrutchBoard
 * @brief Smart crutches board.
 * @ingroup Drivers
 * @{
 */

#define CRUTCHBOARD_RX_DATA_BYTES_BUFFER_SIZE 1000 ///< Size of the RX bytes buffer to build a single RX packet [B].

/**
 * @brief IMU with arbitrary samples.
 * This Imu subclass let the user define manually the readings.
 */
class ManualImu : public Imu
{
public:
    ManualImu(std::string calibFile);
    virtual ~ManualImu() override;
    void setRawValues(Vec3f acceleration, Vec3f angularSpeed,
                      float temperature);

protected:
    void readAll() override;
};

/**
 * @brief Interface with the Walki CrutchBoard, used the smart crutches.
 */
class CrutchBoard : public Peripheral
{
public:
    CrutchBoard(std::string name, UartPort serialPort,
                std::string loadCellCalibFile, std::string imuCalibFile);
    virtual ~CrutchBoard() override;

    void update(float dt) override;

    void setLoadCellZero();

    float getLoad() const;
    const std::array<bool, 9>& getGpioChannels() const;
    void setFailSafeGpioStates(std::array<bool, 9> gpioStates);
    bool isConnected() const;

protected:
    bool processRxBytes(const std::vector<uint8_t> &rxBuffer);
    bool currentMessageIsValid(int expectedPayloadSize);

private:
    std::string name; ///< Human-readable name.
    Uart uart; ///< UART communication port to communicate with the motorboard.
    std::vector<uint8_t> txBuffer; ///< Pre-allocated buffer for sending data.
    uint8_t rxDataBytesBuffer[CRUTCHBOARD_RX_DATA_BYTES_BUFFER_SIZE]; ///< Pre-allocated buffer for building a single RX packet.
    unsigned int rxBytesCount, ///< Current number of bytes in the RX packet.
                 rxCurrentMessageType; ///< Type of the current RX packet.
    uint8_t firstHalfByte; ///< Temporary byte buffer to store the first half of the data byte.
    float timeWithoutRx; ///< Time elapsed since the last valid message received [s].
    bool previouslyConnected; ///< True if the serial datalink was already established the last time update() was called, false otherwise.

    float crutchLoad; // [N].
    float crutchLoadOffset; // [N].
    float batteryVoltage; // [V].
    uint64_t boardTimestamp; // [us].
    std::array<bool, 9> gpioChannels, failSafeGpioValues;
    ManualImu imu;
    StateEstimator orientation;

    ConfigFile configFile;
};

/**
 * @}
 */


#endif
