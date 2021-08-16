#include "ewalkcontroller.h"

#include "../../lib/debugstream.h"
#include "../../lib/utils.h"

using namespace std;
using namespace chrono;

eWalkController::eWalkController(PeripheralsSet peripherals):
    Controller("eWalk Setup & Test", peripherals),
    rightMotor(&can, 1, RIGHT_MOTOR_SIGN,RIGHT_ANGLE_OFFSET),
    leftMotor(&can, 2, LEFT_MOTOR_SIGN, LEFT_ANGLE_OFFSET)
{
    // initialize variables
    startTorqueTest = false;
    rightMotorCtrlMode = 0;
    rightMotorAngleCmd = 0.0f;
    rightMotorSpeedCmd = 0.0f;
    rightMotorTorqueCmd = 0.0f;

    leftMotorCtrlMode = 0;
    leftMotorAngleCmd = 0.0f;
    leftMotorSpeedCmd = 0.0f;
    leftMotorTorqueCmd = 0.0f;

    torqueMagnitude = 0.0f;     // [Nm]
    torqueFrequency = 0.25f;    // [Hz]
    sineTimer = 0.0f;           // [s]

    // add syncVars to be monitored/controlled from the remote PC
    SyncVarList gyemsRightMotorVars, gyemsLeftMotorVars;
    gyemsRightMotorVars.push_back(makeSyncVar("multiturn_pos", "deg", rightMotorAngle, VarAccess::READ, true));
    gyemsRightMotorVars.push_back(makeSyncVar("position", "deg", rightAngle, VarAccess::READ, true));
    gyemsRightMotorVars.push_back(makeSyncVar("speed", "deg/s", rightSpeed, VarAccess::READ, true));
    gyemsRightMotorVars.push_back(makeSyncVar<float>("offset", "deg",
                                                   nullptr,
                                        [=](float offsetAng)
                                        {
                                            rightMotor.setAngleOffset(offsetAng);
                                        } , false));
    gyemsRightMotorVars.push_back(makeSyncVar("control_mode", "1:pos, 2:spd", rightMotorCtrlMode, VarAccess::READWRITE, false));
    gyemsRightMotorVars.push_back(makeSyncVar("speed_command", "deg/s", rightMotorSpeedCmd, VarAccess::READWRITE, false));
    gyemsRightMotorVars.push_back(makeSyncVar("position_command", "deg", rightMotorAngleCmd, VarAccess::READWRITE, false));
    gyemsRightMotorVars.push_back(makeSyncVar("torque_command", "Nm", rightMotorTorqueCmd, VarAccess::READWRITE, true));

    gyemsLeftMotorVars.push_back(makeSyncVar("multiturn_pos", "deg", leftMotorAngle, VarAccess::READ, true));
    gyemsLeftMotorVars.push_back(makeSyncVar("position", "deg", leftAngle, VarAccess::READ, true));
    gyemsLeftMotorVars.push_back(makeSyncVar("speed", "deg/s", leftSpeed, VarAccess::READ, true));
    gyemsLeftMotorVars.push_back(makeSyncVar<float>("offset", "deg",
                                                   nullptr,
                                        [=](float offsetAng)
                                        {
                                            leftMotor.setAngleOffset(offsetAng);
                                        } , false));
    gyemsLeftMotorVars.push_back(makeSyncVar("control_mode", "1:pos, 2:spd", leftMotorCtrlMode, VarAccess::READWRITE, false));
    gyemsLeftMotorVars.push_back(makeSyncVar("speed_command", "deg/s", leftMotorSpeedCmd, VarAccess::READWRITE, false));
    gyemsLeftMotorVars.push_back(makeSyncVar("position_command", "deg", leftMotorAngleCmd, VarAccess::READWRITE, false));
    gyemsLeftMotorVars.push_back(makeSyncVar("torque_command", "Nm", leftMotorTorqueCmd, VarAccess::READWRITE, true));

    syncVars.add("right_motor/", gyemsRightMotorVars);
    syncVars.add("left_motor/", gyemsLeftMotorVars);

    syncVars.push_back(makeSyncVar("torque_test?", "[0/1]", startTorqueTest, VarAccess::READWRITE, false));
    syncVars.push_back(makeSyncVar("torqTest_magnitude", "[Nm]", torqueMagnitude, VarAccess::READWRITE, true));
    syncVars.push_back(makeSyncVar("torqTest_frequency", "[Hz]", torqueFrequency, VarAccess::READWRITE, true));
    syncVars.push_back(makeSyncVar("sine time", "[s]", sineTimer, VarAccess::READ, true));

    // Creating the thread for handling the CAN communication with the motors
    canThread = new thread(&eWalkController::handleCanCommunication, this);

    // Setting the thread priority for CAN communication
    struct sched_param sp;
    sp.sched_priority = 2;
    pthread_setschedparam(canThread->native_handle(), SCHED_RR, &sp);
}


