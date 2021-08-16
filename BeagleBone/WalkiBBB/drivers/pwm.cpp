#include "pwm.h"
#include "../config/config.h"
#include "../lib/utils.h"
#include "../lib/debugstream.h"

#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;
using namespace chrono;
using namespace Utils;

#define PWM_PERIOD 30518.0f // [ns] => 32768 Hz.

#if DEBIAN_VERSION == 7
#define PWM_OVERLAY "am33xx_pwm"

const string PWM_PINS[] = { "P9_14", "P9_16", "P8_19", "P8_13" };

const string PWM_BASEPATHS[] =
{
    "/sys/devices/ocp.*/pwm_test_P9_14.*/",
    "/sys/devices/ocp.*/pwm_test_P9_16.*/",
    "/sys/devices/ocp.*/pwm_test_P8_19.*/",
    "/sys/devices/ocp.*/pwm_test_P8_13.*/"
};

#define PWM_DUTY_FILENAME "duty"
#define PWM_POLARITY_VALUE "0" // 0 or 1.
#elif DEBIAN_VERSION == 8
#define PWM_OVERLAY "BB-EHRPWM12"

const string PWM_CHIP_PATHS[] =
{
    "/sys/class/pwm/pwmchip0/",
    "/sys/class/pwm/pwmchip0/",
    "/sys/class/pwm/pwmchip2/",
    "/sys/class/pwm/pwmchip2/"
};

const string PWM_CHANNELS[] = { "0", "1", "0", "1" };

#define PWM_DUTY_FILENAME "duty_cycle"
#define PWM_POLARITY_VALUE "normal" // "normal" or "inversed".
#endif

#define PWM_PERIOD_FILENAME "period"

/**
 * @brief Constructor.
 * @param device PWM channel to use.
 */
Pwm::Pwm(Pwm_Device device)
{
    this->pwmDevice = device;

#ifdef __arm__
    // Load the main PWM module.
    try
    {
        // Set the pin muxing, and load the PWM pin module.
        #if DEBIAN_VERSION == 7
        loadOverlay(PWM_OVERLAY); // Load the PWM main module.
        // Load the PWM pin module, and set it up.
        string basePath = PWM_BASEPATHS[device];
        loadOverlay("bone_pwm_" + PWM_PINS[device], basePath);
        #elif DEBIAN_VERSION == 8
        string basePath = PWM_CHIP_PATHS[device] + "pwm" + PWM_CHANNELS[device]
                          + "/";
        loadOverlay(PWM_OVERLAY, PWM_CHIP_PATHS[device] + "export"); // Load the PWM main module.

        // Load the PWM pin module.
        execBashCommand("echo " + PWM_CHANNELS[device] + " > "
                        + PWM_CHIP_PATHS[device] + "export");
        #endif

        // Setup the PWM pin module.
        period = (int) PWM_PERIOD;

        if(!writeToDeviceFile(basePath + PWM_DUTY_FILENAME, "0") ||
           !writeToDeviceFile(basePath + "period", to_string(period)) ||
           !writeToDeviceFile(basePath + "polarity", PWM_POLARITY_VALUE))
        {
            throw runtime_error("PWM: could not set a device file: timeout.");
        }

        // Open the file device to set the period later.
        string periodPath = execBashCommandWithResult("ls " + basePath +
                                                      PWM_PERIOD_FILENAME);
        if(periodPath.back() == '\n')
            periodPath.pop_back();

        periodFile.open(periodPath);
        if(!periodFile.is_open())
            throw runtime_error("PWM: can't open period file.");

        // Open the file device to set the duty later.
        string dutyPath = execBashCommandWithResult("ls " + basePath +
                                                    PWM_DUTY_FILENAME);
        if(dutyPath.back() == '\n')
            dutyPath.pop_back();

        dutyFile.open(dutyPath);
        if(!dutyFile.is_open())
            throw runtime_error("PWM: can't open duty file.");

        #if DEBIAN_VERSION == 8
        // Enable the PWM module.
        execBashCommand("echo 1 > " + basePath + "enable");
        #endif
    }
    catch(runtime_error)
    {
        state = PeripheralState::FAULT;
        return;
    }

    state = PeripheralState::ACTIVE;
#else
    state = PeripheralState::DISABLED;
#endif
}

Pwm::~Pwm()
{
    setDuty(0.0f);
}

/**
 * @brief Sets the PWM frequency.
 * @param frequency the PWM frequency [Hz].
 */
void Pwm::setFrequency(float frequency)
{
    // Compute the period.
    period = (int)(1000000000.0f / frequency); // [ns].

#ifdef __arm__
    try
    {
        // Write to the device file.
        periodFile << period << flush;
    }
    catch(std::ios_base::failure)
    {
        periodFile.clear();
        debug << "Error while setting PWM period." << endl;
    }
#endif
}

/**
 * @brief Set the PWM duty.
 * @param dutyRatio the new PWM ON time ratio [0.0-1.0].
 */
void Pwm::setDuty(float dutyRatio)
{
    dutyRatio = clamp(dutyRatio, 0.0f, 1.0f);

    this->dutyRatio = dutyRatio;

#ifdef __arm__
    try
    {
        int onTime = (int)(dutyRatio * period);
        dutyFile << onTime << flush;
    }
    catch(std::ios_base::failure)
    {
        dutyFile.clear();
        debug << "Error while setting PWM duty." << endl;
    }
#endif
}

/**
 * @brief Pwm::getDuty
 * @return The current PWM duty.
 */
float Pwm::getDuty() const
{
    return dutyRatio;
}
