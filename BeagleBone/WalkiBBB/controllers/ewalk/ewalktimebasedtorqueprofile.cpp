#include "ewalktimebasedtorqueprofile.h"

#include <algorithm>    // For rotate() and clamp() functions
#include <numeric>      // For accumulate() function

#include "../../lib/debugstream.h"
#include "../../lib/utils.h"


#include <tgmath.h>


using namespace std;
using namespace chrono;

const float SOLES_EXCIT_VOLTAGE = 3.3f; // [V].
const float ADC_REF = 1.243f; // [V].
const float SOLE_ASSOCIATED_RESISTANCE = 3000.0f; // [ohm].
const float SOLE_COEF_A = 2.8900e5f; // [ohm/N].
const float SOLE_COEF_B = 4.7076e03f; // [ohm].


/**

//BOOK GAIT

const float a1 = 0.4314f;
const float a2 = 0.0932f;
const float a3 = 0.0725f;
const float a4 = 0.0f;
const float a5 = 0.0f;
const float a6 = 0.0f;
const float a7 = 0.0f;
const float a8 = 0.0f;

const float b1 = 3.1417f;
const float b2 = 9.4295f;
const float b3 = 6.2821f;
const float b4 = 0.0f;
const float b5 = 0.0f;
const float b6 = 0.0f;
const float b7 = 0.0f;
const float b8 = 0.0f;

const float c1 = -1.8151f;
const float c2 = -2.0574f;
const float c3 =  2.5740f;
const float c4 = 0.0f;
const float c5 = 0.0f;
const float c6 = 0.0f;
const float c7 = 0.0f;
const float c8 = 0.0f;

**/

/**

//ALPHA

const float a1 = +0.451135f;
const float a2 = -0.295392f;
const float a3 = +0.099296f;
const float a4 = 0.0f;
const float a5 = 0.0f;
const float a6 = 0.0f;
const float a7 = 0.0f;
const float a8 = 0.0f;

const float b1 = 3.1417f;
const float b2 = 9.4295f;
const float b3 = 6.2821f;
const float b4 = 0.0f;
const float b5 = 0.0f;
const float b6 = 0.0f;
const float b7 = 0.0f;
const float b8 = 0.0f;

const float c1 = -1.851456f;
const float c2 = -1.017760f;
const float c3 = +4.169075f;
const float c4 = 0.0f;
const float c5 = 0.0f;
const float c6 = 0.0f;
const float c7 = 0.0f;
const float c8 = 0.0f;

**/

//BETHA

const float a1 = +0.310482f;
const float a2 = +0.006469f;
const float a3 = +0.111714f;
const float a4 = 0.0f;
const float a5 = 0.0f;
const float a6 = 0.0f;
const float a7 = 0.0f;
const float a8 = 0.0f;

const float b1 = 3.1417f;
const float b2 = 9.4295f;
const float b3 = 6.2821f;
const float b4 = 0.0f;
const float b5 = 0.0f;
const float b6 = 0.0f;
const float b7 = 0.0f;
const float b8 = 0.0f;

const float c1 = -1.779109f;
const float c2 = -2.021154f;
const float c3 = +2.535488f;
const float c4 = 0.0f;
const float c5 = 0.0f;
const float c6 = 0.0f;
const float c7 = 0.0f;
const float c8 = 0.0f;

const float pi = 3.14159f;

const float torque_multiplier = -0.22959f;

float period = 1/(b1/(2*pi));



//float fake_period = 4; //sim
//int merda = 0; //sim
//float residu = 0;// sim

//int mutex_right = 0; //sim
//int mutex_left = 1; //sim






