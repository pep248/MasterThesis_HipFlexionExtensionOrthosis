#ifndef EWALKTIMEBASEDTORQUEPROFILE_H
#define EWALKTIMEBASEDTORQUEPROFILE_H

#include "controllers/controller.h"

//device-specific headers
#include "ewalkdefinitions.h"
#include "../../drivers/canbus.h"
#include "../../drivers/gyems.h"

//controller-specific headers
#include "../../drivers/ads7844.h"

#define MAIN_LOOP_PERIOD 0.002f ///< Main loop period [s].
#define GAIT_CYCLE_AVERAGING_PERIOD 5   //No. of last GCs based on which the average
                                        //GC time is calculated.
#define MAX_GC_DURATION 2.0f            //Max. duration of GC [s], used for detecting
                                        //standstill periods


/**
 * @brief A controller to apply a predefined time-based torque profile. The time
 * variable is actually the gait cycle percentage [0-100%] which starts at heel-
 * strike. The percent gait cycle is approximated online by dividing the time
 * since last heel-strike by the average duration of the last few gait cycles
 * determined by "GAIT_CYCLE_AVERAGING_PERIOD".
 */
class eWalkTimeBasedTorqueProfile : public Controller
{
public:
    eWalkTimeBasedTorqueProfile(PeripheralsSet peripherals);
    ~eWalkTimeBasedTorqueProfile();

    void update(float dt) override;
    void handleCanCommunication();

    float soleVoltageToForce(float voltage);
    void updateFootLoads(float dt);
    void updateGaitCycleDuration();
    float getTorqueFromProfile(float percentGc);
    float computeTorqueRight();
    float computeTorqueLeft();


private:
    CanBus can;
    Gyems leftMotor, rightMotor;

    SpiChannel leftFootImuSpiChannel, rightFootImuSpiChannel;
    Ads7844 leftSole, rightSole;

    std::thread *canThread;
    volatile bool stopCanThread;

    float leftHipAngle, rightHipAngle;  ///< [deg]
    float leftHipSpeed, rightHipSpeed;  ///< [deg/s]
    float leftTorque, rightTorque;    ///< Measured torques [N.m]

    VecNf<8> leftSoleVoltages, rightSoleVoltages;
    VecNf<8> leftLoads, rightLoads;
    float leftFootLoad, rightFootLoad;  ///< [N]

    bool leftInStance, rightInStance;
    float stanceFootLoadThreshold;      ///< [N]
    float pilotBodyWeight;              ///< [kg]
    float baselineGcDuration;           ///< [s]
    float percentAssistance;            ///< [%] The fraction of the torque profile that will be applied
    bool startController;

    float timeSinceLeftHeelStrike, timeSinceRightHeelStrike;    ///< [s]
    std::array<float, GAIT_CYCLE_AVERAGING_PERIOD> lastGaitCycleTimes;
    float averageGaitCycleTime;     ///< GC time averaged over the last N GCs [s]
    float leftGaitCyclePercent, rightGaitCyclePercent; ///< %GC for the left and right leg []
    float leftTorqueCmd, rightTorqueCmd;    ///< [N.m]

    float last_heel_strike_left, current_heel_strike_left; ///< [s]
    float original_period_left, previous_step_period_left, new_period_left, previous_step_duration_left, theoretical_period_left, theoretical_period_corrected_left; ///< [s]
    float performed_gait_left; ///< [%]
    float time_offset_left; ///< [s]
    float first_step_left; ///< <<[0-1]
    float sine_torque_left;
    float control_ratio_left;
    

    float last_heel_strike_right, current_heel_strike_right; ///< [s]
    float original_period_right, previous_step_period_right, new_period_right, previous_step_duration_right, theoretical_period_right, theoretical_period_corrected_right; ///< [s]
    float performed_gait_right; ///< [%]
    float time_offset_right; ///< [s] 
    float first_step_right; ///< <<[0-1]
    float sine_torque_right;
    float control_ratio_right;

    float time_right, time_left;
    float math_time;
    int redy_to_go;

    float previous_gain_right, desired_gain_right, current_gain_right;
    float previous_gain_left, desired_gain_left, current_gain_left;

};

typedef eWalkTimeBasedTorqueProfile SelectedController;


// Data for the torque profile, taken from the appendix of the textbook
// "The biomechanics and motor control of human gait" by D. A. Winter (1991).
// The left column is the percent gait cycle, the right column is the hip torque
// normalized by bodyweight (N.m/Kg). In the original textbook, the signs are
// inverted because in the book, extension torques are defined as positive.
typedef struct { float percentGc; float torquePerBodyweight; } TorquePoint_T;


#endif // EWALKTIMEBASEDTORQUEPROFILE_H
