#ifndef EWALKCONTROLLER_H
#define EWALKCONTROLLER_H

#include "../controller.h"
#include "ewalkdefinitions.h"
#include "../../drivers/canbus.h"
#include "../../drivers/gyems.h"

#define MAIN_LOOP_PERIOD 0.002f ///< Main loop period [s].

class eWalkController : public Controller
{
public:
    eWalkController(PeripheralsSet peripherals);
    ~eWalkController();

    void update(float dt) override;

private:
    CanBus can;
    Gyems rightMotor, leftMotor;

    void handleCanCommunication();
    std::thread *canThread;
    volatile bool stopCanThread;

    bool startTorqueTest;

    float rightMotorAngle;
    float rightAngle;
    float rightSpeed;           ///<Right hip joint speed [deg/s]
    int rightMotorCtrlMode;
    float rightMotorSpeedCmd;
    float rightMotorAngleCmd;
    float rightMotorTorqueCmd;


    float leftMotorAngle;
    float leftAngle;
    float leftSpeed;            ///<Left hip joint speed [deg/s]
    int leftMotorCtrlMode;
    float leftMotorSpeedCmd;
    float leftMotorAngleCmd;
    float leftMotorTorqueCmd;

    //sinusoidal torque test vars
    float torqueMagnitude;
    float torqueFrequency;
    float sineTimer;
};

typedef eWalkController SelectedController;

#endif // EWALKCONTROLLER_H
