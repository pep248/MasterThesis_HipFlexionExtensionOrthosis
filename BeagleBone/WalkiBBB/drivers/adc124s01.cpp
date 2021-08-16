#include "adc124s01.h"

#include <cstdio>
#include <cstdlib>

using namespace std;

#define SPI_SPEED_HZ 800000 // Should be between 0.8 and 3.2 MHz.
#define BUFFERS_SIZE 8
#define SUPPLY_VOLTAGE 3.3f // [V].
#define ADC_MAX 4095.0f // ADC 12 bits: 2^12-1 [].

/**
 * @brief Constructor.
 * @param spi the SPI bus the Adc124S01 is connected to.
 * @param chipselect the SPI chipselect pin corresponding to the Adc124S01.
 */
Adc124S01::Adc124S01(SpiChannel &spi) : spi(spi)
{
    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("a", "V", lastSamples.a, VarAccess::READ,
                                   false));
    syncVars.push_back(makeSyncVar("b", "V", lastSamples.b, VarAccess::READ,
                                   false));
    syncVars.push_back(makeSyncVar("c", "V", lastSamples.c, VarAccess::READ,
                                   false));
    syncVars.push_back(makeSyncVar("d", "V", lastSamples.d, VarAccess::READ,
                                   false));

    // Build the TX data that will query all the channels.
    txBuffer.resize(BUFFERS_SIZE);
    rxBuffer.resize(BUFFERS_SIZE);

    txBuffer[0] = (1 << 3);
    txBuffer[2] = (2 << 3);
    txBuffer[4] = (3 << 3);
    txBuffer[6] = (0 << 3);

    // Check the SPI bus status and setup the channel.
    state = spi.getBus().getState();
    spi.setSpeed(SPI_SPEED_HZ);
    spi.setMode(SPI_MODE_3);

    // Read a first time, to check if the sensor is working properly.
    update(0.0f);
}

/**
 * @brief Acquires all the channels.
 */
void Adc124S01::update(float)
{
    if(state == PeripheralState::DISABLED || state == PeripheralState::FAULT)
        return;

    spi.rawTransfer(txBuffer, rxBuffer);

    uint8_t *rawData = rxBuffer.data();

    uint16_t samples[4];

    samples[0] = (((uint16_t)rawData[0]) << 8) | (uint16_t)rawData[1];
    samples[1] = (((uint16_t)rawData[2]) << 8) | (uint16_t)rawData[3];
    samples[2] = (((uint16_t)rawData[4]) << 8) | (uint16_t)rawData[5];
    samples[3] = (((uint16_t)rawData[6]) << 8) | (uint16_t)rawData[7];

    // Check if the samples look valid.
    if(samples[0] == 0xffff && samples[1] == 0xffff &&
       samples[2] == 0xffff && samples[3] == 0xffff)
    {
        // Error, the ADC124S01 is probably not responding (MISO line is always
        // high).
        state = PeripheralState::FAULT;
    }
    else
    {
        lastSamples.a = ((float)samples[0]) / ADC_MAX * SUPPLY_VOLTAGE;
        lastSamples.b = ((float)samples[1]) / ADC_MAX * SUPPLY_VOLTAGE;
        lastSamples.c = ((float)samples[2]) / ADC_MAX * SUPPLY_VOLTAGE;
        lastSamples.d = ((float)samples[3]) / ADC_MAX * SUPPLY_VOLTAGE;
    }
}

/**
 * @brief Gets the last measured samples.
 * @return A Vec4 with the four channels voltages [V].
 */
Vec4f Adc124S01::getLastSamples()
{
    return lastSamples;
}

/**
 * @brief Reads a single channel.
 * @param channel the channel number to read. If channel is not in the range
 * [0-3], then a runtime_error exception will be thrown.
 * @return The selected channel voltage.
 */
float Adc124S01::readChannel(int channel)
{
    if(channel < 0 || channel > 3)
    {
        throw std::runtime_error(string("ADC124S01 channel should be between 0 and 3, got ")
                                 + to_string(channel) + ".");
    }

    // Request the desired channel.
    vector<uint8_t> txBuff, rxBuff;

    txBuff.push_back(channel << 3);
    txBuff.push_back(0);
    txBuff.push_back(channel << 3);
    txBuff.push_back(0);

    spi.rawTransfer(txBuff, rxBuff);

    //
    uint16_t sample = (((uint16_t)rxBuff.data()[2]) << 8) | (uint16_t)rxBuff.data()[3];

    return ((float)sample) / ADC_MAX * SUPPLY_VOLTAGE;
}
