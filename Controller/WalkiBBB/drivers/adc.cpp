#include "adc.h"
#include "../lib/utils.h"
#include "../lib/debugstream.h"

#define ADC_REF_VOLTAGE 1.8f // [V].
#define ADC_MAX 4095.0f // 12-bit ADC [].
#define ADC_OVERLAY "BB-ADC"

using namespace std;

/**
 * @brief Adc constructor.
 * @param channel: the ADC channel to read later.
 */
Adc::Adc(AdcChannel channel)
{
#ifndef __arm__
    state = PeripheralState::DISABLED;
    return;
#endif

    // Build the path of the file device to read later.
    filename = "/sys/bus/iio/devices/iio:device0/in_voltage"
               + to_string(channel) + "_raw";

    // Load the ADC overlay.
    try
    {
        Utils::loadOverlay(ADC_OVERLAY, filename);
        state = PeripheralState::ACTIVE;

        // Open the file.
        file.open(filename, ios::in);

        if(!file.is_open()) // Error.
        {
            state = PeripheralState::FAULT;
            return;
        }
    }
    catch(runtime_error&)
    {
        state = PeripheralState::FAULT;
    }
}

/**
 * @brief Acquires a sample from the ADC.
 */
void Adc::update(float)
{
    try
    {
        file.clear();
        file.seekg(0);

        // Read the ADC value from the file.
        float adcRawValue;
        file >> adcRawValue;

        lastValue = adcRawValue * ADC_REF_VOLTAGE / ADC_MAX;
    }
    catch(std::ios_base::failure&)
    {
        debug << "Error while reading ADC." << endl;
    }
}

/**
 * @brief Returns the last sample from the ADC.
 * @return a const reference to the last measured voltage [V].
 */
const float& Adc::get()
{
    return lastValue;
}
