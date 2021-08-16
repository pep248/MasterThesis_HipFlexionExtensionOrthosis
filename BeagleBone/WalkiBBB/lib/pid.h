#ifndef DEF_LIB_PID_H
#define DEF_LIB_PID_H

#include "syncvar/syncvar.h"
#include "filters/lowpassfilter.h"

/**
 * @defgroup PID PID
 * @brief Proportional-integral-derivate regulator.
 * Simple implementation of a parallel form PID regulator, with optional
 * anti-reset windup (ARW).
 * @ingroup Lib
 * @{
 */

/**
 * @brief PID regulator.
 * @remark The units are not specified, so the user can choose consistently.
 */
class Pid
{
public:
    Pid(float kp, float ki, float kd, float arw, float dFilterTau = 0.0f);
    float update(float current, float target, float dt);
    float getLastCommand() const;
    void reset();
    void freezeIntegrator();

    void setKp(float kp);
    void setKi(float ki, bool resetIntegrator = true);
    void setKd(float kd);
    void setArw(float arw);
    void setDFilter(float tau);

    float &getIntegrator();

    SyncVarList getVars() const;

private:
    float kp; ///< Coefficient of the proportional part [out_unit/in_unit].
    float ki; ///< Coefficient of the integral part [out_unit/(in_unit.s)]. A negative or zero value means the integral effect is disabled.
    float kd; ///< Coefficient of the derivative part [out_unit/(in_unit/s)].
    float arw; ///< Saturation value of the integrator [out_unit]. A negative or zero value means the ARW is disabled.
    LowPassFilter dFilter;

    float currentState; ///< Actual value of the input state to control [in_unit].
    float targetState; ///< Target value of the input state to control [in_unit].
    float integrator; ///< Value of the integrator [out_unit].
    float previousError; ///< Difference between the target state and the actual state [in_unit].
    float command; ///< Last computed output [out_unit].
    bool firstSample; ///< Allows to handle the special case of the first sample, because it is not yet possible to compute the derivative.
    float previousIntegrator; ///< Previous value of the integrator [out_unit].
    SyncVarList syncVars; ///< SyncVars to tune easily the parameters at runtime.
};

/**
 * @}
 */

#endif
