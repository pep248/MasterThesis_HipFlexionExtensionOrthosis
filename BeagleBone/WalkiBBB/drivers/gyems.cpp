#include "gyems.h"

#include "../communication.h"
#include "../lib/debugstream.h"
#include "../lib/utils.h"
#include "../../MotorBoard/src/lib/crc32_table.dat"
#include <vector>

#include <linux/can.h>
#include <linux/can/raw.h>

using namespace std;
using namespace chrono;

// Motor parameters
#define TORQUE_CONST            3.30f //[Nm/A]
#define CURRENT_CMD_CONST       2000.f/32.f     // [(torque command LSB) / A] for mapping the current command to the actual current
#define REDUCTION_RATIO         6.0f
#define ENCODER_BITS            65535.0f        // 16-bit encoder, 2^16-1 = 65535

// Calibration values for converting commanded torque to actual torque
// experimentally determined (Ali)
#define TORQUE_CALIB_GAIN 2.0892f
#define TORQUE_CALIB_OFFSET 0.4474f

// Can protocol
#define CAN_ID                  0x140

#define READ_PID_DATA           0x30
#define WRITE_PID_TO_RAM        0x31
#define WRITE_PID_TO_ROM        0x32
#define READ_ACC                0x33
#define WRITE_ACC_TO_RAM        0x34
#define READ_ENCODER            0x90
#define WRITE_ENCODER_OFFSET    0x91
#define WRITE_CURRENT_POS_AS_ZERO 0x19
#define READ_MULTITURN_ANGLE    0x92
#define READ_SINGLECIRCLE_ANGLE 0x94
#define READ_STATUS_1           0x9A
#define CLEAR_ERROR_FLAG        0x9B
#define READ_STATUS_2           0x9C
#define READ_STATUS_3           0x9D
#define MOTOR_OFF               0x80
#define MOTOR_STOP              0x81
#define MOTOR_RUNNING           0x88
#define TORQUE_COMMAND          0xA1
#define SPEED_COMMAND           0xA2
#define POSITION_COMMAND_1      0xA3
#define POSITION_COMMAND_2      0xA4
#define POSITION_COMMAND_3      0xA5
#define POSITION_COMMAND_4      0xA6

enum CtrlMode
{
    POSITION_CTRL,
    CURRENT_CTRL,
    SPEED_CTRL
};

/**
 * @brief Constructor.
 * @param canbus the CanBus object to use for communicating with the motor.
 * @param id CAN ID of the motor, set by the switches on the case.
 * @param direction Direction sign indicator, use -1 to invert the positive
 * rotation direction (both for angles and torques).
 * @param offsetAng Software offset angle to be added to the angle read from
 * the motor and subtracted from the commanded angles sent to the motor [deg].
 */
Gyems::Gyems(CanBus* canbus, uint8_t id, int direction, float offsetAngle):
    can(canbus),
    motorId(id),
    directionSign(direction),
    softwareOffsetAngle(offsetAngle)
{
    //initialize motor parameters to -1.
    temperature = -1;
    current = -1.0f;
    speed = -1.0f;
    inputShaftAngle = -1.0f;
    multiTurnAngle = -1.0f;

    controlMode = -1;
}

Gyems::~Gyems()
{
    sendPacket(PC_MESSAGE_STOP_STREAMING);
}

/**
 * @brief Updates the status of the motorboard by interpreting the RX data.
 */
void Gyems::update(float dt)
{
    processRxBytes();

    switch(controlMode)
    {
        case SPEED_CTRL:
            sendPacket(SPEED_COMMAND);
            break;
        case POSITION_CTRL:
            sendPacket(POSITION_COMMAND_1);
            break;
        case CURRENT_CTRL:
            sendPacket(TORQUE_COMMAND);
            break;
        default:
            break;
    }
    sendPacket(READ_MULTITURN_ANGLE);
    sendPacket(READ_STATUS_2);          //Mainly to read the speed and current

    //printf("speed[deg/sec] %f - current %f - temp %d\n", speed, current, temperature);
    //printf("encoder pos %f - multu %f \n", encoder_position, multiTurnAngle);
}

