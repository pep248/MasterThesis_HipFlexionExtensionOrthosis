#include "communication.h"
#include "lib/utils.h"
#include "lib/debugstream.h"

#include <stdexcept>
#include <regex>
#include <ctime>

using namespace std;
using namespace chrono;
using namespace Utils;

#define BUFFERS_SIZE 1024
#define DATE_FORMAT "%Y-%m-%d %H:%M:%S"

#define SYNC_LED_SEQUENCE { { true, true, true, 0.2f } }

/**
 * @brief Constructor.
 * @param syncVarManager reference to the SyncVar manager.
 */
Communication::Communication(SyncVarManager &syncVarManager) :
    server(TCP_PORT), syncVarManager(syncVarManager)
{
    //
    rtc = nullptr;
    timeHasBeenSet = false;
    led = nullptr;

    //
    if(!server.start())
        throw runtime_error("Could not start the server.");
}

Communication::~Communication()
{
    debug.setCommunication(nullptr);
}

/**
 * @brief Sets the RTC module pointer.
 * @param rtc pointer to the RTC module.
 */
void Communication::setRtc(Pcf8523 *rtc)
{
    this->rtc = rtc;
}

/**
 * @brief Sets the RGB status LED, used to perform video synchronization.
 * @param led the RGB LED module.
 */
void Communication::setLed(LedStatusIndicator *led)
{
    this->led = led;
}

/**
 * @brief Updates the communication.
 * Checks the connection status, receives and processes all the received bytes,
 * and sends the periodic (heartbeat and streaming) packets, for all clients.
 * @param timestamp current timestamp [us].
 */
void Communication::update(uint64_t timestamp)
{
    // Remove the inactive sockets.
    for(int i = (signed)clients.size()-1; i >= 0; i--)
    {
        if(!clients[i]->isActive())
            clients.erase(clients.begin() + i);
    }

    // Add the clients that just connected.
    server.update();

    auto newSockets = server.getNewClients();

    for(auto &newSocket : newSockets)
    {
        shared_ptr<ClientComm> client(new ClientComm(newSocket, *this,
                                                     syncVarManager));
        clients.push_back(client);
    }

    // Manage each client.
    for(auto &client : clients)
    {
        if(client != nullptr)
            client->update(timestamp);
    }
}

/**
 * @brief Sends a debug message to all the remote clients.
 * @param message text message to send.
 */
void Communication::sendDebugMessage(string message)
{
    vector<uint8_t> data(message.begin(), message.end());
    data.push_back('\0');

    for(auto &client : clients)
    {
        if(client != nullptr)
            client->sendPacket(MbToPcMessageType::DEBUG_TEXT, data);
    }
}

/**
 * @brief Sets the OS date/time.
 * @param time the new date/time.
 * @note This method only sets the time during the first call. Subsequent calls
 * will do nothing. This avoids the jumps of the time in the logfile, when the
 * clients connect.
 */
void Communication::setTime(time_t time)
{
    // Do not re-set the date if it has already been set (to
    // avoid jumps in the logfile).
    if(timeHasBeenSet)
        return;

    // Set the OS time.
    stime(&time);

    timeHasBeenSet = true;

    // Write this event in the logs.
    debug << "The date was set by the remote client." << endl;

    // Set the RTC chip time.
    if(rtc != nullptr)
        rtc->setRtcFromLinuxTime();
}

/**
 * @brief Flashes the LED to provide a video synchronization mark.
 */
void Communication::doLedSyncPulse()
{
    if(led != nullptr)
        led->playSequence(SYNC_LED_SEQUENCE);
}

/**
 * @brief Constructor.
 * @param socket TCP/IP socket to send and receive data with the client.
 * @param comm Communication object that manages this ClientComm.
 * @param syncVarManager SyncVarManager object.
 */
ClientComm::ClientComm(shared_ptr<ClientSocket> socket, Communication &comm,
                       SyncVarManager &syncVarManager) :
    socket(socket), communication(comm), syncVarManager(syncVarManager)
{
    lastSentHeartbeatTime = steady_clock::now();
    lastSentStreamPacketTime = steady_clock::now();

    txBuffer.reserve(BUFFERS_SIZE);
    rxDataBytesBuffer.reserve(BUFFERS_SIZE);
}

/**
 * @brief Updates the communication.
 * Checks the connection status, receives and processes all the received bytes,
 * and sends the periodic (heartbeat and streaming) packets.
 * @param timestamp current timestamp [us].
 */