TorquePoint_T winterHipTorqueProfile1[51] =
{
    {0.00 , -0.249},
    {0.02 , -0.600},
    {0.04 , -0.556},
    {0.06 , -0.416},
    {0.08 , -0.359},
    {0.10 , -0.305},
    {0.12 , -0.245},
    {0.14 , -0.159},
    {0.16 , -0.084},
    {0.18 , -0.000},
    {0.20 , 0.064},
    {0.22 , 0.092},
    {0.24 , 0.098},
    {0.26 , 0.092},
    {0.28 , 0.085},
    {0.30 , 0.088},
    {0.32 , 0.100},
    {0.34 , 0.130},
    {0.36 , 0.168},
    {0.38 , 0.199},
    {0.40 , 0.231},
    {0.42 , 0.269},
    {0.44 , 0.312},
    {0.46 , 0.364},
    {0.48 , 0.401},
    {0.50 , 0.404},
    {0.52 , 0.356},
    {0.54 , 0.262},
    {0.56 , 0.251},
    {0.58 , 0.310},
    {0.60 , 0.344},
    {0.62 , 0.295},
    {0.64 , 0.228},
    {0.66 , 0.169},
    {0.68 , 0.126},
    {0.70 , 0.089},
    {0.72 , 0.069},
    {0.74 , 0.057},
    {0.76 , 0.044},
    {0.78 , 0.026},
    {0.80 , 0.009},
    {0.82 , -0.008},
    {0.84 , -0.029},
    {0.86 , -0.060},
    {0.88 , -0.106},
    {0.90 , -0.170},
    {0.92 , -0.242},
    {0.94 , -0.296},
    {0.96 , -0.301},
    {0.98 , -0.237},
    {1.00 , -0.118}
};


