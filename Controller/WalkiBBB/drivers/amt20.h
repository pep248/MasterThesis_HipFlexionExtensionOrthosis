#ifndef DEF_DRIVERS_AMT20_H
#define DEF_DRIVERS_AMT20_H

#include "../lib/peripheral.h"
#include "spi.h"

/**
 * @defgroup AMT20 AMT20
 * @brief AMT20 absolute encoder.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief Cui AMT20 absolute encoder.
 */
class Amt20 : public Peripheral
{
public:
    Amt20(SpiChannel &spi);
    void update(float dt) override;
    const float& getAngle() const;
    bool setZero();

private:
    uint8_t doSpiTransaction(uint8_t command);

    float angle; ///< Last measured angle, between -180 and 180 deg.
    SpiChannel &spi; ///< The SPI bus the encoder is linked to.
};

/**
 * @}
 */

#endif