void ClientComm::update(uint64_t timestamp)
{
    // Check the socket status.
    if(!socket->isActive())
        return;

    //
    const vector<uint8_t>& rxBuffer = socket->receive();
    processRxBytes(rxBuffer);

    // Send a heartbeat packet periodically.
    auto timeSinceLastHeartbeat = steady_clock::now() - lastSentHeartbeatTime;

    if(timeSinceLastHeartbeat > milliseconds(TCP_MAX_TIME_WITHOUT_RX / 2))
    {
        lastSentHeartbeatTime = steady_clock::now();
        sendPacket(MbToPcMessageType::HEARTBEAT);
    }

    // Send the streaming variables periodically.
    auto timeSinceLastStreamPacket = steady_clock::now() - lastSentStreamPacketTime;

    if(timeSinceLastStreamPacket > streamingPeriod)
    {
        lastSentStreamPacketTime = steady_clock::now();
        sendStreamingPacket(timestamp);
    }
}

/**
 * @brief Sends a packet to the remote client.
 * @param type packet type.
 * @param data data payload to send.
 */
void ClientComm::sendPacket(MbToPcMessageType type,
                            const std::vector<uint8_t> &data)
{
    txBuffer.clear();
    txBuffer.push_back((1<<7) + (int)type);

    for(unsigned int i=0; i<data.size(); i++)
    {
        txBuffer.push_back(data[i] >> 4); // MSB.
        txBuffer.push_back(data[i] & 0xf); // LSB.
    }

    socket->send(txBuffer);
}

/**
 * @brief Gets if the sockets is still active.
 * @return true if the the socket is still active, false if it was closed.
 */
bool ClientComm::isActive()
{
    return socket->isActive();
}

/**
 * @brief Interprets the given bytes, and acts accordingly.
 * @param rxData the data bytes to interpret.
 */
