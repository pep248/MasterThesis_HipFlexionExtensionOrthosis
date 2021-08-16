#include "peripheral.h"

#include "utils.h"

using namespace std;
using namespace chrono;

/**
 * @brief Constructor.
 */
Peripheral::Peripheral()
{
    state = PeripheralState::DISABLED;
    syncVars.push_back(makeSyncVar("state", "", state, VarAccess::READ, false));

    updateThread = nullptr;
}

Peripheral::~Peripheral()
{
    stopAutoUpdate();
}

/**
 * @brief Gets the current state of the device.
 * @return the current state of the device.
 */
PeripheralState Peripheral::getState() const
{
    return state;
}

/**
 * @brief Gets the Syncvars that the device has.
 * @return the list of SyncVars that the device has.
 */
SyncVarList Peripheral::getVars() const
{
    return syncVars;
}

/**
 * @brief Prints whether the initialization was successful or not.
 * @param name a short description of the peripheral to be printed. The first
 * letter should be capitalized, since it will start the sentence.
 * @return a message describing the if the device is working or not.
 */
string Peripheral::printInitResult(string name)
{
    if(state == ACTIVE || state == CALIBRATING)
        return name + " up and running.";
    else
        return name + " could not be initialized.";
}

/**
 * @brief Starts the dedicated thread, so that update() is called automatically.
 * @param autoUpdatePeriod the period at which the update() period should be
 * called [s].
 * @warning stopAutoUpdate() must be called before the destructor call,
 * otherwise this may cause a segmentation fault.
 */
void Peripheral::startAutoUpdate(float autoUpdatePeriod)
{
    this->autoUpdatePeriod = autoUpdatePeriod;

    // Start the continuous update thread.
    updateThread = new thread(&Peripheral::continuouslyUpdate, this,
                              autoUpdatePeriod);

    // Set a lower priority.
    struct sched_param sp;
    sp.sched_priority = 2;
    pthread_setschedparam(updateThread->native_handle(), SCHED_RR, &sp);
}

/**
 * @brief Stops the updater thread.
 * @remark This method must be called before the destructor call, otherwise this
 * may cause a segmentation fault.
 */
void Peripheral::stopAutoUpdate()
{
    if(updateThread != nullptr)
    {
        stopAcquisition = true;
        updateThread->join();
        delete updateThread;
        updateThread = nullptr;
    }
}

/**
 * @brief Updates the peripheral state.
 * @param dt the time elapsed since the last call to this function [s].
 * @remark This function does nothing in Peripheral, so it should be
 * reimplemented by the subclasses.
 */
void Peripheral::update(float dt)
{
    (void)dt;
}

/**
 * @brief Calls the update() method at a fixed rate.
 * Calls the update() method at a fixed rate, until stopAcquisition is set to
 * true, in another thread.
 * @param period the period at which the update() period should be called [s].
 */
void Peripheral::continuouslyUpdate(float period)
{
    auto chronoPeriod = duration_cast<microseconds>(duration<float>(period));
    auto nextExecTime = high_resolution_clock::now();

    stopAcquisition = false;

    while(!stopAcquisition)
    {
        auto now = high_resolution_clock::now();

        update(period);

        nextExecTime = now + chronoPeriod;
        std::this_thread::sleep_until(nextExecTime);
    }
}
