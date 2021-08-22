#ifndef DEF_DRIVERS_UART_H
#define DEF_DRIVERS_UART_H

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

#include "../public_definitions.h"
#include "../lib/peripheral.h"

/**
 * @defgroup UART UART
 * @brief BeagleBone Black UART bus.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief UART ports enumeration.
 */
enum UartPort
{
    UART_PORT_A = 0,
    UART_PORT_B,
    UART_PORT_C,
    UART_PORT_D,

    UART_BT_A,
    UART_BT_B
};

/**
 * @brief UART bus manager.
 * C++ wrapper around the Linux userspace UART driver.
 */
class Uart : public Peripheral
{
public:
    Uart(UartPort port, speed_t baudRate);
    ~Uart();
    void reset();
    void write(const std::vector<uint8_t> &data);
    const std::vector<uint8_t>& read();
    UartPort getPort() const;
    
private:
    int fileDescriptor; ///< C-style device file handle.
    UartPort uartPort; ///< UART port.
    speed_t baudRate; ///< UART baud rate.
    std::vector<uint8_t> rxBuffer; ///< Pre-allocated buffer to receive data.
};

/**
 * @}
 */

#endif
