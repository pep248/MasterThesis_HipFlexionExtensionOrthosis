#include "pid.h"

#include "utils.h"

using namespace std;
using namespace Utils;

/**
 * @brief Constructor.
 * @param kp coefficient of the proportional part [out_unit/in_unit].
 * @param ki coefficient of the integral part [out_unit/(in_unit.s)]. A negative
 * or zero value disables the integral effect.
 * @param kd coefficient of the derivative part [out_unit/(in_unit/s)].
 * @param arw saturation value of the integrator [out_unit]. A negative or zero
 * value disables the ARW.
 * @param dFilterTau cut-off period [s] of the low pass filter for the
 * derivative part. If zero or negative, this filter is disabled.
 */
Pid::Pid(float kp, float ki, float kd, float arw, float dFilterTau) :
    kp(kp), ki(ki), kd(kd), arw(arw), dFilter(dFilterTau, 0.0f)
{
    reset();

    syncVars.add("kp", "", this->kp, VarAccess::READWRITE, false);
    syncVars.add("ki", "", this->ki, VarAccess::READWRITE, false);
    syncVars.add("kd", "", this->kd, VarAccess::READWRITE, false);
    syncVars.add("arw", "", this->arw, VarAccess::READWRITE, false);
    syncVars.add("d_filt_tau", "s", dFilter.getTau(), VarAccess::READWRITE,
                 false);
    syncVars.add("integrator", "", integrator, VarAccess::READWRITE, false);
    syncVars.add("current_state", "", currentState, VarAccess::READ, false);
    syncVars.add("target_state", "", targetState, VarAccess::READ, false);
    syncVars.add("command", "", command, VarAccess::READ, false);
}

/**
 * @brief Computes the new command value, from the current and target states.
 * @param current actual value of the input state to control [in_unit].
 * @param target target value of the input state to control [in_unit].
 * @param dt time elapsed since the last call to this method [s].
 * @return the new command [out_unit].
 */
float Pid::update(float current, float target, float dt)
{
    currentState = current;
    targetState = target;

    float error = target - current;

    // Proportional part.
    command = error * kp;

    // Integral part.
    previousIntegrator = integrator;
    integrator += ki * (error * dt);

    if(arw > 0.0f)
        integrator = clamp(integrator, -arw, arw);

    command += integrator;

    // Derivative part.
    if(!firstSample)
    {
        dFilter.update(error, dt);
        command += (dFilter.get() - previousError) * kd / dt;
    }

    previousError = dFilter.get();
    firstSample = false;

    return command;
}

/**
 * @brief Gets the command computed by the last call of update().
 * @return The last computed command [out_unit].
 */
float Pid::getLastCommand() const
{
    return command;
}

/**
 * @brief Resets the PID.
 */
void Pid::reset()
{
    command = 0.0f;
    previousIntegrator = 0.0f;
    integrator = 0.0f;
    firstSample = true;
    dFilter.reset(0.0f);
}

/**
 * @brief Prevent the integrator from updating its value.
 * Calling this method will revert the integrator back to the value before the
 * last call to update(). Calling this method after (or before) each call to
 * update() will then approximately "freeze" the integrator value.
 */
void Pid::freezeIntegrator()
{
    integrator = previousIntegrator;
}

/**
 * @brief Sets the Kp coefficient.
 * @param kp the new value of the proportional coefficient [out_unit/in_unit].
 */
void Pid::setKp(float kp)
{
    this->kp = kp;
}

/**
 * @brief Sets the Ki coefficient.
 * @param ki the new value of the integral coefficient [out_unit/(in_unit.s)].
 * A negative or zero value disables the integral effect.
 * @param resetIntegrator true to set the integrator to zero, false to keep its
 * current value.
 */
void Pid::setKi(float ki, bool resetIntegrator)
{
    this->ki = ki;

    if(resetIntegrator)
        integrator = 0.0f;
}

/**
 * @brief Sets the Kd coefficient.
 * @param kd the new value of the derivative coefficient [out_unit/(in_unit/s)].
 */
void Pid::setKd(float kd)
{
    this->kd = kd;
}

/**
 * @brief Sets the ARW coefficient.
 * @param arw the new value of the ARW coefficient [out_unit]. A negative or
 * zero value disables the integral effect.
 */
void Pid::setArw(float arw)
{
    this->arw = arw;
}

/**
 * @brief Sets the strength of the error filter of the derivative effect.
 * @param tau cut-off period [s].
 */
void Pid::setDFilter(float tau)
{
    dFilter.setTau(tau);
}

/**
 * @brief Gets the current integrator value.
 * @return the integrator value [out_unit].
 */
float &Pid::getIntegrator()
{
    return integrator;
}

/**
 * @brief Gets the Syncvars of the PID.
 * @return the list of SyncVars of this PID.
 */
SyncVarList Pid::getVars() const
{
    return syncVars;
}
