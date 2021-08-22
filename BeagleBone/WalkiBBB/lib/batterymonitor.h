#ifndef DEF_LIB_BATTERYMONITOR_H
#define DEF_LIB_BATTERYMONITOR_H

#include "../drivers/adc.h"

/**
 * @defgroup BatteryMonitor Battery monitor
 * @brief Monitors the battery state.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Monitors the battery state.
 * @ingroup Lib
 */
class BatteryMonitor : public Peripheral
{
public:
    BatteryMonitor();
    ~BatteryMonitor();
    void update(float dt);
    const float& getBoardVoltage() const; // [V].
    const float& getBoardCurrent() const; // [A].
    const float& getBatteryLevel() const; // [].

private:
    Adc voltageAdc, ///< ADC to acquire the battery voltage.
        currentAdc; ///< ADC to acquire the battery current.
    float boardVoltage; ///< Last measured battery voltage [V].
    float boardCurrent; ///< Last mesasured battery current [A].
    float batteryLevel; ///< Estimated battery level [0.0-1.0].
    float energyConsumption; ///< Current estimated energy consumption [A.h].
    float boardCurrentOffset; ///< Offset value for the current measurements [A].
    int nBatteryCells; ///< Number of battery cells detected, 6 or 12.
};

/**
 * @}
 */

#endif
