#ifndef DEF_COMMUNICATION_H
#define DEF_COMMUNICATION_H

#include <iostream>
#include <stdexcept>
#include <chrono>

#include "public_definitions.h"
#include "server.h"
#include "drivers/motorboard.h"
#include "drivers/pcf8523.h"
#include "drivers/ledstatusindicator.h"
#include "lib/bytebuffer.h"
#include "lib/syncvar/syncvarmanager.h"
#include "lib/peripheral.h"

/**
 * @defgroup Communication Communication
 * @brief Manages the communication protocol to talk with a remote computer.
 * @ingroup Main
 * @{
 */

class ClientComm;

/**
 * @brief Manages the communication with all the remote clients.
 */
class Communication
{
public:
    Communication(SyncVarManager &syncVarManager);
    ~Communication();
    void setRtc(Pcf8523 *rtc);
    void setLed(LedStatusIndicator *led);
    void update(uint64_t timestamp);
    void sendDebugMessage(std::string message);

    void setTime(time_t time);
    void doLedSyncPulse();

private:
    Server server; ///< TCP server to communicate with the client.
    SyncVarManager &syncVarManager; ///< SyncVar manager.

    Pcf8523 *rtc; ///< RTC driver.
    bool timeHasBeenSet; ///< true if a remote has already send the SET_DATE message, false otherwise.
    LedStatusIndicator *led;

    std::vector<std::shared_ptr<ClientComm>> clients;
};

/**
 * @brief Manages the communication with a remote client.
 */
class ClientComm
{
public:
    ClientComm(std::shared_ptr<ClientSocket> socket, Communication &comm,
               SyncVarManager &syncVarManager);
    void update(uint64_t timestamp);
    void sendPacket(MbToPcMessageType type,
                    const std::vector<uint8_t> &data = std::vector<uint8_t>());
    bool isActive();

private:
    void processRxBytes(const std::vector<uint8_t> &rxData);
    void sendStreamingPacket(uint64_t timestamp);

    std::shared_ptr<ClientSocket> socket;
    Communication &communication;
    SyncVarManager &syncVarManager; ///< SyncVar manager.

    PcToMbMessageType rxCurrentMessageType; ///< Type of the packet currently processed.
    int rxBytesCount; ///< Number of (half) bytes received.
    uint8_t firstHalfByte; ///< Temporary storage of the first half of all incomming bytes.

    std::vector<uint8_t> txBuffer, ///< Pre-allocated buffer for the data to send.
                         rxDataBytesBuffer; ///< Pre-allocated buffer for building a single RX packet.
    std::vector<SyncVar*> syncVarsToStream; ///< List of the SyncVar to stream to the remote client.
    std::chrono::duration<int, std::milli> streamingPeriod; ///< Time between each streaming packet sending [ms].
    uint8_t streamingRequest; ///< Current streaming request number.
    std::chrono::time_point<std::chrono::steady_clock,
                            std::chrono::nanoseconds> lastSentStreamPacketTime; ///< Time of last stream packet sending [ns].
    std::chrono::time_point<std::chrono::steady_clock,
                            std::chrono::nanoseconds> lastSentHeartbeatTime; ///< Time of the last heartbeat packet sending [ns].
};

/**
 * @}
 */

#endif
