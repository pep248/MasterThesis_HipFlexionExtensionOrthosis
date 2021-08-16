#ifndef DEF_DRIVERS_ADS1148_H
#define DEF_DRIVERS_ADS1148_H

#include "spi.h"

#include "../lib/peripheral.h"
#include "../lib/vec4.h"

/**
 * @defgroup ADS1148 ADS1148
 * @brief Quad ADC with programable gain amplifier.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Quad differential ADC TI ADS1148 driver.
 */
class Ads1148 : public Peripheral
{
public:
    /**
     * @brief PGA gains settings.
     */
    typedef enum
    {
        GAIN_1 = 0,
        GAIN_2 = 1,
        GAIN_4 = 2,
        GAIN_8 = 3,
        GAIN_16 = 4,
        GAIN_32 = 5,
        GAIN_64 = 6,
        GAIN_128 = 7
    } Gain;

    /**
     * @brief Sampling frequencies settings.
     */
    typedef enum
    {
        DATARATE_5SPS = 0,
        DATARATE_10SPS = 1,
        DATARATE_20SPS = 2,
        DATARATE_40SPS = 3,
        DATARATE_80SPS = 4,
        DATARATE_160SPS = 5,
        DATARATE_320SPS = 6,
        DATARATE_640SPS = 7,
        DATARATE_1000SPS = 8,
        DATARATE_2000SPS = 9
    } Datarate;

    Ads1148(SpiBus &spi, const Gpio_Id &chipselect, Datarate rate);
    void update(float dt) override;
    void setGain(Gain gain);
    const Vec4f& getLastSamples();

private:
    void setChannel(int channelIndex);
    void writeRegister(uint8_t registerAddress,
                       std::vector<uint8_t> const registerValue);
    void readRegister(uint8_t registerAddress,
                      std::vector<uint8_t> &registerValue);

    SpiChannel spi; ///< SPI channel for communication with the Adc124S01.
    std::vector<uint8_t> txBuffer, ///< Pre-allocated buffer to send data.
                         rxBuffer; ///< Pre-allocated buffer to receive data.
    Vec4f lastSamples; ///< Last measured samples [V].
    float gain;
};

/**
 * @}
 */

#endif
