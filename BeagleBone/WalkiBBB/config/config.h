#ifndef DEF_CONFIG_H
#define DEF_CONFIG_H

// Measure loop time periodically.
#define MEASURE_LOOP_TIME false

// Controller choice: #include line of the Controller header file.
//#include "../controllers/test/testcontroller.h"
//#include "../controllers/echoing/echoing.h"
//#include "../controllers/active_captur/active_captur.h"

#include "../controllers/ewalk/ewalkreactivecontroller.h"
//#include "../controllers/ewalk/ewalkhipphasebasedcontroller.h"
//#include "../controllers/ewalk/ewalktimebasedtorqueprofile.h"

// Linux Debian version.
#define DEBIAN_VERSION 8

// Description of the optional heartbeat rate sensor. It should be defined as
// the call to the constructor, with its MAC address as the argument.
// If not defined, the object will not be created.
//#define HR_SENSOR PolarH7("00:00:00:00:00:00")
#define HR_SENSOR PolarH10("D3:CE:E5:75:F3:9D")

// Enable GPS logging.
#define GPS_LOGGING false

// Kill the program if the battery level is critical.
#define KILL_IF_BATTERY_CRITICAL false

// Try to get the current date/time from the real-time clock (RTC).
#define USE_RTC false

#endif