eWalkController::~eWalkController()
{
    rightMotor.setSpeed(0);
    //rightMotor.update(MAIN_LOOP_PERIOD);
    leftMotor.setSpeed(0);
    //leftMotor.update(MAIN_LOOP_PERIOD);

    // Stop the CAN communication thread
    stopCanThread = true;
    canThread->join();
    delete canThread;
}

void eWalkController::update(float dt)
{
    if(startTorqueTest)                          //sinusoidal torque test
    {
        if(sineTimer >= 1/torqueFrequency)      //keep 2*PI*omega*t in the range [0,2*PI) to avoid overflow
            sineTimer = 0;

        rightMotorTorqueCmd = torqueMagnitude*sin(2*PI*torqueFrequency*sineTimer);
        leftMotorTorqueCmd = -torqueMagnitude*sin(2*PI*torqueFrequency*sineTimer + PI); // two motors are facing each other

        rightMotor.setTorque(rightMotorTorqueCmd);
        leftMotor.setTorque(leftMotorTorqueCmd);

        sineTimer += dt;
    }
    else
    {
        sineTimer = 0;
        switch(rightMotorCtrlMode)
        {
        case 0:                             //braking mode
            rightMotor.setSpeed(0);
            break;

        case 1:                             //position control
            rightMotor.setPosition(rightMotorAngleCmd);
            break;

        case 2:                             //speed control
            rightMotor.setSpeed(rightMotorSpeedCmd);
            break;

        case 3:                             //torque control
            rightMotor.setTorque(rightMotorTorqueCmd);
            break;

        default:
            break;
        }

        switch(leftMotorCtrlMode)
        {
        case 0:                             //braking mode
            leftMotor.setSpeed(0);
            break;

        case 1:                             //position control
            leftMotor.setPosition(leftMotorAngleCmd);
            break;

        case 2:                             //speed control
            leftMotor.setSpeed(leftMotorSpeedCmd);
            break;

        case 3:                             //torque control
            leftMotor.setTorque(leftMotorTorqueCmd);
            break;

        default:
            break;
        }
    }

    /* Moving to new thread function
    //handle the CAN communication
    rightMotor.update(dt);
    leftMotor.update(dt);
    //can.Update();
    */

    //Update motor variables
    rightMotorAngle = rightMotor.getPosition();
    leftMotorAngle = leftMotor.getPosition();
    rightAngle = rightMotor.getShaftAngle();
    leftAngle = leftMotor.getShaftAngle();
    rightSpeed = rightMotor.getSpeed();
    leftSpeed = leftMotor.getSpeed();
}

void eWalkController::handleCanCommunication()
{
    float dt = USEC_TO_SEC(CAN_UPDATE_PERIOD);
    system_clock::time_point nextExecTime = high_resolution_clock::now();

    stopCanThread = false;

    while(!stopCanThread)
    {
        //
        auto now = high_resolution_clock::now();

        // Acquire the load cells, and compute the haptic feedback.
        //handle the CAN communication
        rightMotor.update(dt);
        leftMotor.update(dt);
        //can.Update();

        //
        nextExecTime = now + microseconds(CAN_UPDATE_PERIOD);
        std::this_thread::sleep_until(nextExecTime);
    }
}
