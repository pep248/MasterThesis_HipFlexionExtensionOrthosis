#include "ads7844.h"

using namespace std;

#define SPI_SPEED_HZ 1000000 // Should be lower than 2.5 MHz.
#define N_CHANNELS 8 // Number of channels.
#define BUFFERS_SIZE (1+2*N_CHANNELS) // First setup byte, then 2 bytes per channel.
#define ADC_MAX 4095.0f // ADC 12 bits: 2^12-1 [].

const uint8_t CONFIG_BITS = (1<<7) | // Start bit.
                            (1<<2) | // Single-ended (non-differential) mode.
                            (0<<0);  // Power down between each conversion.

/**
 * @brief Constructor.
 * @param spi the SPI bus the ADS7844 is connected to.
 * @param chipselect the SPI chipselect pin corresponding to the ADS7844.
 * @param refVoltage voltage applied to the pin Vref [V].
 */
Ads7844::Ads7844(SpiBus &spi, const Gpio_Id &chipselect, float refVoltage)
    : spi(spi, chipselect, SPI_SPEED_HZ, SPI_MODE_3), refVoltage(refVoltage)
{
    // Create the SyncVars.
    for(int i=0; i<N_CHANNELS; i++)
    {
        syncVars.push_back(makeSyncVar("ch" + to_string(i), "V", lastSamples[i],
                                       VarAccess::READ, false));
    }

    // Build the TX data that will query all the channels.
    txBuffer.resize(BUFFERS_SIZE);
    rxBuffer.resize(BUFFERS_SIZE);

    for(int i=0; i<N_CHANNELS; i++)
    {
        txBuffer[i*2] = (i<<4) | CONFIG_BITS; // Acquire the i-th channel.
        txBuffer[i*2+1] = 0;
    }

    // Check the SPI bus status.
    state = spi.getState();

    // Read a first time, to check if the sensor is working properly.
    update(0.0f);
}

/**
 * @brief Acquires all the channels.
 */
void Ads7844::update(float)
{
    if(state == PeripheralState::DISABLED || state == PeripheralState::FAULT)
        return;

    // Perform the actual SPI transaction.
    spi.rawTransfer(txBuffer, rxBuffer);

    // Check if the device responded.
    if(rxBuffer[0] == 0xff)
    {
        // Error, the ADS7844 is probably not responding (MISO line is always
        // high).
        state = PeripheralState::FAULT;
        return;
    }

    // Assemble the received bytes in 16 bits samples, and convert them to
    // voltages in float.
    uint8_t *rawData = &rxBuffer[1];

    for(int i=0; i<N_CHANNELS; i++)
    {
        uint16_t sample = (((uint16_t)rawData[i*2  ]) << 5) |
                          (((uint16_t)rawData[i*2+1]) >> 3);

        lastSamples[i] = ((float)sample) / ADC_MAX * refVoltage;
    }
}

/**
 * @brief Gets the last measured samples.
 * @return A Vec8 with the eight channels voltages [V].
 */
const VecNf<8>& Ads7844::getLastSamples()
{
    return lastSamples;
}

/**
 * @brief Reads a single channel.
 * @param channel the channel number to read. If channel is not in the range
 * [0-7], then a runtime_error exception will be thrown.
 * @return The selected channel voltage.
 * @remark This function returns 0.0f if the ADS7844 did not respond.
 */
float Ads7844::readChannel(int channel)
{
    // Check the channel number.
    if(channel < 0 || channel > N_CHANNELS-1)
    {
        throw std::runtime_error(
            string("ADC124S01 channel should be between 0 and 7, got ")
            + to_string(channel) + ".");
    }

    // Acquire the selected channel.
    vector<uint8_t> tx(3);
    vector<uint8_t> rx(3);

    tx[0] = (channel<<4) | CONFIG_BITS;
    tx[1] = 0;
    tx[2] = 0;

    // Perform the actual SPI transaction.
    spi.rawTransfer(tx, rx);

    // Check if the device responded.
    if(rx[0] == 0xff)
    {
        // Error, the ADS7844 is probably not responding (MISO line is always
        // high).
        state = PeripheralState::FAULT;
        return 0.0f;
    }

    // Assemble the received bytes in a 16 bits sample, and convert it to a
    // voltage in float.
    uint8_t *rawData = &rx.data()[1];

    uint16_t sample = (((uint16_t)rawData[0]) << 5) |
                      (((uint16_t)rawData[1]) >> 3);

    return ((float)sample) / ADC_MAX * refVoltage;
}
