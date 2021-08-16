#include "crutchboard.h"

#include "../lib/debugstream.h"

using namespace std;

#define CRUTCH_LOAD_CELL_GAIN (500.0f/(0.0015*3.3f)*2.0f) // [N/V].
#define UART_BAUDRATE B115200
#define MAX_TIME_WITHOUT_RX 1.0f ///< Maximum acceptable time without receiving a valid message from the crutchboard [s].
#define RECONNECT_INTERVAL 4.0f ///< Time between two tentatives to reconnect to the crutchboard [s].

/**
 * @brief Constructor.
 * @param calibFile calibration file for the IMU, or "" if unused.
 */
ManualImu::ManualImu(string calibFile) : Imu(calibFile)
{
    state = PeripheralState::ACTIVE;
}

/**
 * @brief Destructor.
 */
ManualImu::~ManualImu()
{

}

/**
 * @brief Sets manually the raw readings of the IMU.
 * @param acceleration the new acceleration vector [m/s^2].
 * @param angularSpeed the new angularSpeed vector [deg/s].
 * @param temperature the new temperature [celsius].
 */
void ManualImu::setRawValues(Vec3f acceleration, Vec3f angularSpeed,
                             float temperature)
{
    this->rawAcceleration = acceleration;
    this->rawAngularSpeed = angularSpeed;
    this->temperature = temperature;
}

/**
 * @brief Updates the raw values.
 * @remark This method actually does nothing, since the user should set the
 * readings explicitely by calling setRawValues().
 */
void ManualImu::readAll()
{

}

/**
 * @brief Constructor.
 * @param name human-readable name, to help identification in the text logfile.
 * @param serialPort the serial port to use for communicating with the board.
 * @param loadCellCalibFile calibration file of the crutch load cell.
 * @param imuCalibFile calibration file of the crutchboard IMU.
 */
CrutchBoard::CrutchBoard(string name, UartPort serialPort,
                         string loadCellCalibFile, string imuCalibFile) :
    name(name), uart(serialPort, UART_BAUDRATE), imu(imuCalibFile),
    orientation(ImuOrientation::BACKPACK_VERTICAL_REVERSED, &imu),
    configFile(loadCellCalibFile)
{
    rxCurrentMessageType = CB_MID_COUNT;
    rxBytesCount = 0;
    timeWithoutRx = 0.0f;
    previouslyConnected = false;
    failSafeGpioValues.fill(false);

    for(auto& gc : gpioChannels)
        gc = true;

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("timestamp", "us", boardTimestamp,
                                   VarAccess::READ, false));
    syncVars.push_back(makeSyncVar("load", "N", crutchLoad, VarAccess::READ,
                                   true));
    syncVars.push_back(makeSyncVar("battery_voltage", "V", batteryVoltage,
                                   VarAccess::READ, false));
    syncVars.add("imu/", imu.getVars());
    syncVars.add("orientation/", orientation.getSyncVars());

    for(unsigned int i=0; i<gpioChannels.size(); i++)
    {
        syncVars.push_back(makeSyncVar("gpio_" + to_string(i+1), "",
                                       gpioChannels[i], VarAccess::READ, false));
    }

    syncVars.push_back(makeSyncVar<bool>("set_load_cell_zero", "", nullptr,
                                         [=](bool set)
                                         {
                                             if(set)
                                                 setLoadCellZero();
                                         }, false));

    // Read the calibration values from the file.
    crutchLoadOffset = configFile.get("crutch_load_offset", 0.0f);

    //
    if(uart.getState() != PeripheralState::ACTIVE)
    {
        debug << "CrutchBoard: could not open UART." << endl;
        state = PeripheralState::FAULT;
        return;
    }
}

/**
 * @brief Destructor.
 */
CrutchBoard::~CrutchBoard()
{
    configFile.set("crutch_load_offset", crutchLoadOffset);
}

/**
 * @brief Acquires the data from the CrutchBoard.
 * @param dt time elapsed since the last call to this function [s].
 */
void CrutchBoard::update(float dt)
{
    // Read the data received through the serial link.
    bool messageReceived = processRxBytes(uart.read());

    if(messageReceived)
    {
        timeWithoutRx = 0.0f;

        if(!previouslyConnected)
        {
            debug << "CrutchBoard: successfully connected to " << name
                  << " crutch." << endl;
            previouslyConnected = true;
        }
    }
    else
    {
        timeWithoutRx += dt;

        // If no valid message was received for too long, try to reopen the
        // serial device (needed in case the Bluetooth just reconnected).
        if(timeWithoutRx > MAX_TIME_WITHOUT_RX)
        {
            if(previouslyConnected)
            {
                debug << "CrutchBoard: lost datalink with " << name
                      << " crutch." << endl;
                previouslyConnected = false;
            }

            if(timeWithoutRx > RECONNECT_INTERVAL)
            {
                timeWithoutRx = 0.0f;
                uart.reset();
            }
        }
    }

    // Compute the board orientation from the IMU readings.
    imu.update(dt);
    orientation.update(dt);
}

