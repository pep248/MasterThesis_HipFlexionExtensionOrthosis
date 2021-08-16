#ifndef DEF_LIB_SCHEDULER_H
#define DEF_LIB_SCHEDULER_H

#include <chrono>

/**
 * @defgroup Scheduler Scheduler
 * @brief Setups the operating system scheduler to get a constant loop period.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Setups the operating system scheduler to get a constant loop period.
 */
class Scheduler
{
public:
    Scheduler(float period, float allocatedTime);
    Scheduler(float loopTime);
    ~Scheduler();
    void yield();
    void disable();

private:
    bool useScheduler;
    std::chrono::duration<float> loopTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> loopStartTime;
};

/**
 * @}
 */

#endif
