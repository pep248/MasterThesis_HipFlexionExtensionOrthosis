#ifndef DEF_DRIVERS_POLARH7_H
#define DEF_DRIVERS_POLARH7_H

#include <string>

#include "../lib/peripheral.h"

/**
 * @defgroup POLAR_HRS Polar heartbeat rate sensor
 * @brief Polar H7/H10 heartbeat rate sensor.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Driver for the a Polar heartbeat rate sensor.
 *
 * This driver communicates with a Polar H7/H10 heartbeat sensor, via Bluetooth.
 * A Bluetooth 4 low-energy USB adapter is required. This driver relies on
 * GATTTool to use the Bluetooth, so it must be installed.
 *
 * @remark To obtain the MAC address of a Bluetooth device, you can run the
 * command: "hcitool lescan" in a Linux terminal.
 */
class PolarHRS : public Peripheral
{
public:
    PolarHRS(std::string deviceMacAddress,
             std::string acquireBatteryCommand,
             std::string startHrAcquisitionCommand,
             std::string stopHrAcquisitionCommand,
             std::string hrDataFormat);
    ~PolarHRS();

    float getHeartBeatRate();
    float getBatteryLevel();

private:
    const std::string acquireBatteryCommand;
    const std::string startHrAcquisitionCommand;
    const std::string stopHrAcquisitionCommand;
    const std::string hrDataFormat;

    void acquireContinuously();
    void update(float dt) override;

    FILE *file; ///< File descriptor to read continuously the output of the "gatttool" command.
    float lastHeartBeatRate; ///< Last received heartbeat rate [bpm].
    std::array<uint16_t, 4> rrIntervals; ///< Last received R-R intervals [s].
    std::thread *acquisitionThread; ///< Continuous acquisition thread.
    volatile bool keepAcquiring; ///< Allows to stop the acquisition thread.
    float initBatteryLevel; ///< Battery level, read during the initialization sequence [%].
    uint8_t sensorStatus; ///< Raw sensor status byte, which indicates if a body and the heartbeat is detected.
};

/**
 * @brief Driver for the Polar H7 heartbeat rate sensor.
 * Please see the documentation of PolarHRS for full description.
 */
class PolarH7 : public PolarHRS
{
public:
    PolarH7(std::string deviceMacAddress);
};

/**
 * @brief Driver for the Polar H10 heartbeat rate sensor.
 * Please see the documentation of PolarHRS for full description.
 */
class PolarH10 : public PolarHRS
{
public:
    PolarH10(std::string deviceMacAddress);
};

/**
 * @}
 */

#endif
