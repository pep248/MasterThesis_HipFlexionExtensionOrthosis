#include "beeper.h"
#include "utils.h"

using namespace std;

/**
 * @brief Constructor.
 * @param pwmDevice
 */
Beeper::Beeper(Pwm_Device pwmDevice) : pwm(pwmDevice)
{
    state = pwm.getState();

    if(state == PeripheralState::ACTIVE)
        pwm.setDuty(0.0f);

    beepRemainingTime = 0.0f;
}

/**
 * @brief Destructor.
 */
Beeper::~Beeper()
{
    pwm.setDuty(0.0f);
}

/**
 * @brief Plays a single beep.
 * @param frequency tone frequency [Hz].
 * @param strength tone duty cycle [0.0-1.0].
 * @param duration tone duration [s].
 * @note This will abort the beep sequence currently being played, if any.
 */
void Beeper::beep(float frequency, float strength, float duration)
{
    beepsQueue.clear();
    setPwm(frequency, strength, duration);
}

/**
 * @brief Plays a sequence of beeps.
 * @param beepSequence sequence of tones to be played sequentially
 * @note This will abort the beep sequence currently being played, if any.
 */
void Beeper::playSequence(std::list<Beep> beepSequence)
{
    beepsQueue = beepSequence;

    if(!beepsQueue.empty())
    {
        // Start playing the next beep from the queue.
        Beep b = beepsQueue.front();
        beepsQueue.pop_front();
        setPwm(b.frequency, b.strength, b.duration);
    }
}

/**
 * @brief Updates the beeper state.
 * @param dt timestamp [s].
 */
void Beeper::update(float dt)
{
    if(beepRemainingTime > 0.0f)
    {
        beepRemainingTime -= dt;

        if(beepRemainingTime <= 0.0f)
        {
            if(beepsQueue.empty())
                pwm.setDuty(0.0f); // No more beeps to play, stop.
            else
            {
                // Start playing the next beep from the queue.
                Beep b = beepsQueue.front();
                beepsQueue.pop_front();
                setPwm(b.frequency, b.strength, b.duration);
            }
        }
    }
}

/**
 * @brief Emits a beep.
 * @param frequency tone frequency [Hz].
 * @param strength tone duty cycle [0.0-1.0].
 * @param duration duration of the tone [s].
 */
void Beeper::setPwm(float frequency, float strength, float duration)
{
    // Convert the strength to a PWM duty.
    float pwmDuty = powf(strength, 4.0f) / 2.0f; // Compensate for the non linear response of the loudspeader.

    // Set the PWM.
    pwm.setFrequency(frequency);

    clamp(strength, 0.0f, 1.0f);
    pwm.setDuty(pwmDuty);

    beepRemainingTime = duration;
}
