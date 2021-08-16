#ifndef EWALKDEFINITIONS_H
#define EWALKDEFINITIONS_H

/* Constants and variables common to all eWalk controllers */

#define CAN_UPDATE_PERIOD 2000 //[us]

/******** HARDWARE CONSTANTS *************/
//Angle offsets (these values are added to the raw angles read from the
//motors and subtracted from the commanded values sent to the motors)
#define RIGHT_ANGLE_OFFSET 75.0f    // [deg]
#define LEFT_ANGLE_OFFSET 80.0f    // [deg]
//Direction signs for torque and angle, such that flexion is positive.
//For both motors, when looking from the shaft side, CW is positive. but since
//the motors are mirrored, the right motor is positive in the direction of
//flexion, while the left motor is positive in extension.
#define RIGHT_MOTOR_SIGN 1
#define LEFT_MOTOR_SIGN -1

#endif // EWALKDEFINITIONS_H
