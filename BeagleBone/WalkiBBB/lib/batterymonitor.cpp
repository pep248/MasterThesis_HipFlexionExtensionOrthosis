#include "batterymonitor.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "utils.h"
#include "debugstream.h"

#define LIPO_VOLTAGE_MIN 3.7f // [V].
#define LIPO_VOLTAGE_MAX 4.2f // [V].
#define LIPO_VOLTAGE_DETECT_MARGIN 0.5f // [V].
#define LIPO_6S_CELLS 6.0f
#define LIPO_12S_CELLS 12.0f

#define CURRENT_CALIB_N_SAMPLES 100
#define REF_IDLE_CURRENT_24V 0.250f // [A]. TODO: update when all peripherals will be connected.

using namespace std;

/**
 * @brief Constructor.
 */
BatteryMonitor::BatteryMonitor() :
    voltageAdc(ADC_BOARD_VOLTAGE), currentAdc(ADC_BOARD_CURRENT)
{
    energyConsumption = 0.0f;

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("input_voltage", "V", boardVoltage,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("input_current", "A", boardCurrent,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("battery_level", "", batteryLevel,
                                   VarAccess::READ, false));

    // Check the status of the ADC channels.
    if(voltageAdc.getState() != PeripheralState::ACTIVE ||
       currentAdc.getState() != PeripheralState::ACTIVE)
    {
        state = PeripheralState::FAULT;
        return;
    }

    state = PeripheralState::ACTIVE;

    // Detect the input voltage (24 or 48V).
    voltageAdc.update(0.0f);
    float voltage = voltageAdc.get() * ADC_BOARD_VOLTAGE_FACTOR;

    if(voltage > LIPO_6S_CELLS * (LIPO_VOLTAGE_MIN
                                  - LIPO_VOLTAGE_DETECT_MARGIN) &&
       voltage < LIPO_6S_CELLS * (LIPO_VOLTAGE_MAX
                                  + LIPO_VOLTAGE_DETECT_MARGIN))
    {
        nBatteryCells = 6;
    }
    else if(voltage > LIPO_12S_CELLS * (LIPO_VOLTAGE_MIN
                                        - LIPO_VOLTAGE_DETECT_MARGIN) &&
            voltage < LIPO_12S_CELLS * (LIPO_VOLTAGE_MAX
                                        + LIPO_VOLTAGE_DETECT_MARGIN))
    {
        nBatteryCells = 12;
    }
    else
    {
        debug << "The input voltage is out of range (" << voltage << " V). "
                 "Remaining capacity estimation will not be available." << endl;
        nBatteryCells = 0;
    }

    // Calibrate the current sensor.
    float sum = 0.0f;

    for(int i=0; i<CURRENT_CALIB_N_SAMPLES; i++)
    {
        // Acquire the board voltage channel. This is normally not necessary,
        // but the changing the way the channels are acquired actually changes
        // a bit the measurements (cross-talk/multiplexer problem?).
        voltageAdc.update(0.0f);

        // Acquire the board current channel, and compute the average.
        currentAdc.update(0.0f);
        sum += currentAdc.get() * ADC_BOARD_CURRENT_FACTOR;
        this_thread::sleep_for(chrono::milliseconds(5));
    }

    float refCurrent;

    if(nBatteryCells == 6)
        refCurrent = REF_IDLE_CURRENT_24V;
    else if(nBatteryCells == 12)
        refCurrent = REF_IDLE_CURRENT_24V / 2.0f;
    else
        refCurrent = 0.0f;

    boardCurrentOffset = sum / (float)CURRENT_CALIB_N_SAMPLES
                         - refCurrent;
}

/**
 * @brief Destructor.
 */
BatteryMonitor::~BatteryMonitor()
{
    stopAutoUpdate();
}

/**
 * @brief Acquires the voltage and current, and update the battery estimation.
 * @param dt the time elapsed since the last call to this function [s].
 */
void BatteryMonitor::update(float dt)
{
    // Read the ADC to get the board voltage and current.
    voltageAdc.update(dt);
    boardVoltage = voltageAdc.get() * ADC_BOARD_VOLTAGE_FACTOR;

    currentAdc.update(dt);
    float rawCurrentAdcVoltage = currentAdc.get();

    boardCurrent = rawCurrentAdcVoltage * ADC_BOARD_CURRENT_FACTOR
                   - boardCurrentOffset;

    // Integrate the current to compute the energy consumption.
    energyConsumption += boardCurrent * dt / 3600.0f;

    // Compute the battery level from the voltage.
    // This formula is not very accurate and should be improved.
    if(nBatteryCells != 0)
    {
        batteryLevel = (boardVoltage - LIPO_VOLTAGE_MIN*nBatteryCells) /
                       ((LIPO_VOLTAGE_MAX - LIPO_VOLTAGE_MIN) * nBatteryCells);
    }
    else
        batteryLevel = 1.0f;
}

/**
 * @brief Gets the board (battery) voltage.
 * @return the board voltage [V].
 */
const float &BatteryMonitor::getBoardVoltage() const
{
    return boardVoltage;
}

/**
 * @brief Gets the board (battery) current.
 * @return the board current [A].
 */
const float &BatteryMonitor::getBoardCurrent() const
{
    return boardCurrent;
}

/**
 * @brief Gets the battery level.
 * @return the battery level [0.0-1.0]. 1.0 is full, 0.0 is empty.
 */
const float &BatteryMonitor::getBatteryLevel() const
{
    return batteryLevel;
}
