#ifndef DEF_DRIVERS_ADC124S01_H
#define DEF_DRIVERS_ADC124S01_H

#include "spi.h"

#include "../lib/peripheral.h"
#include "../lib/vec4.h"

/**
 * @defgroup ADC124S01 ADC124S01
 * @brief Quad ADC.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Quad ADC TI ADC124S01 driver.
 */
class Adc124S01 : public Peripheral
{
public:
    Adc124S01(SpiChannel &spi);
    void update(float dt) override;
    Vec4f getLastSamples();
    float readChannel(int channel);

private:
    SpiChannel &spi; ///< SPI channel for communication with the Adc124S01.
    std::vector<uint8_t> txBuffer, ///< Pre-allocated buffer to send data.
                         rxBuffer; ///< Pre-allocated buffer to receive data.
    Vec4f lastSamples; ///< Last measured samples [V].
};

/**
 * @}
 */

#endif