/**
 * @brief Set the target current.
 * @param current: the desired current in A in range -32A->32A
 */
void Gyems::setCurrent(float current)
{
    targetCurrent = directionSign * current;
    controlMode = CURRENT_CTRL;
}

/**
 * @brief Set the torque.
 * @param torque: the desired torque in N.m
 */
void Gyems::setTorque(float torque)
{
    // Map the desired torque to the experimentally determined command values
    // so as to get an actual torque close to the desired one
    float commanded_torque = TORQUE_CALIB_GAIN * torque + TORQUE_CALIB_OFFSET;
    commanded_torque *= directionSign;
    targetCurrent = commanded_torque / TORQUE_CONST;
    controlMode = CURRENT_CTRL;
}

/**
 * @brief Set the target positon.
 * @param pos: the desired angle (output shaft) in deg
 * @note the target position corresponds to the multiturn angle before reducer (in deg)
 */
void Gyems::setPosition(float pos)
{
    pos = (pos - softwareOffsetAngle) * directionSign;      //implementing software offset and direction
    targetPosition = int32_t (pos * REDUCTION_RATIO);
    controlMode = POSITION_CTRL;
}

/**
 * @brief Set the target speed.
 * @param speed in deg/sec
 */
void Gyems::setSpeed(int32_t speed)
{
    targetSpeed = speed * REDUCTION_RATIO * directionSign;
    controlMode = SPEED_CTRL;
}

/**
  @brief Set the encoder angle offset (input shaft angle, 0-360).
  @param offset offset in degrees
  @note This also affects the multiTurnAngle
  */
void Gyems::setAngleOffset(float offset)
{
    inputShaftEncoderOffset = (uint16_t)(offset * ENCODER_BITS/360.0f);      //convert degrees to pulse counts
    sendPacket(WRITE_ENCODER_OFFSET);
    debug<<"encoder offset changed to "<<inputShaftEncoderOffset<<endl;
}

/**
 * @brief get the actuator's multiturn position in deg
 */
double Gyems::getPosition() const
{
    return (multiTurnAngle*directionSign)+softwareOffsetAngle;
}

/**
  * @brief Get the single-turn input shaft encoder angle [0-360].
  * @note Given the 6:1 reduction ratio, this angle goes from 0-360 deg
  * with every 60 degree rotation of the output shaft.
  */
float Gyems::getShaftAngle() const
{
    return inputShaftAngle;
}

/**
 * @brief Get the actuator's speed in deg/sec.
 */
float Gyems::getSpeed() const
{
    return speed*directionSign;
}

/**
 * @brief Get the actuator's current in A.
 */
float Gyems::getCurrent() const
{
    return current*directionSign;
}

/**
 * @brief Get the actuator's torque in N.m, calculated based on the reported current.
 */
float Gyems::getTorque() const
{
    float gyemsTorque = current * directionSign * TORQUE_CONST;
    float experimentalTorque = (gyemsTorque - TORQUE_CALIB_OFFSET) / TORQUE_CALIB_GAIN;
    return experimentalTorque;
}

/**
 * @brief Get the actuator's temperature in degC.
 */
int Gyems::getTemperature() const
{
    return temperature;
}

/**
 * @brief Sends a message packet to the motor.
 * @param messageType type of the message.
 */