eWalkTimeBasedTorqueProfile::eWalkTimeBasedTorqueProfile(PeripheralsSet peripherals):
    Controller("eWalk Time-Based Josep", peripherals),
    leftMotor(&can, 2, LEFT_MOTOR_SIGN, LEFT_ANGLE_OFFSET),
    rightMotor(&can, 1, RIGHT_MOTOR_SIGN,RIGHT_ANGLE_OFFSET),
    leftFootImuSpiChannel(*peripherals.spiBus, SpiBus::CS_LEFT_FOOT_MPU),
    rightFootImuSpiChannel(*peripherals.spiBus, SpiBus::CS_RIGHT_FOOT_MPU),
    leftSole(*peripherals.spiBus, SpiBus::CS_RIGHT_ADC, ADC_REF),
    rightSole(*peripherals.spiBus, SpiBus::CS_LEFT_ADC, ADC_REF)
{   
    //Initialize controller constant parameters
    stanceFootLoadThreshold = 0.5f;
    pilotBodyWeight = 60.0f;
    baselineGcDuration = 1.0f;
    percentAssistance = 25.0f;

    syncVars.push_back(makeSyncVar("enable_controller", "0/1", startController,
                                   VarAccess::READWRITE, true));

    syncVars.push_back(makeSyncVar("left_hip_angle", "deg", leftHipAngle,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("right_hip_angle", "deg", rightHipAngle,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("left_hip_speed", "deg/s", leftHipSpeed,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("right_hip_speed", "deg/s", rightHipSpeed,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("left_torque_cmd", "N.m", leftTorqueCmd,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("right_torque_cmd", "N.m", rightTorqueCmd,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("left_gc_percent", "%", leftGaitCyclePercent,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("right_gc_percent", "%", rightGaitCyclePercent,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("avg_GC_time", "s", averageGaitCycleTime,
                                   VarAccess::READ, true));

    // Params that are only needed for troubleshooting and checking
    syncVars.push_back(makeSyncVar("check/left_stance?", "",leftInStance,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/right_stance?", "", rightInStance,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/left_foot_load", "N", leftFootLoad,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/right_foot_load", "N", rightFootLoad,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/left_torque_actual", "N.m", leftTorque,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/right_torque_actual", "N.m", rightTorque,
                                   VarAccess::READ, true));

    syncVars.push_back(makeSyncVar("check/sine_torque_right", "N.m", sine_torque_right,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/sine_torque_left", "N.m", sine_torque_left,
                                   VarAccess::READ, true));

    syncVars.push_back(makeSyncVar("check/math_time", "s", math_time,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/new_period_left", "s", new_period_left,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/new_period_right", "s", new_period_right,
                                   VarAccess::READ, true));
                                   

    //syncVars.push_back(makeSyncVar("check/fake_period", "s", fake_period,
    //                               VarAccess::READWRITE, true));

    syncVars.push_back(makeSyncVar("check/control_ratio_right", "k", control_ratio_right,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/current_gain_right", "k", current_gain_right,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/performed_gait_right", "%", performed_gait_right,
                                   VarAccess::READ, true));

    syncVars.push_back(makeSyncVar("check/control_ratio_left", "k", control_ratio_left,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/current_gain_left", "k", current_gain_left,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/performed_gait_left", "%", performed_gait_left,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/ready_to_go", "0-1", redy_to_go,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("check/first_step_left", "0-1", first_step_left,
                                   VarAccess::READ, true));

    // Constants
    syncVars.push_back(makeSyncVar("const/percent_assist", "0-100", percentAssistance,
                                   VarAccess::READWRITE, true));
    syncVars.push_back(makeSyncVar("const/bodyweight", "Kg", pilotBodyWeight,
                                   VarAccess::READWRITE, true));
    syncVars.push_back(makeSyncVar("const/baseline_GC_duration", "s", baselineGcDuration,
                                   VarAccess::READWRITE, true));
    syncVars.push_back(makeSyncVar("const/stance_footload_thresh", "N", stanceFootLoadThreshold,
                                   VarAccess::READWRITE, true));
    /*
    for(int i=0; i<8; i++)
        syncVars.push_back(makeSyncVar("left_sole/cell_" + std::to_string(i),
                                       "N", leftLoads[i], VarAccess::READ, true));
    for(int i=0; i<8; i++)
        syncVars.push_back(makeSyncVar("right_sole/cell_" + std::to_string(i),
                                       "N", rightLoads[i], VarAccess::READ, true));
    */

    // Creating the thread for handling the CAN communication with the motors
    canThread = new thread(&eWalkTimeBasedTorqueProfile::handleCanCommunication, this);

    // Setting the thread priority for CAN communication
    struct sched_param sp;
    sp.sched_priority = 2;
    pthread_setschedparam(canThread->native_handle(), SCHED_RR, &sp);

    // Set some initial values for the GC times
    lastGaitCycleTimes.fill(baselineGcDuration);
    averageGaitCycleTime = baselineGcDuration;
    percentAssistance = 0.0f;
    redy_to_go = 0;
    startController = false;
    math_time = 0;

    time_right = 0; ///< [s]
    time_left = 0; ///< [s]

    original_period_right = period; previous_step_period_right = period; new_period_right = period; previous_step_duration_right = period; theoretical_period_right = period; theoretical_period_corrected_right = period; ///< [s]
    original_period_left = period; previous_step_period_left = period; new_period_left = period; previous_step_duration_left = period; theoretical_period_left = period; theoretical_period_corrected_left = period; ///< [s]

    performed_gait_right = 0.5; ///< [%]
    performed_gait_left = 0.5; ///< [%]

    time_offset_right = 0; ///< [s]
    time_offset_left = 0; ///< [s]

    previous_gain_right = 1; desired_gain_right = 1; current_gain_right = 1;
    previous_gain_left = 1; desired_gain_left = 1; current_gain_left = 1;

    control_ratio_right = 0.5;
    control_ratio_left = 0.5;

    first_step_left = 0;
    first_step_right = 0;

    sine_torque_left = 0;
    sine_torque_right = 0;

}

eWalkTimeBasedTorqueProfile::~eWalkTimeBasedTorqueProfile()
{
    rightMotor.setTorque(0.0f);
    rightMotor.update(MAIN_LOOP_PERIOD);
    leftMotor.setTorque(0.0f);
    leftMotor.update(MAIN_LOOP_PERIOD);

    // Stop the CAN communication thread
    stopCanThread = true;
    canThread->join();
    delete canThread;
}

/**
 * @brief update function of the controller to be run at each time step.
 */
void eWalkTimeBasedTorqueProfile::update(float dt)
{

    math_time = math_time + dt;

    // Get the current joint positions.
    leftHipAngle = leftMotor.getPosition();
    rightHipAngle = rightMotor.getPosition();
    leftHipSpeed = leftMotor.getSpeed();
    rightHipSpeed = rightMotor.getSpeed();
    leftTorque = leftMotor.getTorque();
    rightTorque = rightMotor.getTorque();

    // If the values received from the motorboard are bogus, emergency stop.
    if(leftHipAngle < -180.0f || leftHipAngle > 180.0f ||
       rightHipAngle < -180.0f || rightHipAngle > 180.0f)
    {
        debug<<"Abnormal joint angles."<<endl;
        //motorBoard.disarmBridges();
        //throw runtime_error("Bogus hip angles received from the motorboard.");
    }

    // Acquire the instrumented soles and update footLoad values
    updateFootLoads(dt);


    if(startController)
    {
        // Heel-strike detection and avg. GC time calculation
        updateGaitCycleDuration();


        //Calculate torque commands
        leftTorqueCmd = computeTorqueRight();
        rightTorqueCmd = computeTorqueLeft();

        leftTorqueCmd *= percentAssistance / 100.0f;
        rightTorqueCmd *= percentAssistance / 100.0f;

        leftMotor.setTorque(leftTorqueCmd);
        rightMotor.setTorque(rightTorqueCmd);

        timeSinceLeftHeelStrike += dt; //unused
        timeSinceRightHeelStrike += dt; //unused

    }
    else if(fabsf(averageGaitCycleTime - baselineGcDuration) > 0.1f)
    {
        //Reset GC duration data to the assumed baseline once the controller
        //is disabled.
        lastGaitCycleTimes.fill(baselineGcDuration);
        averageGaitCycleTime = baselineGcDuration;
    }
    else
    {
        leftMotor.setTorque(0.0f);
        rightMotor.setTorque(0.0f);
    }
}

void eWalkTimeBasedTorqueProfile::updateFootLoads(float dt)
{
    leftSole.update(dt);
    rightSole.update(dt);

    leftSoleVoltages = leftSole.getLastSamples();
    rightSoleVoltages = rightSole.getLastSamples();

    for(int i=0; i<8; i++)
    {
        leftLoads[i] = soleVoltageToForce(leftSoleVoltages[i]);
        rightLoads[i] = soleVoltageToForce(rightSoleVoltages[i]);
    }
    leftFootLoad = leftLoads.sum();
    rightFootLoad = rightLoads.sum();
}

/**
 * @brief Updates the average duration of the GC by detecting heel-strikes, counting the time
 * between 2 consecutive heel-strikes and averaging it over the last N gait cycles, where N is
 * defined by the GAIT_CYCLE_AVERAGING_PERIOD constant in the .h file. For heel-strike
 * detection, first we wait for each leg to enter swing (detected when footLoad < threshold),
 * then as soon as the footLoad is above the threshold value, heel-strike is detected.
 */
void eWalkTimeBasedTorqueProfile::updateGaitCycleDuration()
{
    /**
    residu = fmod(math_time,fake_period);
    if (residu >= fake_period/2 )
    {
        merda = 1;
    }
    else
    {
        merda = 0;
    }
    **/


    if(!leftInStance)   //left leg in swing
    //if(merda == 0)   //left leg in swing
    {
        if(leftFootLoad > stanceFootLoadThreshold)  // left heel-strike detected
        //if (merda == 0 && mutex_left == 1)
        {
            if (first_step_left == 0)  // we execute this at the first step to get a good first period
            {
                current_heel_strike_left = math_time;
                first_step_left = first_step_left + 1;
            }
            else
            {
                // update the time of the previous step gather the time of the current step
                last_heel_strike_left = current_heel_strike_left;
                current_heel_strike_left = math_time;

                // gather information from the past step
                previous_step_period_left = new_period_left; // period of the previous step
                previous_step_duration_left = current_heel_strike_left - last_heel_strike_left; //real duration of the previou step
                
                // we check how different was our step from what we expected
                performed_gait_left = performed_gait_left + (previous_step_duration_left/previous_step_period_left) + (-1); // if bigger  than 1 -> the step was slower than the torque
                                                                                                                    // if smaller than 1 -> the step was faster than the torque
                
                if (performed_gait_left >= 2)
                {
                    performed_gait_left = 1.9;
                }
                else if( performed_gait_left <= 0)
                {
                    performed_gait_left = 0.1;
                }

                // we compute the upcoming period
                //control_ratio_left = 0.1;
                control_ratio_left = 1;
                //control_ratio_left = ((1-abs(1-performed_gait_left)) / (abs(1-performed_gait_left)+1))*((1-abs(1-performed_gait_left)) / (abs(1-performed_gait_left)+1));
                //control_ratio_left = (1/(abs(1-performed_gait_left)+1)); //dynamic control ratio
                theoretical_period_left = previous_step_duration_left/(2-performed_gait_left);
                //theoretical_period_corrected_left = theoretical_period_corrected_left + ( - theoretical_period_corrected_left + theoretical_period_left ) * control_ratio_left; // desired duration of the following gait
                theoretical_period_corrected_left = theoretical_period_corrected_left + ( - theoretical_period_corrected_left + theoretical_period_left ) * control_ratio_left; // desired duration of the following gait
                new_period_left = theoretical_period_corrected_left; // period of the upcoming wave

                if (new_period_left < 0.2)
                {
                    new_period_left = previous_step_period_left;
                }

                // we update the time_offset (used to compute wave reset point) every time we detect a heel strike
                time_offset_left = math_time;
                desired_gain_left = (1/new_period_left)/(1/period);

                leftInStance = true;
                timeSinceLeftHeelStrike = 0.0f; // reset period counter

                //mutex_left = 0;
                //mutex_right = 1;
            }
        }
    }
    else                //leg in stance, check for switching to swing
    {
        if(leftFootLoad < stanceFootLoadThreshold)
            leftInStance = false;
    }

    if(!rightInStance)   //right leg in swing
    //if(merda == 1)   //right leg in swing
    {
        if(rightFootLoad > stanceFootLoadThreshold)
        //if(merda == 1 && mutex_right == 1)
        {

            if (first_step_right == 0)  // we execute this at the first step to get a good first period
            {

                current_heel_strike_right = 0;
                first_step_right = first_step_right + 1;
            }
            else
            {
                // update the time of the previous step gather the time of the current step
                last_heel_strike_right = current_heel_strike_right;
                current_heel_strike_right = math_time;

                // gather information from the past step
                previous_step_period_right = new_period_right; // period of the previous step
                previous_step_duration_right = current_heel_strike_right - last_heel_strike_right; //real duration of the previou step
                
                // we check how different was our step from what we expected
                performed_gait_right = performed_gait_right + (previous_step_duration_right/previous_step_period_right) + (-1); // if bigger  than 1 -> the step was slower than the torque
                                                                                                                    // if smaller than 1 -> the step was faster than the torque
                
                if (performed_gait_right >= 2)
                {
                    performed_gait_right = 1.9;
                }
                else if( performed_gait_right <= 0)
                {
                    performed_gait_right = 0.1;
                }

                // we compute the upcoming period
                //control_ratio_right = 0.1;
                control_ratio_right = 1;
                ///control_ratio_right = ((1-abs(1-performed_gait_right)) / (abs(1-performed_gait_right)+1))*((1-abs(1-performed_gait_right)) / (abs(1-performed_gait_right)+1));
                //control_ratio_right = (1/(abs(1-performed_gait_right)+1)); //dynamic control ratio
                theoretical_period_right = previous_step_duration_right/(2-performed_gait_right);
                //theoretical_period_corrected_right = theoretical_period_corrected_right + ( - theoretical_period_corrected_right + theoretical_period_right ) * control_ratio_right; // desired duration of the following gait
                theoretical_period_corrected_right = theoretical_period_corrected_right + ( - theoretical_period_corrected_right + theoretical_period_right ) * control_ratio_right; // desired duration of the following gait
                new_period_right = theoretical_period_corrected_right; // period of the upcoming wave

                if (new_period_right < 0.2)
                {
                    new_period_right = previous_step_period_right;
                }

                // we update the time_offset (used to compute wave reset point) every time we detect a heel strike
                time_offset_right = math_time;
                desired_gain_right = (1/new_period_right)/(1/period);


                rightInStance = true;
                timeSinceRightHeelStrike = 0.0f; // reset period counter

                //mutex_left = 1; //sim
                //mutex_right = 0; //sim
            }
        }
    }
    else                //leg in stance, check for switching to swing
    {
        if(rightFootLoad < stanceFootLoadThreshold)
            rightInStance = false;
    }

    if(performed_gait_right > 0.90 && performed_gait_right < 1.10 && performed_gait_left > 0.90 && performed_gait_left < 1.10)
    {
        redy_to_go = 1;
    }
    
}

/**
void eWalkTimeBasedTorqueProfile::updateGaitCycleDuration()
{
    if(!leftInStance)   //leg in swing
    {
        if(leftFootLoad > stanceFootLoadThreshold)  // heel-strike detected
        {
            leftInStance = true;

            if(timeSinceLeftHeelStrike <= MAX_GC_DURATION)
            {
                // Insert the new gait cycle duration into the latest GC times array
                rotate(lastGaitCycleTimes.begin(),lastGaitCycleTimes.begin()+1,
                       lastGaitCycleTimes.end());       //shift to the left
                lastGaitCycleTimes[lastGaitCycleTimes.size()-1] = timeSinceLeftHeelStrike;

                // Update the average GC duration
                averageGaitCycleTime = accumulate(lastGaitCycleTimes.begin(),
                                                  lastGaitCycleTimes.end(), 0.0f)
                                            / lastGaitCycleTimes.size();
            }

            // reset the GC timer
            timeSinceLeftHeelStrike = 0.0f;
        }
    }
    else                //leg in stance, check for switching to swing
    {
        if(leftFootLoad < stanceFootLoadThreshold)
            leftInStance = false;
    }

    if(!rightInStance)   //leg in swing
    {
        if(rightFootLoad > stanceFootLoadThreshold)
        {
            rightInStance = true;

            if(timeSinceRightHeelStrike <= MAX_GC_DURATION)
            {
                // Insert the new gait cycle duration into the latest GC times array
                rotate(lastGaitCycleTimes.begin(),lastGaitCycleTimes.begin()+1,
                       lastGaitCycleTimes.end());       //shift to the left
                lastGaitCycleTimes[lastGaitCycleTimes.size()-1] = timeSinceRightHeelStrike;

                // Update the average GC duration
                averageGaitCycleTime = accumulate(lastGaitCycleTimes.begin(),
                                                  lastGaitCycleTimes.end(), 0.0f)
                                            / lastGaitCycleTimes.size();
            }

            // reset the GC timer
            timeSinceRightHeelStrike = 0.0f;
        }
    }
    else                //leg in stance, check for switching to swing
    {
        if(rightFootLoad < stanceFootLoadThreshold)
            rightInStance = false;
    }
}
**/

float eWalkTimeBasedTorqueProfile::soleVoltageToForce(float voltage)
{
    float cellResistance = SOLES_EXCIT_VOLTAGE * SOLE_ASSOCIATED_RESISTANCE /
                           voltage - SOLE_ASSOCIATED_RESISTANCE; // [ohm].

    return SOLE_COEF_A / (cellResistance - SOLE_COEF_B); // [N].
}

/**
 * @brief Calculates the desired torque for a given %GC from the torque profile taken from
 * Winter's gait data. The desired torque is calculated using linear interpolation between the
 * data points in the table. Also uses the winterHipTorqueProfile and pilotBodyWeight parameters
 * defined earlier in this file.
 * @param percentGc Current percentage of the gait cycle [0-1]
 * @return Target torque based on the given profile [N.m].
 */
float eWalkTimeBasedTorqueProfile::getTorqueFromProfile(float percentGc)
{
    if(percentGc < 0.0f || percentGc > 1.0f)
        return 0.0f;    // percentGc has to be between 0 and 1

    uint8_t index = percentGc * 100 / 2;        //%GC of the torque profile increases with
                                                //2% increments, therefore the array index for
                                                //the nearest point in the profile can be
                                                //directly calculated from the %GC.

    //Linear interpolation
    float diffx = percentGc - winterHipTorqueProfile1[index].percentGc;
    float diffn = 0.02f;

    float normalizedTorque = winterHipTorqueProfile1[index].torquePerBodyweight
                + ( winterHipTorqueProfile1[index+1].torquePerBodyweight - winterHipTorqueProfile1[index].torquePerBodyweight ) * diffx / diffn;

    return normalizedTorque * pilotBodyWeight; //Torque values are normalized by bodyweight
}

float eWalkTimeBasedTorqueProfile::computeTorqueRight()
{
    if (redy_to_go == 1)
    {
        if (first_step_right == 1) // We start when we detect the first heel-strike
        {
            current_gain_right = current_gain_right + (desired_gain_right-current_gain_right)*0.001;
            //current_gain_right = 1;
            time_right = (math_time - time_offset_right + performed_gait_right*new_period_right) * ( original_period_right / new_period_right );
            sine_torque_right = current_gain_right*torque_multiplier*pilotBodyWeight*(a1 * sin(b1 * time_right + c1) + a2 * sin(b2 * time_right + c2) + a3 * sin(b3 * time_right + c3) + a4 * sin(b4 * time_right + c4) + a5 * sin(b5 * time_right + c5) + a6 * sin(b6 * time_right + c6) + a7 * sin(b7 * time_right + c7) + a8 * sin(b8 * time_right + c8) );
        }
    }
    return sine_torque_right; //Torque values are normalized by bodyweight
}

float eWalkTimeBasedTorqueProfile::computeTorqueLeft()
{
    if (redy_to_go == 1)
    {
        if (first_step_left == 1) // We start when we detect the first heel-strike
        {
            current_gain_left = current_gain_left + (desired_gain_left-current_gain_left)*0.001;
            //current_gain_left = 1;
            time_left = (math_time - time_offset_left + performed_gait_left*new_period_left) * ( original_period_left / new_period_left );
            sine_torque_left = current_gain_left*torque_multiplier*pilotBodyWeight*(a1 * sin(b1 * time_left + c1) + a2 * sin(b2 * time_left + c2) + a3 * sin(b3 * time_left + c3) + a4 * sin(b4 * time_left + c4) + a5 * sin(b5 * time_left + c5) + a6 * sin(b6 * time_left + c6) + a7 * sin(b7 * time_left + c7) + a8 * sin(b8 * time_left + c8) );
        }
    }
    return sine_torque_left; //Torque values are normalized by bodyweight
}


void eWalkTimeBasedTorqueProfile::handleCanCommunication()
{
    float dt = USEC_TO_SEC(CAN_UPDATE_PERIOD);
    system_clock::time_point nextExecTime = high_resolution_clock::now();

    stopCanThread = false;

    while(!stopCanThread)
    {
        //
        auto now = high_resolution_clock::now();

        //handle the CAN communication
        rightMotor.update(dt);
        leftMotor.update(dt);
        //can.Update();

        //
        nextExecTime = now + microseconds(CAN_UPDATE_PERIOD);
        std::this_thread::sleep_until(nextExecTime);
    }
}
