#ifndef DEF_DRIVERS_GPIO_H
#define DEF_DRIVERS_GPIO_H

#include <string>
#include <fstream>

#include "../lib/peripheral.h"

/**
 * @defgroup GPIO GPIO
 * @brief BeagleBone Black GPIO.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief GPIO enumerations.
 * The full list of identifiers can be found in BeagleBone Black System
 * Reference Manual, in the Expansion headers Pinout.
 * To convert from GPIO name to ID: GPIO*A*_*B* => id = 32*A + B.
 */
enum Gpio_Id
{
    GPIO3_17 = 113, ///< SPI1_CS0, P9_28. SPI_CS0 (FOOT)
    GPIO0_7 = 7, ///< GPIO0_7, P9_42.     SPI_CS1 (FOOT)
    GPIO1_17 = 49, ///< SPI1_CS1, P9_23.  SPI_CS2 (FOOT)
    GPIO3_19 = 115, ///< GPIO3_19, P9_27. SPI_CS3 (FOOT)
    GPIO1_19 = 51, ///< EHRPWM1B, P9_16.  GPIO_VIBR1
    GPIO0_23 = 23, ///< EHRPWM2B, P8_13.  GPIO_VIBR2
    GPIO1_28 = 60, ///< GPIO1_28, P9_12.  GPIO_CRUTCH_TRIGGER
    GPIO2_2 = 66, ///< TIMER4, P8_7.      GPIO_CRUTCH_MODE_UP
    GPIO2_3 = 67, ///< TIMER7, P8_8.      GPIO_CRUTCH_MODE_DOWN / GPIO_LED_R
    GPIO2_5 = 69, ///< TIMER5, P8_9.      GPIO_LED_G
    GPIO2_4 = 68, ///< TIMER6, P8_10.     GPIO_LED_B
    GPIO1_29 = 61, ///< GPIO1_29, P8_26.  GPIO_CRUTCH_EXIT
    GPIO2_22 = 86, ///< GPIO2_22, P8_27.  SPI_CS4
    GPIO2_24 = 88, ///< GPIO2_24, P8_28.  SPI_CS5
    GPIO0_10 = 10, ///< GPIO0_10, P8_31.  SPI_CS6
    GPIO0_11 = 11, ///< GPIO0_11, P8_32.  SPI_CS7
    GPIO1_16 = 48, ///< GPIO1_16, P9_15.
    GPIO3_21 = 117 ///< GPIO3_21, P9_25.
};

/*
 * P9_14 PWM_BEEPER
 * P8_19 PWM_FAN
 */

/**
 * @brief GPIO pin manager.
 * C++ wrapper around the Linux userspace GPIO driver.
 */
class Gpio : public Peripheral
{
public:
    Gpio(Gpio_Id gpioId);
    void setupAsOutput();
    void setupAsInput();
    void setPinState(bool state);
    bool getPinState();

private:
    std::ofstream pinStateOut; ///< Device file to control the pin output state.
    std::ifstream pinStateIn; ///< Device file to control the pin input state.
    Gpio_Id gpioId; ///< GPIO identifier, to build the devices files names.
    bool writtenPinState; ///< Last state written to the pin (output mode).
};

/**
 * @}
 */

#endif
