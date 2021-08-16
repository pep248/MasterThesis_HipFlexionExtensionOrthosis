#include "gpio.h"
#include "../lib/utils.h"
#include "../lib/debugstream.h"

#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;
using namespace chrono;

#define GPIO_PATH string("/sys/class/gpio/")

const auto GPIO_LOAD_TIMEOUT = milliseconds(1000);

/**
 * @brief Constructor.
 * @param gpioId identifier of the GPIO.
 */
Gpio::Gpio(Gpio_Id gpioId)
{
    this->gpioId = gpioId;
    writtenPinState = false;

#ifdef __arm__
    // Setup the selected pin as GPIO, if not already the case.
    if(!Utils::directoryExists(GPIO_PATH + "gpio" + to_string(gpioId)))
    {
        try
        {
            Utils::execBashCommand("echo " + to_string(gpioId) + " > " +
                                   GPIO_PATH + "export");
        }
        catch(runtime_error)
        {
            state = PeripheralState::FAULT;
            return;
        }

        // Block until the userland driver files appear.
        auto startTime = steady_clock::now();

        while(!Utils::fileExists(GPIO_PATH + "gpio" + to_string(gpioId) +
                                 "/direction") ||
              !Utils::fileExists(GPIO_PATH + "gpio" + to_string(gpioId) +
                                 "/value"))
        {
            if(steady_clock::now() - startTime < GPIO_LOAD_TIMEOUT)
                this_thread::sleep_for(milliseconds(100));
            else
            {
                state = PeripheralState::FAULT;
                return;
            }
        }
    }

    state = PeripheralState::ACTIVE;
#else
    state = PeripheralState::DISABLED;
#endif
}

/**
 * @brief Setups the GPIO as output.
 */
void Gpio::setupAsOutput()
{
#ifdef __arm__
    // Setup the GPIO as output.
    if(Utils::writeToDeviceFile(GPIO_PATH + "gpio" + to_string(gpioId) +
                                "/direction", "out"))
    {
        // Open the file device for future writes.
        pinStateOut.open(GPIO_PATH + "gpio" + to_string(gpioId) + "/value");
        if(!pinStateOut.is_open())
        {
            state = PeripheralState::FAULT;
            throw runtime_error("GPIO: can't open value file.");
        }

        // Set to low state.
        setPinState(false);
    }
    else
        state = PeripheralState::FAULT;
#endif
}

/**
 * @brief Setups the GPIO as input.
 */
void Gpio::setupAsInput()
{
#ifdef __arm__
    pinStateOut.close();

    // Setup the GPIO as input.
    if(Utils::writeToDeviceFile(GPIO_PATH + "gpio" + to_string(gpioId) +
                                "/direction", "in"))
    {
        string pinStateFilenameIn = GPIO_PATH + "gpio" + to_string(gpioId) + "/value";

        // Open the file.
        pinStateIn.open(pinStateFilenameIn, ios::in);

        if(!pinStateIn.is_open()) // Error.
        {
            state = PeripheralState::FAULT;
            //return 1;
        }
    }
    else
        state = PeripheralState::FAULT;
#endif
}

/**
 * @brief Sets the GPIO state.
 * @param state true for the high state, false for the low state.
 * @remark If the pin is in input mode, this function only sets writtenPinState,
 * which is only used when getPinState() is called, but the GPIO state cannot be
 * read (if in output mode or error).
 */
void Gpio::setPinState(bool state)
{
    try
    {
        if(pinStateOut.is_open() && state != writtenPinState)
            pinStateOut << (state ? 1 : 0) << flush;

        writtenPinState = state;
    }
    catch(std::ios_base::failure&)
    {
        pinStateOut.clear();
        debug << "Error while writing GPIO." << endl;
    }
}

/**
 * @brief Gets the pin state.
 * @return true if the pin is in the high state, false otherwise.
 */
bool Gpio::getPinState()
{
    if(pinStateIn.is_open())
    {
        try
        {
            // Read the ADC value from the file.
            pinStateIn.clear();
            pinStateIn.seekg(0);
            int statenum;
            pinStateIn >> statenum;

            return statenum != 0;
        }
        catch(std::ios_base::failure&)
        {
            debug << "Error while reading GPIO." << endl;
            return writtenPinState;
        }
    }
    else
        return writtenPinState;
}
