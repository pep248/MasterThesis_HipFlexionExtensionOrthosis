#include "pcf8523.h"

#include <sstream>

#include "../lib/debugstream.h"
#include "../lib/utils.h"

using namespace std;
using namespace chrono;
using namespace Utils;

#define SLAVE_ADDRESS 0x68

/**
 * @brief Constructor.
 * @param i2c I2C bus to which the PCB8523 is connected to.
 */
Pcf8523::Pcf8523(I2c &i2c) : i2c(i2c)
{
    // Create the SyncVars.
    syncVars.push_back(makeSyncVar<bool>("set_rtc_time", "", nullptr,
    [=](bool set)
    {
        if(set)
        {
            Time time = setRtcFromLinuxTime();

            if(state == PeripheralState::ACTIVE)
            {
                debug << "Set the RTC chip time to " << printTime(time) << "."
                      << endl;
            }
            else
                debug << "Could not set the RTC chip time" << endl;
        }
    }, false));

    // Test the device.
    if(i2c.getState() == PeripheralState::ACTIVE)
    {
        state = PeripheralState::ACTIVE;
        getRtcTime();
    }
    else
        state = PeripheralState::DISABLED;
}

/**
 * @brief Updates the driver.
 * This method actually has no effect.
 */
void Pcf8523::update(float)
{

}

/**
 * @brief Acquires the time from the RTC.
 * @return the time given by the PCB8523, in a Time structure.
 * @remark If the method failed to communicate with the RTC chip, the state will
 * change to PeripheralState::FAULT, and the Time returned will be invalid.
 */
Pcf8523::Time Pcf8523::getRtcTime()
{
    Time time;
    time.isValid = true;

    try
    {
        // Acquire the time data from the RTC.
        vector<uint8_t> timeData = i2c.readRegister(SLAVE_ADDRESS, 0x3, 7);

        // Validity.
        if(timeData[0] & 0x80)
            time.isValid = false;

        // Seconds.
        int secondsTens = (timeData[0] & 0x70) >> 4;
        int secondsUnit = (timeData[0] & 0xf);
        time.seconds = secondsTens * 10 + secondsUnit;

        // Minutes.
        int minutesTens = (timeData[1] & 0x70) >> 4;
        int minutesUnit = (timeData[1] & 0xf);
        time.minutes = minutesTens * 10 + minutesUnit;

        // Hours.
        int hoursTens = (timeData[2] & 0x30) >> 4;
        int hoursUnit = (timeData[2] & 0xf);
        time.hours = hoursTens * 10 + hoursUnit;

        // Days.
        int daysTens = (timeData[3] & 0x30) >> 4;
        int daysUnit = (timeData[3] & 0xf);
        time.day = daysTens * 10 + daysUnit;

        // Weekday.
        time.weekDay = timeData[4] & 0x7;

        // Month.
        int monthTens = (timeData[5] & (1<<4)) >> 4;
        int monthUnit = (timeData[5] & 0xf);
        time.month = monthTens * 10 + monthUnit;

        // Year.
        int yearTens = (timeData[6] & 0xf0) >> 4;
        int yearUnit = (timeData[6] & 0xf);
        time.year = 2000 + yearTens * 10 + yearUnit;
    }
    catch(runtime_error&)
    {
        state = PeripheralState::FAULT;
        time.isValid = false;
    }

    return time;
}

/**
 * @brief Sets the time on the RTC.
 * @param time time to set the RTC.
 */
void Pcf8523::setRtcTime(Time time)
{
    vector<uint8_t> timeData(7);

    timeData[0] = ((time.seconds / 10) << 4) |
                  (time.seconds % 10);
    timeData[1] = ((time.minutes / 10) << 4) |
                  (time.minutes % 10);
    timeData[2] = (((time.hours / 10) & 0x3) << 4) |
                  (time.hours % 10);
    timeData[3] = (((time.day / 10) & 0x3) << 4) |
                  (time.day % 10);
    timeData[4] = (time.weekDay & 0x7);
    timeData[5] = (((time.month / 10) & 0x1) << 4) |
                  (time.month % 10);
    timeData[6] = ((time.year % 100 / 10) << 4) |
                  (time.year % 10);

    try
    {
        i2c.writeRegister(SLAVE_ADDRESS, 0x3, timeData);
    }
    catch(runtime_error&)
    {
        state = PeripheralState::FAULT;
        debug << "Pcf8523::setRtcTime(): I2C error." << endl;
    }
}

/**
 * @brief Sets the Linux system time from the RTC time.
 * @return The time the Linux has been set to.
 * @warning The Linux time will not be set if the date reported by the RTC is
 * bogus.
 */
Pcf8523::Time Pcf8523::setLinuxTimeFromRtc()
{
    // Get the time from the RTC clock.
    Time time = getRtcTime();

    if(time.isValid)
    {
        // Execute the "date" Unix command to set the date.
        ostringstream oss;
        oss << time.year << "-" << time.month << "-" << time.day << " "
            << time.hours << ":" << time.minutes << ":" << time.seconds;

        execBashCommand("date -s '" + oss.str() + "'");
    }

    return time;
}

/**
 * @brief Sets the time on the RTC, from the current Linux system time.
 * @return The time the RTC chip has been set to.
 */
Pcf8523::Time Pcf8523::setRtcFromLinuxTime()
{
    Time time;

    // Get the time from the operating system.
    time_t now = system_clock::to_time_t(system_clock::now());
    struct tm *timeParts = localtime(&now);

    time.year = timeParts->tm_year + 1900;
    time.month = timeParts->tm_mon + 1;
    time.day = timeParts->tm_mday;
    time.hours = timeParts->tm_hour;
    time.minutes = timeParts->tm_min;
    time.seconds = timeParts->tm_sec;
    time.weekDay = timeParts->tm_wday;

    // Set the time of the RTC.
    setRtcTime(time);

    return time;
}

/**
 * @brief Generates a human-readable string to describe the given Time.
 * @param time Time structure to print.
 * @return A string in the format "year-month-day (weekday)
 * hours:minutes:seconds".
 */
string Pcf8523::printTime(Pcf8523::Time time)
{
    ostringstream oss;
    oss << time.year << "-" << time.month << "-" << time.day << " "
        << "(" << time.weekDay << ") "
        << time.hours << ":" << time.minutes << ":" << time.seconds;

    return oss.str();
}
