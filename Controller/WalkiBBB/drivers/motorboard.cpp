#include "motorboard.h"

#include "../communication.h"
#include "../lib/debugstream.h"
#include "../lib/utils.h"
#include "../../MotorBoard/src/lib/crc32_table.dat"

using namespace std;
using namespace chrono;

#define TX_BUFFER_SIZE 1000
#define UART_COMM_MAX_TRIALS 10 ///< Maximum number of trials to establish the UART communication.
#define PING_CHECK_INTERVAL 0.001f ///< Time interval between each motorboard update, to check the answer to the ping [s].
#define PING_TIMEOUT_TIME milliseconds(100) ///< Maximum wait time for ping answer [ms].
#define RESET_MAX_DURATION milliseconds(1000) ///< Maximum time allowed for the motorboard to resume streaming status packets, after reset.
#define UART_BAUDRATE B3000000 ///< Speed of the UART communication between the BeagleBone and the motorboard. Note that a value of B1500000 will actually give 1.886 Mbaud!
#define MAX_TIME_WITHOUT_RX 0.100f ///< Maximum acceptable time without receiving a valid message from the motorboard [s].
#define TEMPERATURE_WARNING 95 // [celsius].

#define SIMULATED_ENCODERS_FILENAME "" // Empty to disable.

const vector<string> FAULT_MESSAGES =
{
    "Critical battery voltage.",
    "Board current too high.",
    "Motor A current too high.",
    "Motor B current too high.",
    "Inconsistent position of motor A (sensors conflict).",
    "Inconsistent position of motor B (sensors conflict).",
    "Communication timeout (no messages from the mainboard received).",
    "Watchdog timeout (real-time issue).",
    "H-bridge A over-temperature.",
    "H-bridge A fault (12V undervoltage).",
    "H-bridge B over-temperature.",
    "H-bridge B fault (12V undervoltage).",
    "CPU overloaded.",
    "Joint position A out of limits.",
    "Joint position B out of limits.",
    "Emergency stop button pressed.",
    "Encoder A missed too many steps.",
    "Encoder B missed too many steps.",
    "Joint A position error too large.",
    "Joint B position error too large.",
    "Over-temperature A.",
    "Over-temperature B.",
    "Over-temperature C.",
    "Load cell A fault.",
    "Load cell B fault.",
    "Actuator sensorsboard A fault.",
    "Actuator sensorsboard B fault.",
};

/**
 * @brief Constructor.
 * @param serialPort the serial port to use for communicating with the
 * motorboard.
 * @param speedAccelFilteringTau cut-off period of the low-pass filters of the
 * speeds and accelerations [s].
 * @param doPingTest true to perform a ping test to check the status of the
 * board, false if the board link is assumed to be good.
 */
MotorBoard::MotorBoard(UartPort serialPort, float speedAccelFilteringTau,
                       bool doPingTest) :
#if !SIMULATED_ENCODERS
    uart(serialPort, UART_BAUDRATE),