void Gyems::sendPacket(uint8_t messageType)
{
    struct can_frame frame;
    frame.can_id = CAN_ID + motorId;
    frame.can_dlc = 8;
    frame.data[0]=messageType;
    frame.data[1]=0x0;
    frame.data[2]=0x0;
    frame.data[3]=0x0;
    frame.data[4]=0x0;
    frame.data[5]=0x0;
    frame.data[6]=0x0;
    frame.data[7]=0x0;

    //For the commands requiring a message body in addition to the header...
    switch(messageType)
    {
        case TORQUE_COMMAND:
        {
            int16_t can_current = targetCurrent * CURRENT_CMD_CONST; //mapping +/-32A to +/-2000 range
            frame.data[4]=(can_current & 0b11111111);
            frame.data[5]=(can_current & 0b1111111100000000)>>8;
            break;
        }
        case SPEED_COMMAND:
        {
            int32_t can_speed = targetSpeed * 100.0f; //in 0.01deg/sec
            frame.data[4]=(can_speed & 0xFF);
            frame.data[5]=(can_speed & 0xFF00)>>8;
            frame.data[6]=(can_speed & 0xFF0000)>>16;
            frame.data[7]=(can_speed & 0xFF000000)>>24;
            break;
        }
        case POSITION_COMMAND_1:
        {
            int32_t can_position = targetPosition  * 100.0f; // in 0.01deg
            frame.data[4]=(can_position & 0xFF);
            frame.data[5]=(can_position & 0xFF00)>>8;
            frame.data[6]=(can_position & 0xFF0000)>>16;
            frame.data[7]=(can_position & 0xFF000000)>>24;
            break;
        }
        case WRITE_ENCODER_OFFSET:
            frame.data[6]=*(uint8_t*)(&inputShaftEncoderOffset);
            frame.data[7]=*((uint8_t*)(&inputShaftEncoderOffset)+1);
            break;
        default:
            break;
    }

    can->PushMessage(frame);
    can->Update();
}

/**
 * @brief Interprets all the received bytes, sent by the motorboard.
 * @return true if a valid message has been decoded, false otherwise.
 */
bool Gyems::processRxBytes()
{
    bool validMessageReceived = false;
    std::vector<can_frame> rxFrames;
    int r = can->PullMessage(CAN_ID + motorId, rxFrames);
    if(r && rxFrames.size()>0)
    {
        for(auto i=0u; i < rxFrames.size(); i++)
        {
            can_frame rxFrame = rxFrames[i];
            switch(rxFrame.data[0])
            {
            case TORQUE_COMMAND:
            case SPEED_COMMAND:
            case POSITION_COMMAND_1:
            case POSITION_COMMAND_2:
            case POSITION_COMMAND_3:
            case POSITION_COMMAND_4:
            case READ_STATUS_2:
            {
                temperature = rxFrame.data[1]; //[degC]
                current = float(int16_t(rxFrame.data[2] + (rxFrame.data[3]<<8)))*33.0/2048.0; //[A] range -2048,2048 to -33,33A
                inputSpeed = int16_t(rxFrame.data[4] + (rxFrame.data[5]<<8)); //[deg/sec]
                speed = inputSpeed / REDUCTION_RATIO;
                inputShaftAngle = float(uint16_t(rxFrame.data[6] + (rxFrame.data[7]<<8)))*360.0/ENCODER_BITS;//*360.0/16383.0; //[deg] 16bit encoder range 0-16383
                validMessageReceived = true;
                break;
            }
            case READ_MULTITURN_ANGLE:
            {
                //The multi-turn angle is a signed 7-byte int, which needs to be written into a signed 8-byte int64
                //so in the case of negative numbers, the highest byte (indicating the sign) needs to be set manually.
                int64_t m_temp = (int64_t)rxFrame.data[1] + ((int64_t)rxFrame.data[2]<<8) + ((int64_t)rxFrame.data[3]<<16) +
                        ((int64_t)rxFrame.data[4]<<24) + ((int64_t)rxFrame.data[5]<<32) + ((int64_t)rxFrame.data[6]<<40) +
                        ((int64_t)rxFrame.data[7]<<48);
                if (rxFrame.data[7] & (uint8_t)0x80)        // if the sign bit is 1 (negative number)
                    m_temp |= (int64_t)0xFF<<56;

                multiTurnAngle = double(m_temp / REDUCTION_RATIO / 100.0);
                validMessageReceived = true;
                break;
            }
            default:
                break;
            }
        }
    }
    return validMessageReceived;
}
