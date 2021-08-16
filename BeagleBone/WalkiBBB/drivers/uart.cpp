#include "uart.h"
#include "../config/config.h"
#include "../lib/utils.h"

using namespace std;

const char* UART_PATHS[] =
{
    "/dev/ttyO1",
    "/dev/ttyO2",
    "/dev/ttyO4",
    "/dev/ttyO5",

    "/dev/rfcomm1",
    "/dev/rfcomm2"
};

const char* UART_OVERLAYS[] =
{
    "BB-UART1",
    "BB-UART2",
    "BB-UART4",
    "BB-UART5",
    "",
    ""
};

/*const string UART_RX_PINS[] =
{
    "P9.26",
    "P9.22",
    "P9.11",
    "P8.38"
};

const string UART_TX_PINS[] =
{
    "P9.24",
    "P9.21",
    "P9.13",
    "P8.37"
};*/

#define RX_BUFFER_SIZE 1024

/**
 * @brief Constructor.
 * @param port UART port.
 * @param baudRate UART baud rate.
 */
Uart::Uart(UartPort port, speed_t baudRate)
{
    this->uartPort = port;
    this->baudRate = baudRate;
    
    rxBuffer.reserve(RX_BUFFER_SIZE);

    fileDescriptor = -1;

    reset();
}

/**
 * @brief Destructor.
 */
Uart::~Uart()
{
    if(fileDescriptor >= 0)
    {
        close(fileDescriptor);
        fileDescriptor = -1;
    }
}

/**
 * @brief Resets and setups the serial communication port.
 */
void Uart::reset()
{
#ifdef __arm__
    try
    {
        //
        if(fileDescriptor >= 0)
            close(fileDescriptor);

        // Set the pins muxing.
        if(strlen(UART_OVERLAYS[uartPort]) > 0)
            Utils::loadOverlay(UART_OVERLAYS[uartPort], UART_PATHS[uartPort]);

        // Open the file device.
        fileDescriptor = ::open(UART_PATHS[uartPort], O_RDWR | O_NOCTTY | O_NDELAY);

        if(fileDescriptor < 0)
            throw runtime_error("UART: could not open serial device.");

        // Create a termios structure and fill it.
        termios tty;
        memset(&tty, 0, sizeof(tty));

        // Setup input modes.
        tty.c_iflag &= ~BRKINT; // No signal interrupt on break.
        tty.c_iflag |= IGNBRK; // Ignore break condition.
        tty.c_iflag &= ~(ICRNL | IGNCR | INLCR | ISTRIP); // No character remapping or ignoring.
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable flow control.
        tty.c_iflag |= IGNPAR; // Ignore framing and parity errors.
        tty.c_iflag &= ~(INPCK | PARMRK); // Disable parity check and marking.

        // Setup output modes.
        tty.c_oflag = 0; // Diable post-process, char mapping and delays.

        // Setup control modes.
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit data bytes.
        tty.c_cflag |= (CLOCAL | CREAD); // Ignore modem status lines, enable receiver.
        tty.c_cflag &= ~PARENB; // Disable parity.
        tty.c_cflag &= ~CSTOPB; // One stop bit.
        tty.c_cflag &= ~CRTSCTS; // Disable flow control.

        // Setup baud rate.
        cfsetospeed(&tty, baudRate);
        cfsetispeed(&tty, baudRate);

        // Setup local modes.
        tty.c_lflag = 0; // No echo, no canonical input, no ext char, no signals.

        // Set control characters.
        tty.c_cc[VMIN]  = 0; // Non-blocking read.
        tty.c_cc[VTIME] = 0; // 0 timeout.

        // Flush the RX and TX queues.
        tcflush(fileDescriptor, TCIOFLUSH);

        // Apply the settings.
        if(tcsetattr(fileDescriptor, TCSANOW, &tty) != 0)
            throw runtime_error("UART: could not apply settings.");
    }
    catch(runtime_error)
    {
        state = PeripheralState::FAULT;
        return;
    }

    state = PeripheralState::ACTIVE;
#else
    state = PeripheralState::DISABLED;
#endif
}

/**
 * @brief Writes the given bytes to the UART port.
 * @param data vector of data bytes to write.
 */
void Uart::write(const std::vector<uint8_t> &data)
{
#ifdef __arm__
    if(state == PeripheralState::ACTIVE)
    {
        unsigned int n = ::write(fileDescriptor, &data[0], data.size());

        if(n != data.size())
            throw runtime_error("Uart::write could not write all bytes.");
    }
    else
    {
        throw runtime_error(string("UART device ") + UART_PATHS[uartPort] +
                            string(" is not opened."));
    }
#else
    unused(data);
#endif
}

/**
 * @brief Gets all the bytes received from the UART.
 * @return A const reference to the vector of received bytes.
 */
const vector<uint8_t>& Uart::read()
{
#ifdef __arm__
    if(state == PeripheralState::ACTIVE)
    {
        rxBuffer.resize(RX_BUFFER_SIZE);
        int nChars = ::read(fileDescriptor, &rxBuffer[0], RX_BUFFER_SIZE);
        rxBuffer.resize(nChars);
        return rxBuffer;
    }
    else
    {
        throw runtime_error(string("UART device ") + UART_PATHS[uartPort] +
                            string(" is not opened."));
    }
#else
    rxBuffer.clear();
    return rxBuffer;
#endif
}

UartPort Uart::getPort() const
{
    return uartPort;
}
