#include "ads1148.h"

#include <cstdio>
#include <cstdlib>

using namespace std;

#define SPI_SPEED_HZ 2000000 // Should be <=2 MHz [Hz].
#define BUFFERS_SIZE 8
#define REF_VOLTAGE 3.3f // [V].

const float GAIN_FROM_REG_VAL[] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f,
                                    64.0f, 128.0f };
const uint8_t WAKEUP_COMMAND = 0x0;
const uint8_t RESET_COMMAND = 0x6;
const uint8_t RDATA_COMMAND = 0x12;
const uint8_t RDATAC_COMMAND = 0x14;
const uint8_t SDATAC_COMMAND = 0x16;
const uint8_t RREG_COMMAND = 0x20;
const uint8_t WREG_COMMAND = 0x40;

#define MUX0_REG 0x0
#define SYS0_REG 0x3
#define FSC0_REG 0x7

#define FSC_UNIT_GAIN (0x400000)

#define ADC_MAX_VALUE 65535.0f // ADC 16 bits: 2^15-1.

/**
 * @brief Constructor.
 * @param spi the SPI bus the ADS1148 is connected to.
 * @param chipselect the SPI chipselect pin corresponding to the ADS1148.
 * @param rate update rate of the ADS1148.
 */
Ads1148::Ads1148(SpiBus &spi, const Gpio_Id &chipselect, Datarate rate)
    : spi(spi, chipselect, SPI_SPEED_HZ, SPI_MODE_1)
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

    // Check the SPI bus status.
    state = spi.getState();

    // Reset the device.
    this->spi.rawTransfer({RESET_COMMAND}, rxBuffer);
    this_thread::sleep_for(chrono::milliseconds(2));

    // Wake-up the device.
    this->spi.rawTransfer({WAKEUP_COMMAND}, rxBuffer);

    // Stop the continuous reading mode.
    this->spi.rawTransfer({SDATAC_COMMAND}, rxBuffer);

    // Setup the channels (muxing).
    setChannel(0);

    // Set the gain and the sampling rate.
    uint8_t sys0Config = (GAIN_1<<4) | // PGA gain of 1 (default).
                         (rate<<0);    // Output data rate.
    writeRegister(SYS0_REG, {sys0Config});
    this->gain = GAIN_FROM_REG_VAL[GAIN_1];

    // Read a first time, to check if the sensor is working properly.
    update(0.0f);
}

/**
 * @brief Acquires all the channels.
 * @param dt the time elapsed since the last call to this function [s]. Actually
 * unused.
 */
void Ads1148::update(float)
{
    if(state == PeripheralState::DISABLED || state == PeripheralState::FAULT)
        return;

    // Read all the channels.
    for(int channelIndex=0; channelIndex<4; channelIndex++)
    {
        // Set the channel (muxing).
        setChannel(channelIndex);
        this_thread::sleep_for(chrono::microseconds(5000));

        // Read and convert the voltage value.
        txBuffer = { RDATA_COMMAND, 0, 0 };
        spi.rawTransfer(txBuffer, rxBuffer);

        // Check that the device answered the command. If 0xff for all bytes, it
        // proably means that the MISO line remained high, and the communication
        // does not work.
        if(all_of(rxBuffer.begin(), rxBuffer.end(),
                  [](uint8_t b){ return b == 0xff; }))
        {
            state = PeripheralState::FAULT;
        }

        // Convert the register value to a voltage.
        uint16_t rawAdcValue = (int16_t)((((uint16_t)rxBuffer[1])<<8) |
                                         (uint16_t)rxBuffer[2]);

        lastSamples[channelIndex] = ((float)rawAdcValue)
                                    / ADC_MAX_VALUE * REF_VOLTAGE / gain;
    }
}

/**
 * @brief Sets the integrated amplifier (PGA) gain.
 * @param gain amplification of the programmable gain amplifier.
 */
void Ads1148::setGain(Ads1148::Gain gain)
{
    vector<uint8_t> sys0Value(1);
    vector<uint8_t> fscValue(3);

    this->gain = GAIN_FROM_REG_VAL[gain];

    // Set the analog (PGA) gain.
    readRegister(SYS0_REG, sys0Value);
    sys0Value[0] = (sys0Value[0] & 0xf) | (gain<<4);
    writeRegister(SYS0_REG, sys0Value);

    // Set the digital (full-scale calibration) gain.
    fscValue[0] = FSC_UNIT_GAIN & 0xff;
    fscValue[1] = (FSC_UNIT_GAIN>>8) & 0xff;
    fscValue[2] = (FSC_UNIT_GAIN>>16) & 0xff;

    writeRegister(FSC0_REG, fscValue);
}

/**
 * @brief Gets the last measured samples.
 * @return A Vec4 with the four channels voltages [V].
 */
const Vec4f& Ads1148::getLastSamples()
{
    return lastSamples;
}

/**
 * @brief Sets the current channel, to be read later with the RDATA command.
 * @param channelIndex channel index [0-3].
 */
void Ads1148::setChannel(int channelIndex)
{
    uint8_t mux0Config = (0<<6) | // Burn-out current source off.
                         ((channelIndex*2  )<<3) | // ADC positive input.
                         ((channelIndex*2+1)<<0); // ADC negative.
    writeRegister(MUX0_REG, {mux0Config});
}

/**
 * @brief Sets a register to the desired value.
 * @param registerAddress address of the register.
 * @param registerValue value to be written to the register.
 */
void Ads1148::writeRegister(uint8_t registerAddress,
                            const std::vector<uint8_t> registerValue)
{
    txBuffer.clear();
    txBuffer.push_back(WREG_COMMAND | registerAddress); // Write register designated by the 4-bits address.
    txBuffer.push_back(registerValue.size() - 1); // Number of bytes to read, minus 1.
    txBuffer.insert(txBuffer.end(), registerValue.begin(), registerValue.end());

    spi.rawTransfer(txBuffer, rxBuffer);
}

/**
 * @brief Gets the value of a register.
 * @param registerAddress address of the register.
 * @param registerValue vector that will contain the values read from the
 * register. Its size determines the number of bytes to read.
 */
void Ads1148::readRegister(uint8_t registerAddress,
                           std::vector<uint8_t> &registerValue)
{
    txBuffer.clear();
    txBuffer.push_back(RREG_COMMAND | registerAddress); // Read register designated by the 4-bits address.
    txBuffer.push_back(registerValue.size() - 1); // Number of bytes to write, minus 1.
    txBuffer.resize(registerValue.size() + 2);

    spi.rawTransfer(txBuffer, rxBuffer);

    copy(rxBuffer.begin()+2, rxBuffer.end(), registerValue.begin());
}