void ClientComm::processRxBytes(const std::vector<uint8_t> &rxData)
{
    for(unsigned int i=0; i<rxData.size(); i++)
    {
        uint8_t rxByte = rxData[i];

        if(rxByte & (1<<7)) // The start byte has the most significant bit high.
        {
            rxCurrentMessageType = (PcToMbMessageType)(rxByte & ~(1<<7)); // Remove the start bit.
            rxBytesCount = 0;
            rxDataBytesBuffer.clear();
        }
        else // The data bytes have the most significant byte low.
            rxBytesCount++;

        if(rxBytesCount % 2 == 1) // First half of the data byte has been received.
            firstHalfByte = rxByte; // Store it until the second half arrives.
        else // Second half of the data byte has been received.
        {
            if(rxBytesCount > 0)
                rxDataBytesBuffer.push_back((firstHalfByte<<4) + (rxByte & 0xf));

            switch(rxCurrentMessageType)
            {
            case PcToMbMessageType::HEARTBEAT:
                break;

            case PcToMbMessageType::SET_DATE:
                if(rxDataBytesBuffer.size() == 19)
                {
                    // Create a UTC date string from the RX bytes.
                    string dateStr(rxDataBytesBuffer.begin(),
                                   rxDataBytesBuffer.end());

                    // Parse the date string and check it.
                    tm dateStruct;
                    char *result = strptime(dateStr.c_str(), DATE_FORMAT,
                                            &dateStruct);

                    if(result != nullptr)
                    {
                        // Set the date/time.
                        time_t dateSeconds = mktime(&dateStruct);

                        communication.setTime(dateSeconds);
                    }
                    else
                        debug << "SET_DATE: wrong format." << endl;
                }
                break;

            case PcToMbMessageType::GET_VARS_LIST:
                if(rxDataBytesBuffer.size() == 0)
                {
                    if(syncVarManager.isLocked())
                    {
                        ByteBuffer bb;
                        const SyncVarList &vars = syncVarManager.getVars();

                        bb << (uint16_t) vars.size();

                        for(unsigned int i=0; i<vars.size(); i++)
                        {
                            string name = vars[i]->getName();
                            name.resize(SYNCVAR_NAME_COMM_LENGTH, '\0');

                            string unit = vars[i]->getUnit();
                            unit.resize(SYNCVAR_UNIT_COMM_LENGTH, '\0');

                            bb << name
                               << unit
                               << (uint8_t) vars[i]->getType()
                               << (uint8_t) vars[i]->getAccess()
                               << vars[i]->getLength();
                        }

                        sendPacket(MbToPcMessageType::VARS_LIST, bb);
                    }
                }
                break;

            case PcToMbMessageType::GET_VAR:
                if(rxDataBytesBuffer.size() == 4)
                {
                    uint32_t varIndex;
                    memcpy(&varIndex, &rxDataBytesBuffer[0], sizeof(uint32_t));

                    if(varIndex < syncVarManager.getVars().size())
                    {
                        ByteBuffer bb;
                        bb << varIndex;
                        bb << *syncVarManager.getVars()[varIndex];

                        sendPacket(MbToPcMessageType::VAR_VALUE, bb);
                    }
                }
                break;

            case PcToMbMessageType::SET_VAR:
                if(rxDataBytesBuffer.size() >= 4)
                {
                    uint32_t varIndex;
                    memcpy(&varIndex, &rxDataBytesBuffer[0], sizeof(uint32_t));

                    const SyncVarList & syncVars = syncVarManager.getVars();

                    if(varIndex < syncVars.size() &&
                       rxDataBytesBuffer.size() == 4 + syncVars[varIndex]->getLength())
                    {
                        syncVars[varIndex]->setData(rxDataBytesBuffer, 4);
                    }
                }
                break;

            case PcToMbMessageType::SET_VAR_LOG:
                if(rxDataBytesBuffer.size() >= 4)
                {
                    uint32_t varIndex;
                    memcpy(&varIndex, &rxDataBytesBuffer[0], sizeof(uint32_t));

                    const SyncVarList & syncVars = syncVarManager.getVars();

                    if(varIndex < syncVars.size() &&
                       rxDataBytesBuffer.size() == 4 + syncVars[varIndex]->getLength())
                    {
                        SyncVar &sv = *syncVars[varIndex];

                        string previousValueStr;
                        if(sv.getAccess() == VarAccess::WRITE)
                            previousValueStr = "?";
                        else
                            previousValueStr = sv.getValueString();

                        string newValueStr = sv.setData(rxDataBytesBuffer, 4);

                        string unitStr = " " + sv.getUnit();
                        if(sv.getUnit().empty())
                            unitStr = "";

                        debug << "Set " << sv.getName()
                              << " to " << newValueStr << unitStr
                              << " (was " << previousValueStr << unitStr
                              << " before)."
                              << endl;
                    }
                }
                break;

            case PcToMbMessageType::SET_STREAMING:
                if(rxDataBytesBuffer.size() > sizeof(uint32_t))
                {
                    uint32_t nVarsToStream;
                    memcpy(&nVarsToStream, &rxDataBytesBuffer[0],
                           sizeof(uint32_t));

                    if(rxDataBytesBuffer.size() == 4 + 4 + 1 + 4*nVarsToStream)
                    {
                        // Read the streaming period.
                        uint32_t periodMs;
                        memcpy(&periodMs, &rxDataBytesBuffer[4],
                               sizeof(uint32_t));
                        streamingPeriod = chrono::milliseconds(periodMs);

                        // Read the streaming request number.
                        memcpy(&streamingRequest, &rxDataBytesBuffer[8],
                                sizeof(uint8_t));

                        // Read the indices of the variables to stream.
                        syncVarsToStream.clear();

                        for(unsigned int i=0; i<nVarsToStream; i++)
                        {
                            uint32_t varIndex;
                            memcpy(&varIndex, &rxDataBytesBuffer[9+i*4],
                                   sizeof(uint32_t));
                            syncVarsToStream.push_back(&syncVarManager.getVar(varIndex));
                        }
                    }
                }
                break;

            case PcToMbMessageType::LOG_MESSAGE:
                if(rxDataBytesBuffer.size() > 0 &&
                   rxDataBytesBuffer.back() == '\0')
                {
                    // Write the message to the log.
                    debug << "Client message: " << rxDataBytesBuffer.data()
                          << endl;
                }
                break;

            case PcToMbMessageType::SYNC_LED:
                if(rxDataBytesBuffer.size() > 0 &&
                   rxDataBytesBuffer.back() == '\0')
                {
                    // Start flashing the LED.
                    communication.doLedSyncPulse();

                    // Print the sync message into the log.
                    debug << "SYNC_LED: " << rxDataBytesBuffer.data() << endl;
                }
                break;

            default: // No data bytes for the other message types.
                break;
            }
        }
    }
}

/**
 * @brief Sends a streaming packet.
 * @param timestamp current timestamp [us].
 */
void ClientComm::sendStreamingPacket(uint64_t timestamp)
{
    if(syncVarsToStream.size() > 0)
    {
        ByteBuffer bb;

        bb << timestamp;
        bb << streamingRequest;

        for(SyncVar* vars : syncVarsToStream)
            bb << *vars;

        sendPacket(MbToPcMessageType::STREAMING, bb);
    }
}