/**
 * @brief Adjusts the calibration offset such that the current load is zero.
 */
void CrutchBoard::setLoadCellZero()
{
    crutchLoadOffset = crutchLoad + crutchLoadOffset;
}

/**
 * @brief Gets the load measured by the load cell.
 * @return the axial load measured [N].
 */
float CrutchBoard::getLoad() const
{
    return crutchLoad;
}

/**
 * @brief Gets the state of all the GPIO channels.
 * @return The current state of the GPIO channels, or the failsafe values in
 * case the datalink is down.
 */
const std::array<bool, 9>& CrutchBoard::getGpioChannels() const
{
    if(previouslyConnected)
        return gpioChannels;
    else
        return failSafeGpioValues;
}

/**
 * @brief Sets the GPIO failsafe values, to be used when the datalink is down.
 * @param gpioStates the failsafe values for all the GPIOs.
 */
void CrutchBoard::setFailSafeGpioStates(std::array<bool, 9> gpioStates)
{
    failSafeGpioValues = gpioStates;
}

/**
 * @brief Indicates the status of the datalink with the crutchboard.
 * @return true if data is received periodically, false otherwise.
 */
bool CrutchBoard::isConnected() const
{
    return previouslyConnected;
}

/**
 * @brief Process the bytes received, and update the state variables.
 * @param rxBuffer bytes received since the last call to this method, to be
 * interpreted.
 * @return true if at least one valid message could be read, false otherwise.
 */
bool CrutchBoard::processRxBytes(const std::vector<uint8_t> &rxBuffer)
{
    bool validMessageReceived = false;

    for(unsigned int i=0; i<rxBuffer.size(); i++)
    {
        uint8_t rxByte = rxBuffer[i];

        //cout << (int)rxByte << endl;

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
                throw runtime_error("Crutchboard " +
                                    to_string((int)uart.getPort()) +
                                    ": serious communication error (RX "
                                    "buffer full).");
            }

            if(dataBytesReady > 0)
                rxDataBytesBuffer[dataBytesReady-1] = (firstHalfByte<<4) + (rxByte&0xf);

            switch(rxCurrentMessageType)
            {
            case CB_MID_DEBUG_TEXT:
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

            case CB_MID_STATUS:
                if(currentMessageIsValid(sizeof(uint64_t) + CB_N_VARS*sizeof(float)))
                {
                    // Extract the values from the packet.
                    //uint64_t timestamp; // [us].
                    memcpy(&boardTimestamp, &rxDataBytesBuffer[0], sizeof(uint64_t));

                    array<float, CB_N_VARS> samples;

                    for(int i=0; i<CB_N_VARS; i++)
                    {
                        memcpy(&samples[i],
                               &rxDataBytesBuffer[8+i*sizeof(float)],
                               sizeof(float));
                    }

                    crutchLoad = samples[0] * CRUTCH_LOAD_CELL_GAIN
                                 - crutchLoadOffset;

                    uint32_t gpioVals;
                    memcpy(&gpioVals, &samples[1], 4);

                    for(unsigned int i=0; i<gpioChannels.size(); i++)
                        gpioChannels[i] = (gpioVals & (1<<i)) != 0;

                    validMessageReceived = true;
                }
                break;

            default: // No data bytes for the other message types.
                break;
            }
        }
    }

    return validMessageReceived;
}

/**
 * @brief Check the validity of the message being received.
 * @param expectedPayloadSize expected size of the message payload [B].
 * @return true if the message is valid (right size and right checksum), false
 * otherwise.
 */
bool CrutchBoard::currentMessageIsValid(int expectedPayloadSize)
{
    uint32_t computedCrc, messageCrc;

    int dataBytesReady = rxBytesCount/2;

    // Test if the current number of accumulated bytes matches the expected
    // payload size.
    if(dataBytesReady != expectedPayloadSize + CRUTCHBOARD_COMM_CRC_SIZE)
        return false;

    // Get the CRC given at the end of the message.
    memcpy(&messageCrc,
           &rxDataBytesBuffer[dataBytesReady-CRUTCHBOARD_COMM_CRC_SIZE],
           CRUTCHBOARD_COMM_CRC_SIZE);

    // Compute the CRC with all the other bytes of the message.
    computedCrc = MotorBoard::computeMessageCrc(rxCurrentMessageType,
                                                rxDataBytesBuffer,
                                                dataBytesReady-CRUTCHBOARD_COMM_CRC_SIZE);

    // A full message has been received, ignore further data until a new message
    // header is received.
    rxCurrentMessageType = CB_MID_COUNT;

    // Compare both CRCs.
    if(computedCrc != messageCrc)
    {
        debug << "Crutchboard " << to_string((int)uart.getPort())
              << ": wrong CRC." << endl;
    }

    return (computedCrc == messageCrc);
}
