#ifndef DEF_DRIVERS_ADS7844_H
#define DEF_DRIVERS_ADS7844_H

#include "spi.h"

#include "../lib/vecn.h"
#include "../lib/peripheral.h"

/**
 * @defgroup ADS7844 ADS7844
 * @brief Octal ADC.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Octal ADC TI ADS7844 driver.
 */
class Ads7844 : public Peripheral
{
public:
    Ads7844(SpiBus &spi, const Gpio_Id &chipselect, float refVoltage);
    void update(float dt) override;
    const VecNf<8>& getLastSamples();
    float readChannel(int channel);

private:
    SpiChannel spi; ///< SPI channel for communication with the ADS7844.
    std::vector<uint8_t> txBuffer, ///< Pre-allocated buffer to send data.
                         rxBuffer; ///< Pre-allocated buffer to receive data.
    const float refVoltage; ///< Reference voltage applied to the pin Vref. This corresponds to the maximum voltage that can be measured.
    VecNf<8> lastSamples; ///< Last measured samples [V].
};

/**
 * @}
 */

#endif
