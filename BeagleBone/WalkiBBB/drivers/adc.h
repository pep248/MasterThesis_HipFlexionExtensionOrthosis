#ifndef DEF_DRIVERS_ADC_H
#define DEF_DRIVERS_ADC_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../lib/peripheral.h"

/**
 * @defgroup ADC ADC
 * @brief BeagleBone Black integrated ADC.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief ADC channels enumeration.
 */
enum AdcChannel
{
    ADC_BOARD_VOLTAGE = 0,
    ADC_BOARD_CURRENT = 1,
    ADC_AIN2 = 2,
    ADC_AIN3 = 3,
    ADC_AIN4 = 4,
    ADC_AIN5 = 5,
    ADC_AIN6 = 6
};

#define ADC_BOARD_VOLTAGE_FACTOR 31.0f ///< Input voltage division factor, because of the voltage divider of the mainboard: 30 kOhm / 1kOhm [].
//#define ADC_BOARD_CURRENT_FACTOR (1.0f/0.055f * (1.0f/(0.91f+1.0f))) ///< ACS711ELCTR-25AB-T sensitivity at 3.3V and 0.91kOhm / 1kOhm voltage divider [A/V].
#define ADC_BOARD_CURRENT_FACTOR (1.0f/0.0562f * (1.0f/(0.91f+1.0f))) ///< ADC voltage to current conversion factor, with ACS711ELCTR-25AB-T sensitivity at 3.37V and 0.91kOhm / 1kOhm voltage divider [A/V].

/**
 * @brief ADC channel manager.
 * C++ wrapper around the Linux userspace ADC driver.
 */
class Adc : public Peripheral
{
public:
    Adc(AdcChannel channel);
    void update(float dt) override;
    const float& get();

private:
    std::string filename; ///< Filename of the device file.
    float lastValue; ///< Last measured voltage [V].
    std::ifstream file; ///< Driver file.
};

/**
 * @}
 */

#endif
