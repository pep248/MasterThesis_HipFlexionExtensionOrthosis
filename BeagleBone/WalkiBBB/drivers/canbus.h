#ifndef DEF_DRIVERS_CANBUS_H
#define DEF_DRIVERS_CANBUS_H

#include <string>
#include <vector>
#include <stdexcept>

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

#include "../public_definitions.h"
#include "../lib/peripheral.h"
#include <linux/can.h>
#include <linux/can/raw.h>

/**
 * @defgroup CAN CAN
 * @brief BeagleBone Black CAN bus.
 * @ingroup Drivers
 * @{
 */


/**
 * @brief CAN bus manager.
 */
class CanBus : public Peripheral
{
public:
    CanBus();
    ~CanBus();
    void PushMessage(can_frame frame);
    int PullMessage(unsigned int id, std::vector<can_frame>& rxFrames);
    void Update();
protected:
    void Send();
    void Receive();
private:
    int s;
    std::map<unsigned int, std::vector<can_frame>> receive_memory;
    std::vector<can_frame> pending_request;
};

/**
 * @}
 */

#endif
