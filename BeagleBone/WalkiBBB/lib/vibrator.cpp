#include "vibrator.h"

#include <chrono>

#include "../config/config.h"
#include "utils.h"

using namespace std;
using namespace chrono;

#define VIBRATOR_TAU 0.035f // Vibrator time constant (acceleration time) [s].
#define STARTING_TIME 0.010f // [s].
#define SPEED_DEADZONE 0.05f // At a speed lower than this threshold, no voltage will be applied [0.0-1.0].
#define MIN_DUTY 0.10f // Minimal pwm duty to maintain the vibrator movement [0.0-1.0].
#define MAX_DUTY 1.0f // Maximum pwm duty (greater duties do not significantly increase the speed) [0.0-1.0].

/**
 * @brief Constructor.
 * @param pwmDevice PWM device the vibrator is connected to.
 */
Vibrator::Vibrator(Gpio_Id gpioDevice) :
    gpio(gpioDevice), estimatedSpeed(VIBRATOR_TAU, 0.0f)
{
    gpio.setupAsOutput();

    targetSpeed = 0.0f;
    command = false;

    remainingStartingTime = STARTING_TIME;

    lastUpdateTime = high_resolution_clock::now();
}

Vibrator::~Vibrator()
{
    gpio.setPinState(false);
}

/**
 * @brief Updates the speed of the vibrator.
 * This function should be called often (at every controller timestep), in order
 * to ensure a good timing accuracy.
 * @param dt the time elapsed since the last call to this function [s].
 */
void Vibrator::update(float)
{
    // Temporary: compute the dt here, to minimize the jitter.
    // Update the timestamp and compute the dt.
    auto now = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(now-lastUpdateTime);
    float dt = USEC_TO_SEC(duration.count());
    lastUpdateTime = now;

    // If the dt seems to be incoherent (may happen when the time is set),
    // use the nominal value to avoid further computation errors.
    if(dt < 0.0f || dt > MAIN_LOOP_PERIOD * 100.0f)
        dt = MAIN_LOOP_PERIOD;

    // Update the vibrator model.
    estimatedSpeed.update((float)command, dt);

    //
    if(targetSpeed > 0.0f)
    {
        if(remainingStartingTime > 0.0f)
        {
            // The motor needs to start, and overcome stiction. Apply full
            // voltage for a short time.
            remainingStartingTime -= dt;
            command = true;
        }
        else
        {
            // The motor is already spinning, regulate the speed, using a basic
            // model (no feedback loop) and a "on-off" controller.
            command = estimatedSpeed.get() < targetSpeed;
        }
    }
    else
    {
        remainingStartingTime = STARTING_TIME;
        command = false;
    }

    // Apply the computed command.
    gpio.setPinState(command);
}

/**
 * @brief Sets the vibrator speed.
 * @param speed the new vibrator speed [0.0-1.0].
 */
void Vibrator::setSpeed(float speed)
{
    if(speed < SPEED_DEADZONE)
        targetSpeed = 0.0f;
    else
    {
        clamp(speed, 0.0f, 1.0f);
        targetSpeed = Utils::map(speed, 0.0f, 1.0f, MIN_DUTY, MAX_DUTY);
    }
}
