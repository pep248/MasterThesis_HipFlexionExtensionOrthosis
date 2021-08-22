#include "scheduler.h"

#include <unistd.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>

#include "debugstream.h"
#include "utils.h"

using namespace std;
using namespace chrono;

// The following are constants and functions definitions from the kernel
// documentation.
// https://www.kernel.org/doc/Documentation/scheduler/sched-deadline.txt
#define gettid() syscall(__NR_gettid) ///< Macro to get the thread ID.
#define SCHED_DEADLINE 6 ///< Scheduler policy index for "earliest deadline first" scheduling.

/**
 * @brief Set of parameters to define a thread scheduling policy.
 */
struct sched_attr
{
    __u32 size; ///< Size of this structure.
    __u32 sched_policy; ///< Scheduler policy index.
    __u64 sched_flags; ///< Scheduler flags.

    // SCHED_NORMAL, SCHED_BATCH.
    __s32 sched_nice; ///< "Niceness" of the process.

    // SCHED_FIFO, SCHED_RR.
    __u32 sched_priority; ///< Thread priority.

    // SCHED_DEADLINE.
    __u64 sched_runtime; ///< Estimated task run time [ns].
    __u64 sched_deadline; ///< Task maximum run time [ns].
    __u64 sched_period; ///< Task repeat period [ns].
};

/**
 * @brief Sets the thread scheduling policy.
 * @param pid the ID of the thread to set.
 * @param attr a sched_attr structure to describe the scheduling parameters.
 * @param flags currently unused.
 * @return 0 in case of success, -1 in case of error.
 */
int sched_setattr(pid_t pid, const sched_attr *attr, unsigned int flags)
{
   return syscall(__NR_sched_setattr, pid, attr, flags);
}

/**
 * @brief Gets the thread scheduling policy.
 * @param pid the ID of the thread to get.
 * @param attr sched_attr structure to be written with the thread scheduling
 * parameters.
 * @param size size of the given attr structure.
 * @param flags currently unused.
 * @return 0 in case of success, -1 in case of error.
 */
int sched_getattr(pid_t pid, sched_attr *attr, unsigned int size,
                  unsigned int flags)
{
   return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

/**
 * @brief Creates a Scheduler object using earliest-deadline-first scheduling.
 * @param period period for the repetition of the task [s].
 * @param allocatedTime ratio of the period time that can be used to run the
 * task [0.0-1.0].
 * @remark If the scheduling could not be setup (e.g. if the program does not
 * have super user rights), a warning will be printed, and the task rate will be
 * controlled using a variable delay (sleep_for()).
 */
Scheduler::Scheduler(float period, float allocatedTime)
{
    loopTime = duration<float>(period);

    // Try to setup the scheduler.
    sched_attr attr;
    attr.size = sizeof(attr);

    attr.sched_flags = 0;
    attr.sched_nice = 0;     // Unused for SCHED_DEADLINE.
    attr.sched_priority = 0; //

    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_period = SEC_TO_NSEC(period);
    attr.sched_runtime = SEC_TO_NSEC(allocatedTime);
    attr.sched_deadline = attr.sched_period;

    if(sched_setattr(gettid(), &attr, 0))
    {
        debug << "Scheduler: could not setup the EDF scheduling. "
                 "Using sleep_for() instead." << endl;
        useScheduler = false;
        loopStartTime = high_resolution_clock::now();
    }
    else
        useScheduler = true;
}

/**
 * @brief Creates a Scheduler object using sleep to keep the rate.
 * @param loopTime desired period for the repetition of the task.
 */
Scheduler::Scheduler(float loopTime)
{
    this->loopTime = duration<float>(loopTime);
    useScheduler = false;
    loopStartTime = high_resolution_clock::now();
}

/**
 * @brief Destructor.
 */
Scheduler::~Scheduler()
{
    disable();
}

/**
 * @brief Pauses the thread in order to achieve the defined loop period.
 */
void Scheduler::yield()
{
    if(useScheduler)
        sched_yield();
    else
    {
        auto loopEndTime = high_resolution_clock::now();
        auto currentLoopTime = loopEndTime - loopStartTime;

        if(currentLoopTime > duration<float>(0.0f) &&
           currentLoopTime < loopTime)
        {
            auto sleepTime = loopTime - currentLoopTime;
            std::this_thread::sleep_for(sleepTime);
        }

        loopStartTime = high_resolution_clock::now();
    }
}

/**
 * @brief Disables the real-time scheduling of the thread.
 */
void Scheduler::disable()
{
    if(useScheduler)
    {
        sched_attr attr;
        attr.size = sizeof(attr);

        attr.sched_policy = SCHED_OTHER;
        attr.sched_flags = 0;
        attr.sched_nice = 0;
        attr.sched_priority = 0;

        attr.sched_period = 0;   // Unused for SCHED_OTHER.
        attr.sched_runtime = 0;  //
        attr.sched_deadline = 0; //

        if(sched_setattr(gettid(), &attr, 0))
            useScheduler = false;
    }
}