#endif
    angles { 0.0f, 0.0f, 0.0f, 0.0f },
    speeds { {speedAccelFilteringTau}, {speedAccelFilteringTau},
             {speedAccelFilteringTau}, {speedAccelFilteringTau} },
    accelerations { {speedAccelFilteringTau}, {speedAccelFilteringTau},
                    {speedAccelFilteringTau}, {speedAccelFilteringTau} }
{
    //
    txBuffer.reserve(TX_BUFFER_SIZE);
    ignoreCommErrors = false;
    gotFault = false;
    rxCurrentMessageType = STM_MESSAGE_COUNT;
    rxBytesCount = 0;
    timeWithoutRx = 0.0f;
    temperatureA = 0.0f;
    temperatureB = 0.0f;
    temperatureC = 0.0f;
    resetTime = steady_clock::now() - RESET_MAX_DURATION;
    motorboardTimestamp = 0;
    previousMotorboardTimestamp = 0;

    // Open the serial communication port.
#if SIMULATED_ENCODERS
    unused(serialPort);
    unused(doPingTest);

    if(strlen(SIMULATED_ENCODERS_FILENAME) > 0)
    {
        // Get simulated encoders values from a text file.
        encodersFile.open(SIMULATED_ENCODERS_FILENAME);

        if(encodersFile.good())
            state = PeripheralState::ACTIVE;
        else
            state = PeripheralState::FAULT;
    }
    else
        state = PeripheralState::ACTIVE;
#else
    if(uart.getState() != PeripheralState::ACTIVE)
    {
        state = PeripheralState::FAULT;
        return;
    }

    // Momentary stop the streaming when the UART is re-initialized. This avoids
    // read errors.
    sendPacket(PC_MESSAGE_STOP_STREAMING);
    this_thread::sleep_for(milliseconds(10));
    uart.reset();
    this_thread::sleep_for(milliseconds(10));
    sendPacket(PC_MESSAGE_START_STREAMING);

    // Check that the motorboard is responding.
    if(doPingTest)
    {
        ignoreCommErrors = true;

        for(int nTrials=0; true ; nTrials++)
        {
            if(ping())
            {
                debug << "The motorboard on " << serialPort
                      << " has been detected." << endl;
                state = PeripheralState::ACTIVE;
                break;
            }

            if(nTrials < UART_COMM_MAX_TRIALS)
            {
                uart.reset();
                this_thread::sleep_for(milliseconds(100));
            }
            else
            {
                debug << "The motorboard on " << serialPort
                      << " is not responding." << endl;
                state = PeripheralState::FAULT;
                break;
            }
        }

        ignoreCommErrors = false;
    }
    else
        state = PeripheralState::ACTIVE;
#endif

    // Create the SyncVars.
    /*syncVars.push_back(makeSyncVar("timestamp", "us", motorboardTimestamp,
                                   VarAccess::READ, true));*/
    syncVars.push_back(makeSyncVar<float>("joint_angle_a", "deg",
                                          [=]() { return angles[JOINT_A]; },
                                          [=](float angle) { setEncoderPosition(JointID::A, angle); },
                                          true));
    syncVars.push_back(makeSyncVar<float>("joint_angle_b", "deg",
                                          [=]() { return angles[JOINT_B]; },
                                          [=](float angle) { setEncoderPosition(JointID::B, angle); },
                                          true));
    syncVars.push_back(makeSyncVar("joint_speed_a", "deg/s", speeds[JOINT_A].getRaw(),
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("joint_speed_b", "deg/s", speeds[JOINT_B].getRaw(),
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("joint_accel_a", "deg/s^2", accelerations[JOINT_A].getRaw(),
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("joint_accel_b", "deg/s^2", accelerations[JOINT_B].getRaw(),
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("target_position_a", "deg", targetAngleA,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("target_position_b", "deg", targetAngleB,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("motor_current_a", "A", currentA,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("motor_current_b", "A", currentB,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("motor_target_current_a", "A",
                                   currentTargetA, VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("motor_target_current_b", "A",
                                   currentTargetB, VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("motor_pot_a", "deg",
                                   potentiometerA, VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("motor_pot_b", "deg",
                                   potentiometerB, VarAccess::READ, false));
    syncVars.push_back(makeSyncVar<float>("soft_reset", "", nullptr,
                                   [&](float v){if(v != 0.0f) resetBoard();},
                                   false));
    syncVars.push_back(makeSyncVar<bool>("arm_bridges", "", nullptr,
                                   [&](bool on) {if(on)
                                                    armBridges();
                                                 else
                                                    disarmBridges();
                                                 }, false));

    syncVars.push_back(makeSyncVar("board_voltage", "V", boardVoltage,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("board_current", "A", boardCurrent,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("motorboard_timestamp", "us",
                                   motorboardTimestamp, VarAccess::READ, false));

    syncVars.push_back(makeSyncVar("temperature_a", "celsius", temperatureA,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("temperature_b", "celsius", temperatureB,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("temperature_c", "celsius", temperatureC,
                                   VarAccess::READ, false));

    syncVars.push_back(makeSyncVar<float>("fan_a_speed", "", nullptr,
                                          [=](uint32_t speed){setVar(VAR_FAN_A_SPEED, speed);}, false));
    syncVars.push_back(makeSyncVar<float>("fan_b_speed", "", nullptr,
                                          [=](uint32_t speed){setVar(VAR_FAN_B_SPEED, speed);}, false));
    syncVars.push_back(makeSyncVar<float>("fan_c_speed", "", nullptr,
                                          [=](uint32_t speed){setVar(VAR_FAN_C_SPEED, speed);}, false));
}

MotorBoard::~MotorBoard()
{
    sendPacket(PC_MESSAGE_STOP_STREAMING);

    // Disarm the board.
    //disarmBridges();
}

/**
 * @brief Updates the status of the motorboard by interpreting the RX data.
 */
void MotorBoard::update(float dt)
{
#if SIMULATED_ENCODERS
    if(encodersFile.is_open())
    {
        // Extract the encoders positions from the file.
        if(encodersFile.eof())
        {
            encodersFile.close();
            encodersFile.open(SIMULATED_ENCODERS_FILENAME);
            cout << "Rewinding dummy encoders positions..." << endl;
        }

        string line;
        getline(encodersFile, line);

        istringstream iss(line);
        iss >> angles[JOINT_A];
        iss >> angles[JOINT_B];
    }
#else
    if(state != PeripheralState::ACTIVE)
        return;

    // Process the bytes received through the serial link.
    std::vector<uint8_t> receivedData = uart.read();
    bool validMessageReceived = false;
    while(!receivedData.empty())
    {
        validMessageReceived |= processRxBytes(receivedData);
        receivedData = uart.read();
    }

    // Check that the motorboard is still sending data periodically.
    if(validMessageReceived)
        timeWithoutRx = 0.0f;
    else if(steady_clock::now() - resetTime > RESET_MAX_DURATION)
    {
        if(!ignoreCommErrors && timeWithoutRx > MAX_TIME_WITHOUT_RX)
        {
            throw runtime_error("Motorboard " + to_string((int)uart.getPort())
                                + ": no valid message received for too long.");
        }

        timeWithoutRx += dt;
    }
#endif

    // Compute the speeds and accelerations.
    float motDt = USEC_TO_SEC(motorboardTimestamp
                              - previousMotorboardTimestamp);
    previousMotorboardTimestamp = motorboardTimestamp;

    for(int i=0; i<4; i++)
    {
        speeds[i].update(angles[i], motDt);
        accelerations[i].update(speeds[i].getFiltered(), motDt);
    }
}

/**
 * @brief Soft-resets the board.
 */
void MotorBoard::resetBoard()
{
    sendPacket(PC_MESSAGE_RESET);

    resetTime = steady_clock::now();
}

/**
 * @brief Sets the timestamp counter of the motorboard.
 * Set the timestamp counter of the motorboard to match the BeagleBone time.
 * It will be actually set to the number of microseconds since epoch.
 */
void MotorBoard::setTime()
{
    auto now = high_resolution_clock::now().time_since_epoch();
    uint64_t timeUs = duration_cast<microseconds>(now).count();

    vector<uint8_t> data((uint8_t*)&timeUs, ((uint8_t*)&timeUs) + sizeof(timeUs));

    sendPacket(PC_MESSAGE_SET_TIME, data);
}

/**
 * @brief Sets the motorboard LED state.
 * @param enable the new state of the LED.
 * @note On the motorboard V2, there is no LED, so this function will have no
 * effect.
 */
void MotorBoard::setLed(bool enable)
{
    vector<uint8_t> data;
    data.push_back(enable ? 1 : 0);
    sendPacket(PC_MESSAGE_SET_LED, data);
}

/**
 * @brief Arms the motorboard H-bridges.
 */
void MotorBoard::armBridges()
{
    sendPacket(PC_MESSAGE_ARM_BRIDGES);
}

/**
 * @brief Disarms the motorboard H-bridges.
 */
void MotorBoard::disarmBridges()
{
    sendPacket(PC_MESSAGE_DISARM_BRIDGES);
}

/**
 * @brief Stops powering the selected motor, so it can spin freely.
 * @param id the motor or joint identifier.
 */
void MotorBoard::coast(MotorJointID id)
{
    vector<uint8_t> data;
    data.push_back(id);

    sendPacket(PC_MESSAGE_COAST, data);
}

/**
 * @brief Sets the voltage to apply to the specified motor.
 * Sets the specified motor regulator in voltage mode, and applies the given
 * voltage.
 * @param id the identifier of the motor or the joint. If a motor ID is used,
 * an unloaded motor will spin clockwise if the voltage is positive. If a joint
 * ID is used an unloaded motor will spin in the positive direction if the
 * voltage is positive. These rotation directions are described in the Walki
 * software user manual, in the "Geometric and anatomic conventions" section.
 * @param voltage voltage to apply [V].
 * @note The maximum voltage that can be applied is limited by the motorboard
 * power supply voltage.
 */
void MotorBoard::setVoltage(MotorJointID id, float voltage)
{
    vector<uint8_t> data(sizeof(uint8_t) + sizeof(voltage));
    data[0] = id;
    memcpy(&data[1], &voltage, sizeof(voltage));

    sendPacket(PC_MESSAGE_SET_VOLTAGE, data);
}

/**
 * @brief Sets the torque to be applied by the specified motor.
 * Sets the specified motor regulator in current mode, and applies the given
 * torque.
 * @param id the identifier of the motor or the joint. If a motor ID is used,
 * an unloaded motor will spin clockwise if the torque is positive. If a joint
 * ID is used an unloaded motor will spin in the positive direction if the
 * torque is positive. These rotation directions are described in the Walki
 * software user manual, in the "Geometric and anatomic conventions" section.
 * @param torque torque to apply [N.m].
 * @note The maximum torque that can be applied is limited the motor nominal
 * current and the power supply.
 */
void MotorBoard::setTorque(MotorJointID id, float torque)
{
    vector<uint8_t> data(sizeof(uint8_t) + sizeof(torque));
    data[0] = id;
    memcpy(&data[1], &torque, sizeof(torque));

    sendPacket(PC_MESSAGE_SET_TORQUE, data);
}

/**
 * @brief Sets the target speed of the specified motor.
 * @param id identifier of the desired motor.
 * @param speed target speed [deg/s].
 */
void MotorBoard::setSpeed(MotorJointID id, float speed)
{
    vector<uint8_t> data(sizeof(uint8_t) + sizeof(speed));
    data[0] = id;
    memcpy(&data[1], &speed, sizeof(speed));

    sendPacket(PC_MESSAGE_SET_SPEED, data);
}

/**
 * @brief Set the target position of the specified motor.
 * Sets the specified motor regulator in position mode, with the specified
 * target position. It is possible to specify a time for reaching the target
 * point, in order to avoid a violent movement, in this case a smooth polynomial
 * trajectory will be generated by the motorboard.
 * @param id the identifier of the motor or the joint. The coordinate system
 * will be different, depending on if the given MotorJointID corresponds to a
 * motor or a joint. These conventions are described in the Walki software user
 * manual, in the "Geometric and anatomic conventions" section.
 * @param position target position [deg].
 * @param duration duration of the trajectory to reach the target point [s]. If
 * a duration of 0 is given, the motorboard will try to reach the target point
 * as fast as possible.
 */
void MotorBoard::setPosition(MotorJointID id, float position, float duration)
{
    vector<uint8_t> data(sizeof(uint8_t) + sizeof(position) + sizeof(duration));
    data[0] = id;
    memcpy(&data[1], &position, sizeof(position));
    memcpy(&data[5], &duration, sizeof(duration));

#if SIMULATED_ENCODERS
    angles[id] = position;
#else
    sendPacket(PC_MESSAGE_SET_POSITION, data);
#endif
}

/**
 * @brief Sets a simple sine waves sum trajectory to the specified motor.
 * Sets the specified motor regulator in position mode, with the specified
 * target trajectory. The three defined sine waves will be summed to obtain the
 * final trajectory.
 * The amplitude and speed modulating factors are set to zero, so to get a
 * movement it is necessary to call setTrajectoryAmplitude() and
 * setTrajectorySpeed().
 * @param id the identifier of the motor or the joint. The coordinate system
 * will be different, depending on if the given MotorJointID corresponds to a
 * motor or a joint. These conventions are described in the Walki software user
 * manual, in the "Geometric and anatomic conventions" section.
 * @param period duration of a cycle [s].
 * @param offset average position [deg].
 * @param a1 first sine wave amplitude [deg].
 * @param a2 second sine wave amplitude [deg].
 * @param a3 third sine wave amplitude [deg].
 * @param f1 first sine wave frequency multiplier []. Final first wave frequency
 * will be (1 / period) * f1.
 * @param f2 second sine wave frequency multiplier []. Final second wave
 * frequency will be (1 / period) * f2.
 * @param f3 thired sine wave frequency multiplier []. Final third wave
 * frequency will be (1 / period) * f3.
 */
void MotorBoard::setSineTrajectory(MotorJointID id, float period, float offset,
                                   float a1, float a2, float a3,
                                   float f1, float f2, float f3)
{
    ByteBuffer data;
    data << (uint8_t)id << period << offset << a1 << f1 << a2 << f2 << a3 << f3;

    sendPacket(PC_MESSAGE_SET_SINE_TRAJ, data);
}

/**
 * @brief Sets a Fourier trajectory to the specified motor.
 * Sets the specified motor regulator in position mode, with the specified
 * target trajectory, defined by Fourier coefficients.
 * @param id the identifier of the motor or the joint. The coordinate system
 * will be different, depending on if the given MotorJointID corresponds to a
 * motor or a joint. These conventions are described in the Walki software user
 * manual, in the "Geometric and anatomic conventions" section.
 * @param period duration of a cycle [s].
 * @param fourierCoefs list of Fourier coefficients. The maximum size of this
 * list is TRJ_FOURIER_MAX_ORDER + 1.
 */
void MotorBoard::setFourierTrajectory(MotorJointID id, float period,
                                      vector<SineDef> fourierCoefs)
{
    if(fourierCoefs.empty())
        return;

    if(fourierCoefs.size() > TRJ_FOURIER_MAX_ORDER + 1)
        throw runtime_error("MotorBoard::setFourierTrajectory: Fourier order is"
                            "too large.");

    float a0 = fourierCoefs[0].a;

    ByteBuffer data;
    data << (uint8_t)id << (uint8_t)fourierCoefs.size() << period << a0;

    for(unsigned int i=1; i<fourierCoefs.size(); i++)
    {
        float a = fourierCoefs[i].a;
        float b = fourierCoefs[i].b;

        data << a << b;
    }

    sendPacket(PC_MESSAGE_SET_FOURIER_TRAJ, data);
}

/**
 * @brief Sets the cyclic trajectory amplitude factor.
 * Sets the cyclic trajectory amplitude factor, that modulates the current
 * trajectory. This has no effect if the current regulation is voltage mode,
 * current mode, or fixed-target trajectory mode.
 * @param id the motor/joint identifier. Using a joint or a motor ID is the
 * same.
 * @param amplitude the new trajectory amplitude factor [0.0-1.0].
 */
void MotorBoard::setTrajectoryAmplitude(MotorJointID id, float amplitude)
{
    ByteBuffer data;
    data << (uint8_t)id << amplitude;

    sendPacket(PC_MESSAGE_SET_TRAJ_AMPLITUDE, data);
}

/**
 * @brief Sets the cyclic trajectory speed factor.
 * Sets the cyclic trajectory speed factor, that modulates the frequency of the
 * trajectory. This has no effect if the current regulation is voltage mode,
 * current mode, or fixed-target trajectory mode.
 * @param id the motor/joint identifier. Using a joint or a motor ID is the
 * same.
 * @param speed the new trajectory speed factor. It can also be negative to run
 * the trajectory in reverse.
 */
void MotorBoard::setTrajectorySpeed(MotorJointID id, float speed)
{
    ByteBuffer data;
    data << (uint8_t)id << speed;

    sendPacket(PC_MESSAGE_SET_TRAJ_SPEED, data);
}

/**
 * @brief Directly sets the current progress of the cyclic trajectory.
 * Directly sets the progress of the current cyclic trajectory, which allows to
 * "jump" to a specific stage of the walking gait. This has no effect if the
 * current regulation is voltage mode, current mode, or fixed-target trajectory
 * mode.
 * @param id the motor/joint identifier. Using a joint or a motor ID is the
 * same.
 * @param progress the new progress to jump to.
 */
void MotorBoard::setTrajectoryProgress(MotorJointID id, float progress)
{
    ByteBuffer data;
    data << (uint8_t)id << progress;

    sendPacket(PC_MESSAGE_SET_TRAJ_PROGRESS, data);
}

/**
 * @brief Gets the actual position of the selected motor.
 * @param id motor/joint ID.
 * @return the motor/joint position [deg].
 */
float MotorBoard::getPosition(MotorJointID id) const
{
    return angles[id];
}

/**
 * @brief Gets the speed of the selected motor.
 * @param id motor/jointID.
 * @return the motor/joint speed [deg/s].
 */
float MotorBoard::getSpeed(MotorJointID id) const
{
    return speeds[id].getFiltered();
}

/**
 * @brief Gets the acceleration of the selected motor.
 * @param id motor/jointID.
 * @return the motor/joint acceleration [deg/s^2].
 */
float MotorBoard::getAcceleration(MotorJointID id) const
{
    return accelerations[id].getFiltered();
}

/**
 * @brief Gets the current of the selected motor.
 * @param id motor ID.
 * @return the motor current [A].
 */
float MotorBoard::getCurrent(MotorJointID id) const
{
    if((id & MOTOR_BIT) == MOTOR_A)
        return currentA;
    else
        return currentB;
}

float MotorBoard::getTargetCurrent(MotorJointID id) const
{
    if((id & MOTOR_BIT) == MOTOR_A)
        return currentTargetA;
    else
        return currentTargetB;
}

/**
 * @brief Changes the joint encoder offset, to match the given angle.
 * This method can be used to calibrate the incremental encoders, even if the
 * joints are not in the zero position.
 * @param id jointID.
 * @param position the new joint position [deg].
 */
void MotorBoard::setEncoderPosition(JointID id, float position)
{
    vector<uint8_t> data(sizeof(uint8_t) + sizeof(position));

    if(id == JointID::A)
        data[0] = VAR_ENCODER_POS_A;
    else
        data[0] = VAR_ENCODER_POS_B;

    memcpy(&data[1], &position, sizeof(position));

    sendPacket(PC_MESSAGE_SET_VAR, data);
}

/**
 * @brief Gets if a fault occured since the last call to this function.
 * @return true if a fault occured since the last call to this function, false
 * otherwise.
 */
bool MotorBoard::fault()
{
    if(gotFault)
    {
        gotFault = false;
        return true;
    }
    else
        return false;
}

/**
 * @brief Indicates if the temperature is too high.
 * @return true if at least one thermistor has reached TEMPERATURE_WARNING.
 */
bool MotorBoard::overTemperatureWarning()
{
    return ((temperatureA >= TEMPERATURE_WARNING) ||
            (temperatureB >= TEMPERATURE_WARNING) ||
            (temperatureC >= TEMPERATURE_WARNING));
}

/**
 * @brief Gets the thermistors temperatures.
 * @param tA address of the variable to be set to the temperature measured by
 * thermistor A [celsius], or nullptr to ignore.
 * @param tB address of the variable to be set to the temperature measured by
 * thermistor B [celsius], or nullptr to ignore.
 * @param tC address of the variable to be set to the temperature measured by
 * thermistor C [celsius], or nullptr to ignore.
 */
void MotorBoard::getTemperatures(float *tA, float *tB, float *tC)
{
    if(tA != nullptr)
        *tA = (float)temperatureA;

    if(tB != nullptr)
        *tB = (float)temperatureB;

    if(tC != nullptr)
        *tC = (float)temperatureC;
}

/**
 * @brief Returns the angle given by the absolute sensor.
 * @param id the joint to get the angle from.
 * @return the joint angle [deg].
 */
float MotorBoard::getAbsoluteSensorPosition(JointID id) const
{
    if(id == JointID::A)
        return potentiometerA;
    else
        return potentiometerB;
}

/**
 * @brief Sends a message packet to the motorboard.
 * @param messageType type of the message.
 * @param data data bytes to send.
 */
void MotorBoard::sendPacket(uint8_t messageType,
                            const std::vector<uint8_t> &data)
{
    txBuffer.resize(1 + 2*data.size() + MOTORBOARD_COMM_CRC_SIZE*2);

    // Message type.
    txBuffer[0] = (1<<7) + messageType;

    // Payload data.
    uint8_t *p = &txBuffer[1];

    for(unsigned int i=0; i<data.size(); i++)
    {
        *p = data[i] >> 4; // MSB.
        p++;
        *p = data[i] & 0xf; // LSB.
        p++;
    }

    // Checksum.
    uint32_t crc = computeMessageCrc(messageType, data.data(), data.size());

    for(int i=0; i<4; i++)
    {
        uint8_t byte = ((uint8_t*)&crc)[i];

        *p = (byte >> 4); // Most significant bits.
        p++;
        *p = byte & 0x0f; // Least significant bits.
        p++;
    }

    //
#if !SIMULATED_ENCODERS
    uart.write(txBuffer);
#endif
}

/**
 * @brief Interprets all the received bytes, sent by the motorboard.
 * @param rxBuffer bytes array to decode.
 * @return true if a valid message has been decoded, false otherwise.
 */
bool MotorBoard::processRxBytes(const vector<uint8_t> &rxBuffer)
{
    bool validMessageReceived = false;

#if SIMULATED_ENCODERS
    unused(&rxBuffer);
    validMessageReceived = true;
#else
    for(unsigned int i=0; i<rxBuffer.size(); i++)
    {
        //debug << (int)rxBuffer[i] << " " << flush;

        uint8_t rxByte = rxBuffer[i];

        if(rxByte & (1<<7)) // The start byte has the most significant bit high.
        {
            rxCurrentMessageType = (rxByte & ~(1<<7)); // Remove the start bit.
            rxBytesCount = 0;
        }
        else // The data bytes have the most significant byte low.
            rxBytesCount++;

        if(rxBytesCount % 2 == 1) // First half of the data byte has been received.
            firstHalfByte = rxByte; // Store it until the second half arrives.
        else // Second half of the data byte has been received.
        {
            int dataBytesReady = rxBytesCount/2;

            if(dataBytesReady-1 >= MOTORBOARD_RX_DATA_BYTES_BUFFER_SIZE)
            {
                if(ignoreCommErrors)
                {
                    rxCurrentMessageType = STM_MESSAGE_COUNT;
                    rxBytesCount = 0;
                    return false;
                }
                else
                {
                    throw runtime_error("Motorboard " +
                                        to_string((int)uart.getPort()) +
                                        ": serious communication error (RX "
                                        "buffer full).");
                }
            }

            if(dataBytesReady > 0)
                rxDataBytesBuffer[dataBytesReady-1] = (firstHalfByte<<4) + (rxByte&0xf);

            switch(rxCurrentMessageType)
            {
            case STM_MESSAGE_PINGBACK:
                if(currentMessageIsValid(0))
                {
                    pingtBack = true;
                    validMessageReceived = true;
                }
                break;

            case STM_MESSAGE_DEBUG_TEXT:
                // For text, no checksum is used.
                if(dataBytesReady > 0 &&
                   rxDataBytesBuffer[dataBytesReady-1] == '\0')
                {
                    string message((char*)&rxDataBytesBuffer[0]);
                    debug << "Motorboard " << (int)uart.getPort()
                          << " message: " << message << endl;

                    validMessageReceived = true;
                }
                break;

            case STM_MESSAGE_POSITION_STATUS:
                if(currentMessageIsValid(67))
                {
                    // Extract the values from the packet.
                    memcpy(&motorboardTimestamp, &rxDataBytesBuffer[0], 8);

                    memcpy(&boardVoltage, &rxDataBytesBuffer[8], 4);
                    memcpy(&boardCurrent, &rxDataBytesBuffer[12], 4);

                    memcpy(&angles[MOTOR_A], &rxDataBytesBuffer[16], 4);
                    memcpy(&angles[JOINT_A], &rxDataBytesBuffer[20], 4);
                    memcpy(&targetAngleA, &rxDataBytesBuffer[24], 4);
                    memcpy(&currentTargetA, &rxDataBytesBuffer[28], 4);
                    memcpy(&currentA, &rxDataBytesBuffer[32], 4);
                    memcpy(&potentiometerA, &rxDataBytesBuffer[36], 4);

                    memcpy(&angles[MOTOR_B], &rxDataBytesBuffer[40], 4);
                    memcpy(&angles[JOINT_B], &rxDataBytesBuffer[44], 4);
                    memcpy(&targetAngleB, &rxDataBytesBuffer[48], 4);
                    memcpy(&currentTargetB, &rxDataBytesBuffer[52], 4);
                    memcpy(&currentB, &rxDataBytesBuffer[56], 4);
                    memcpy(&potentiometerB, &rxDataBytesBuffer[60], 4);

                    temperatureA = (int8_t)rxDataBytesBuffer[64];
                    temperatureB = (int8_t)rxDataBytesBuffer[65];
                    temperatureC = (int8_t)rxDataBytesBuffer[66];

                    validMessageReceived = true;
                }
                break;

            case STM_MESSAGE_FAULT:
                if(currentMessageIsValid(1))
                {
                    uint8_t faultID = rxDataBytesBuffer[0];

                    debug << "Motorboard " << (int)uart.getPort() << " fault: ";

                    if(faultID < FAULT_MESSAGES.size())
                        debug << FAULT_MESSAGES[faultID] << endl;
                    else
                        debug << "unknown error, ID=" << faultID << endl;

                    gotFault = true;
                    validMessageReceived = true;
                }
                break;

            default: // No data bytes for the other message types.
                break;
            }
        }
    }

#endif

    return validMessageReceived;
}

/**
 * @brief Sends a ping request to the motorboard.
 * Sends a ping message to the motorboard, that should respond with a pingback
 * packet.
 * @return true if the motorboard has responded within PING_TIMEOUT_TIME, false
 * otherwise.
 * @warning This function can block until PING_TIMEOUT_TIME is elapsed.
 */
bool MotorBoard::ping()
{
    pingtBack = false;
    auto pingRequestTime = steady_clock::now();

    sendPacket(PC_MESSAGE_PING);

    PeripheralState initialState = state;
    state = PeripheralState::ACTIVE;

    while(true)
    {
        // Wait a short time, then update the board status.
        this_thread::sleep_for(duration<float>(PING_CHECK_INTERVAL));
        update(PING_CHECK_INTERVAL);

        // Has an answer to the ping request been received ?
        if(pingtBack)
        {
            state = initialState;
            return true;
        }

        // Is it too late for the motorboard to respond ?
        if((steady_clock::now() - pingRequestTime) > PING_TIMEOUT_TIME)
        {
            state = initialState;
            return false;
        }
    }
}

/**
 * @brief Computes the 32-bit checksum of the given message.
 * @param type type of the message to compute the CRC32.
 * @param data data bytes of the message to compute the CRC32.
 * @param length size of the given message data (excluding the type byte).
 * @return the CRC-32 of the given message.
 */
uint32_t MotorBoard::computeMessageCrc(uint8_t type, const uint8_t *data,
                                       uint32_t length)
{
    const uint8_t *p;

    uint32_t crc = ~0U;
    uint32_t bytesLeft = length;

    crc = CRC32_TABLE[(crc ^ type) & 0xFF] ^ (crc >> 8);

    p = data;

    while(bytesLeft-- > 0)
        crc = CRC32_TABLE[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

    return crc ^ ~0U;
}

/**
 * @brief Checks if the message being interpreted is valid.
 * This function first checks the length of the message, then computes and
 * checks the CRC.
 * @param expectedPayloadSize expected payload data size, according to the
 * message ID [B].
 * @return true if the message is valid, false otherwise.
 */
bool MotorBoard::currentMessageIsValid(int expectedPayloadSize)
{
    uint32_t computedCrc, messageCrc;

    int dataBytesReady = rxBytesCount/2;

    // Test if the current number of accumulated bytes matches the expected
    // payload size.
    if(dataBytesReady != expectedPayloadSize + MOTORBOARD_COMM_CRC_SIZE)
        return false;

    // Get the CRC given at the end of the message.
    memcpy(&messageCrc,
           &rxDataBytesBuffer[dataBytesReady-MOTORBOARD_COMM_CRC_SIZE],
           MOTORBOARD_COMM_CRC_SIZE);

    // Compute the CRC with all the other bytes of the message.
    computedCrc = computeMessageCrc(rxCurrentMessageType, rxDataBytesBuffer,
                                    dataBytesReady-MOTORBOARD_COMM_CRC_SIZE);

    // A full message has been received, ignore further data until a new message
    // header is received.
    rxCurrentMessageType = STM_MESSAGE_COUNT;

    // Compare both CRCs.
    if(computedCrc != messageCrc)
    {
#if !SIMULATED_ENCODERS
        debug << "Motorboard " << to_string((int)uart.getPort())
              << ": wrong CRC." << endl;
#endif
    }

    return (computedCrc == messageCrc);
}

/**
 * @brief Constructor.
 * @param motorboard the motorboard object associated to the desired joint.
 * @param axis the MotorJointId of the desired joint.
 * @param angularOffset angular offset [deg], to be subtracted from the measured
 * position, and added to the target position.
 */
MotorBoardAxis::MotorBoardAxis(MotorBoard &motorboard, MotorJointID axis,
                               float angularOffset) :
    motorboard(motorboard), axis(axis), angularOffset(angularOffset)
{

}

/**
 * @brief Stops powering the selected motor, so it can spin freely.
 */
void MotorBoardAxis::coast()
{
    motorboard.coast(axis);
}

/**
 * @brief Sets the voltage to apply to the motor/joint.
 * Sets the specified motor regulator in voltage mode, and applies the given
 * voltage.
 * @param voltage voltage to apply [V].
 * @note The maximum voltage that can be applied is limited by the motorboard
 * power supply voltage.
 */
void MotorBoardAxis::setVoltage(float voltage)
{
    motorboard.setVoltage(axis, voltage);
}


/**
 * @brief Sets the torque to be applied by the motor/joint.
 * Sets the specified motor regulator in current mode, and applies the given
 * torque.
 * @param torque torque to apply [N.m].
 * @note The maximum torque that can be applied is limited the motor nominal
 * current and the power supply.
 */
void MotorBoardAxis::setTorque(float torque)
{
    motorboard.setTorque(axis, torque);
}


/**
 * @brief Sets the target speed of the motor/joint.
 * @param speed target speed [deg/s].
 */
void MotorBoardAxis::setSpeed(float speed)
{
    motorboard.setSpeed(axis, speed);
}

/**
 * @brief Set the target position of the motor/joint.
 * Sets the specified motor regulator in position mode, with the specified
 * target position. It is possible to specify a time for reaching the target
 * point, in order to avoid a violent movement, in this case a smooth polynomial
 * trajectory will be generated by the motorboard.
 * @param position target position [deg].
 * @param duration duration of the trajectory to reach the target point [s]. If
 * a duration of 0 is given (or if thi argument is omitted), the motorboard will
 * try to reach the target point as fast as possible.
 */
void MotorBoardAxis::setPosition(float position, float duration)
{
    motorboard.setPosition(axis, position + angularOffset, duration);
}

/**
 * @brief Gets the actual position of the motor/joint.
 * @return the motor/joint position [deg].
 */
float MotorBoardAxis::getPosition() const
{
    return motorboard.getPosition(axis) - angularOffset;
}

/**
 * @brief Gets the speed of the motor/joint.
 * @return the motor/joint speed [deg/s].
 */
float MotorBoardAxis::getSpeed() const
{
    return motorboard.getSpeed(axis);
}

/**
 * @brief Gets the acceleration of the motor/joint.
 * @return the motor/joint acceleration [deg/s^2].
 */
float MotorBoardAxis::getAcceleration() const
{
    return motorboard.getAcceleration(axis);
}

/**
 * @brief Gets the motor current.
 * @return the motor current [A].
 */
float MotorBoardAxis::getCurrent() const
{
    return motorboard.getCurrent(axis);
}

/**
 * @brief Gets the motor target current.
 * @return the motor target current [A].
 */
float MotorBoardAxis::getTargetCurrent() const
{
    return motorboard.getTargetCurrent(axis);
}

/**
 * @brief Sets the angular offset of the joint.
 * @param offset the new angular offset [deg].
 */
void MotorBoardAxis::setAngularOffset(float offset)
{
    angularOffset = offset;
}

/**
 * @brief Gets the angular offset of the joint.
 * @return the angular offset [deg].
 */
float MotorBoardAxis::getAngularOffset() const
{
    return angularOffset;
}
