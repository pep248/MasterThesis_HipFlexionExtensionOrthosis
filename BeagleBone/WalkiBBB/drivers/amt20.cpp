#include "amt20.h"

#include <chrono>

#include "../lib/utils.h"

using namespace std;
using namespace chrono;

#define MAX_ATTEMPTS_READ 10 ///< Maximum number of attempts when reading.
#define MAX_ATTEMPTS_WRITE 100 ///< Maximum number of attempts when writing.
#define INVERT_RX true ///< Bitwise invert MISO data.
#define SPI_SPEED 500000 ///< SPI clock frequency [Hz].

#define COMMAND_NOP_A5 0x00
#define COMMAND_RD_POS 0x10
#define COMMAND_SET_ZERO_POINT 0x70

#define ANSWER_DEFAULT 0xa5
#define ANSWER_ZERO_DONE 0x80

/**
 * @brief Constructor.
 * @param spi SPI bus.
 * @param chipselect SPI chipselect pin corresponding to the encoder.
 */
Amt20::Amt20(SpiChannel &spi) : spi(spi)
{
    // Check that the SPI is working properly, and setup the channel.
    if(spi.getBus().getState() != ACTIVE)
    {
        state = DISABLED;
        return;
    }

    this->spi.setSpeed(SPI_SPEED);
    this->spi.setMode(SPI_MODE_0);

    // Check that the encoder is responding properly.
    doSpiTransaction(COMMAND_NOP_A5);
    uint8_t rxByte = doSpiTransaction(COMMAND_NOP_A5);

    if(rxByte == ANSWER_DEFAULT)
        state = ACTIVE;
    else
        state = FAULT;
}

/**
 * @brief Reads the latest encoder angle value.
 * @param dt time elapsed since the last call to this function [s].
 */
void Amt20::update(float)
{
    if(state != ACTIVE)
        return;

    uint8_t rxByte = doSpiTransaction(COMMAND_RD_POS);

    int nAttempts = 0;

    while(rxByte != COMMAND_RD_POS)
    {
        nAttempts++;

        if(nAttempts > MAX_ATTEMPTS_READ)
            return;

        this_thread::sleep_for(microseconds(20));

        rxByte = doSpiTransaction(COMMAND_NOP_A5);
    }

    uint16_t msb = doSpiTransaction(COMMAND_NOP_A5);
    uint16_t lsb = doSpiTransaction(COMMAND_NOP_A5);

    uint16_t rawValue = (uint16_t)((msb<<8) | lsb);
    angle = ((float)rawValue) / 4095.0f * 360.0f;

    Utils::constrainPeriodic(angle, -180.0f, 180.0f);
}

/**
 * @brief Gets the angle measured by the encoder.
 * @return the last angle read from the encoder, between -180 and 180 deg.
 */
const float &Amt20::getAngle() const
{
    return angle;
}

/**
 * @brief Persistently set the encoder zero position.
 * @return true if successful, false if it failed.
 */
bool Amt20::setZero()
{
    doSpiTransaction(COMMAND_SET_ZERO_POINT);

    for(int nAttempts = 0; nAttempts < MAX_ATTEMPTS_WRITE; nAttempts++)
    {
        uint8_t answer = doSpiTransaction(COMMAND_NOP_A5);

        if(answer == ANSWER_ZERO_DONE)
            return true;

        this_thread::sleep_for(microseconds(100));
    }

    return false; // Did not get confirmation, error.
}

/**
 * @brief Performs a single-byte SPI transaction with the encoder.
 * @param command byte to send to the encoder.
 * @return the data byte the encoder sent while command was being written.
 */
uint8_t Amt20::doSpiTransaction(uint8_t command)
{
    vector<uint8_t> txData;
    vector<uint8_t> rxData;

    txData = { command };
    spi.rawTransfer(txData, rxData);

    if(rxData.empty())
        return 0; // Error.

#if INVERT_RX
    rxData[0] = ~rxData[0];
#endif

    return rxData[0];
}
