#ifndef DEF_DRIVERS_PCF8523_H
#define DEF_DRIVERS_PCF8523_H

#include <chrono>

#include "../lib/peripheral.h"
#include "i2c.h"

/**
 * @defgroup PCF8523 PCF8523
 * @brief Real-time clock.
 * @ingroup Drivers
 * @{
 */

/**
 * @brief The PCF8523 class
 */
class Pcf8523 : public Peripheral
{
public:
    /**
     * @brief Time structure, as represented by the PCF8523.
     */
    struct Time
    {
        bool isValid; ///< true if the clock integrity is garanteed, false otherwise.
        int year, ///< 4 digits year.
            month, ///< Month [1-12].
            weekDay, ///< Week day [1-7].
            day, ///< Day [1-31].
            hours, ///< Hours [0-23].
            minutes, ///< Minutes [0-59].
            seconds; ///< Seconds [0-59].
    };

    Pcf8523(I2c &i2c);

    void update(float dt) override;

    Time getRtcTime();
    void setRtcTime(Time time);

    Time setLinuxTimeFromRtc();
    Time setRtcFromLinuxTime();

    static std::string printTime(Time time);

private:
    I2c &i2c; ///< I2C bus to communicate with the PCF8523.
};

/**
 * @}
 */

#endif
