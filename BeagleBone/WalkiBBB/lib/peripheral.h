#ifndef DEF_LIB_PERIPHERAL_H
#define DEF_LIB_PERIPHERAL_H

#include <vector>
#include <thread>
#include <chrono>

#include "syncvar/syncvar.h"
#include "../public_definitions.h"

/**
 * @defgroup Peripheral Peripheral
 * @brief Represents a sensing or actuating device, with status and SyncVars.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Represents a sensing or actuating device, with status and SyncVars.
 * @ingroup Lib
 */
class Peripheral
{
public:
    Peripheral();
    virtual ~Peripheral();
    PeripheralState getState() const;
    SyncVarList getVars() const;
    std::string printInitResult(std::string name);

    void startAutoUpdate(float autoUpdatePeriod);
    void stopAutoUpdate();

    virtual void update(float dt);

protected:
    PeripheralState state; ///< The status of the device.
    SyncVarList syncVars; ///< The SyncVars of to the device.

private:
    void continuouslyUpdate(float period);

    volatile bool stopAcquisition; ///< Allows terminating the acquisition thread.
    std::thread *updateThread; ///< Independant acquisition thread.
    float autoUpdatePeriod; ///< Time period between each automatic call to update() [s].
};

/**
 * @}
 */

#endif
