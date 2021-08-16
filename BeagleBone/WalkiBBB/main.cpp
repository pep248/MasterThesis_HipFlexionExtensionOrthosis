#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <regex>
#include <sched.h>
#include <sys/mman.h>

#include "communication.h"
#include "config/config.h"
#include "drivers/motorboard.h"
#include "drivers/hmc5883l.h"
#include "drivers/mpu60x0.h"
#include "drivers/polarhrs.h"
#include "drivers/pwm.h"
#include "drivers/pcf8523.h"
#include "drivers/ledstatusindicator.h"
#include "lib/batterymonitor.h"
#include "lib/debugstream.h"
#include "lib/quadloadcells.h"
#include "lib/stateestimator.h"
#include "lib/syncvar/syncvar.h"
#include "lib/utils.h"
#include "lib/scheduler.h"

using namespace std;
using namespace chrono;
using namespace Utils;

/**
 * @defgroup main main
 * @brief General initialization and main loop.
 * @ingroup Main
 * @{
 */

#ifdef __arm__
#define LOGFILES_DIR "/root/WalkiSoftware/BeagleBone/logs/" ///< Logfiles directory.
#else
#define LOGFILES_DIR "logs/" ///< Logfiles directory.
#endif

#define SAFE_STACK_SIZE (2048*1024) ///< Amount of memory to prefault [B].

#define BATTERY_UPDATE_PERIOD 1.0f ///< [s].
#define DT_INCONSISTENCY_FACTOR 100.0f ///< Maximum dt deviation factor [], to check inconsistency.

volatile bool runMainLoop; ///< Set to zero in order to exit the main loop.

/**
 * @brief Stops the main loop.
 * This function is meant to be called by the ctrl+C signal.
 */
void ctrlcHandler(int)
{
    runMainLoop = false;
}

/**
 * @brief Notifies that the SIGHUP signal was ignored.
 * This function is meant to be used as a signal handler for the SIGHUP signal,
 * in order to ignore it.
 */
void sighupHandler(int)
{
    // Ignore the SIGHUP signal.
    debug << "SIGHUP signal received and ignored." << endl;
}

/**
 * @brief Prefaults the stack, in order to improve the real-time performance.
 */
void prefaultStack()
{
    // Allocate a large array, and write to it to prefault the stack pages.
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
        throw runtime_error("mlockall() failed.");

    uint8_t a[SAFE_STACK_SIZE];
    memset(a, 0, SAFE_STACK_SIZE);
}

/**
 * @brief Performs all the initialization, and runs the main loop.
 * @return Status code, always zero for now.
 */
int main()
{
    // Timestamp clock.
    uint64_t timestamp; // [us].
    timestamp = duration_cast<microseconds>(high_resolution_clock::now()
                                            .time_since_epoch()).count();

    // Create the logfile directory, if it does not already exist.
    if(!directoryExists(LOGFILES_DIR))
        unused(system("mkdir " LOGFILES_DIR));

    // Make the logfile filename, looking at the last logfile number.
    vector<string> logNames = listFiles(LOGFILES_DIR);
    int32_t logNumber;

    for(int i=(signed)logNames.size()-1; i>=0; i--)
    {
        if(!regex_match(logNames[i], regex(string(LOGFILES_PREFIX) + "\\d{"
                                           + to_string(5) + "}_.*")))
        {
            logNames.erase(logNames.begin()+i);
        }
    }

    if(logNames.size() > 0)
    {

        string prevLogName = *max_element(logNames.begin(), logNames.end());
        string prevNumStr = prevLogName.substr(strlen(LOGFILES_PREFIX),
                                               LOGFILES_N_DIGITS);
        logNumber = stoi(prevNumStr) + 1;
    }
    else
        logNumber = 0;

    ostringstream logfilesBaseName;
    logfilesBaseName << LOGFILES_DIR << LOGFILES_PREFIX
                     << setw(LOGFILES_N_DIGITS) << setfill('0') << logNumber;

    // Setup the logging.
    debug.setLogfilePrefix(logfilesBaseName.str());
    debug.setTimestamp(&timestamp);

    // Setup the network communication.
    // In case of error while initializing the Communication object, the error
    // can only be printed on cout and the logfile.
    SyncVarManager syncVars(logfilesBaseName.str());
    unique_ptr<Communication> communication;

    try
    {
        communication.reset(new Communication(syncVars));
        debug.setCommunication(communication.get());
    }
    catch(runtime_error &e)
    {
        // Print the runtime error message.
        debug << "A runtime error occured: " << e.what() << endl;

        // Indicate runtime error.
        return 1;
    }

    // From now on, log to cout, logfile and to client in case of runtime error.
    try
    {
        // Setup the OS for maximum performance and reliability.
    #ifdef __arm__
        execBashCommand("rfkill unblock all");
        execBashCommand("cpufreq-set -g performance");
        prefaultStack();
    #endif

        // Prevent the SIGHUP signal from stopping the program. This signal is
        // sent when the host terminal is closed.
        signal(SIGHUP, sighupHandler);

        // Setup the I2C bus dedicated to sensors.
        I2c i2c(I2C_BUS_SENSORS);
        debug << i2c.printInitResult("I2C bus") << endl;

        // Set the system time from the RTC, if available.
#if USE_RTC
        Pcf8523 rtc(i2c);
        syncVars.add("RTC/", rtc);

        if(rtc.getState() == PeripheralState::ACTIVE)
        {
            communication->setRtc(&rtc);
            Pcf8523::Time rtcTime = rtc.setLinuxTimeFromRtc();

            if(rtcTime.isValid)
            {
                debug << "System time has been set from the RTC: "
                      << Pcf8523::printTime(rtcTime) << "." << endl;
            }
            else
                debug << "The RTC date is invalid and was ignored." << endl;
        }
        else
            debug << "Real-time clock not detected." << endl;
#endif

        // Setup the SPI bus.
        SpiBus spi;
        debug << spi.printInitResult("SPI bus") << endl;

        // Setup input voltage and current measurement.
        BatteryMonitor battery;
        battery.startAutoUpdate(BATTERY_UPDATE_PERIOD);
        debug << battery.printInitResult("Battery sensors") << endl;
        syncVars.add("sensors/battery_monitor/", battery);
        LowPassFilter batteryVoltageFiltered(3.0f, 24.0f);

        // Setup MPU-6050 (accelero + gyro).
        Mpu6050 mpu(i2c, MPU_I2C_ADDRESS_HIGH, MPU_ACCEL_RANGE_4G,
                    MPU_GYRO_RANGE_500DPS, MPU_DLPF_BW_256HZ, "back_imu.conf");
        debug << mpu.printInitResult("Mainboard IMU") << endl;
        syncVars.add("sensors/MPU-6050/", mpu);

        // Setup the heartbeat sensor.
        unique_ptr<PolarHRS> heartBeatSensor;
#ifdef HR_SENSOR
        heartBeatSensor.reset(new HR_SENSOR);

        debug << heartBeatSensor->printInitResult("Heartbeat rate sensor");
        if(heartBeatSensor->getState() == ACTIVE)
        {
            debug << " Battery level: " << heartBeatSensor->getBatteryLevel()
                  << "%." << endl;
        }
        else
            debug << endl;

        syncVars.add("sensors/heartbeat_sensor/", *heartBeatSensor);
#endif

        // Case fan.
        Pwm caseFan(PWM_2A);
        caseFan.setDuty(0.3f);
        syncVars.add(makeSyncVar<float>("fan/duty", "",
                            [&caseFan](){return caseFan.getDuty();},
                            [&caseFan](float d){caseFan.setDuty(d);}, false));

        // Status LED.
        LedStatusIndicator led;
        communication->setLed(&led);

        // Optional GPS infos logging.
#if GPS_LOGGING
        double latitude = 0.0; // [deg].
        double longitude = 0.0; // [deg].
        float elevation = 0.0f; // [m].
        float speed = 0.0f; // [m/s].

        syncVars.add(makeSyncVar("sensors/gps/latitude", "deg", latitude,
                                 VarAccess::READWRITE, true));
        syncVars.add(makeSyncVar("sensors/gps/longitude", "deg", longitude,
                                 VarAccess::READWRITE, true));
        syncVars.add(makeSyncVar("sensors/gps/elevation", "m", elevation,
                                 VarAccess::READWRITE, true));
        syncVars.add(makeSyncVar("sensors/gps/speed", "m/s", speed,
                                 VarAccess::READWRITE, true));
#endif

        // Setup the exoskeleton controller. SelectedController should be
        // defined in the desired controller header file, and then included by
        // config.h.
        PeripheralsSet peripherals = { &spi, &i2c,
                                       heartBeatSensor.get(), &mpu,
                                       &caseFan, &battery, &led };

        unique_ptr<Controller> controller(new SelectedController(peripherals));
        syncVars.add("controller/", controller->getVars());

        // Add a SyncVar to read the CPU temperature.
        syncVars.add(makeSyncVar<float>("sensors/cpu_temperature", "°C",
                                        [=](){return getCpuTemperature();},
                                        nullptr, false));

        //TEMPORARY: speed indicator for eWalk experiments *******************************************************
        float treadmillSpeed;
        syncVars.add(makeSyncVar("test/treadmill_speed", "", treadmillSpeed,
                                 VarAccess::WRITE, true));

        // SyncVar with the logfile number.
        syncVars.add(makeSyncVar("test/logfile_number", "", logNumber,
                                 VarAccess::READ, false));


        // Setup the main loop exit.
        signal(SIGINT, ctrlcHandler);
        runMainLoop = true;
        syncVars.add(makeSyncVar("test/run_main_loop", "", runMainLoop,
                                VarAccess::READWRITE, false));

        // Setup the general shutdown (embedded computer shutdown).
        bool shutdownComputer = false;
        syncVars.add(makeSyncVar<bool>("test/computer_shutdown", "", nullptr,
                                       [&](bool shutdown)
                                       {
                                           if(shutdown)
                                           {
                                               runMainLoop = false;
                                               shutdownComputer = true;
                                           }
                                       }, false));

        // Optional main loop time measurement.
    #if MEASURE_LOOP_TIME
        LoopTimeMonitor loopTimeMonitor(1000);
    #endif

        // Initialization done, the SyncVars list can now be locked, and the
        // value of all the writable variables can be printed.
        syncVars.lockSyncVarsList();

        debug << "Values of the writable SyncVars:" << endl;

        for(auto *sv : syncVars.getVars().getAllOfAccess(VarAccess::READWRITE))
            debug << sv->getName() << ": " << sv->getValueString() << endl;

        debug << endl;

        // Display the estimated maximum logging time.
        float freeSpace = (float)getFilesystemFreeSpace(debug.getLogfileName()); // [B].
        float logDataRate = ((float)syncVars.getLogLineSize()) / MAIN_LOOP_PERIOD; // [B/s].

        debug << setprecision(2)
              << freeSpace / 1024.0f / 1024.0f / 1024.0f
              << " GB are available, can log for approx. "
              << freeSpace / logDataRate / 3600.0f << " hours." << endl;

        // Setup the scheduler.
        Scheduler scheduler(MAIN_LOOP_PERIOD, MAIN_LOOP_PERIOD*0.5f);

        // Main loop.
        uint64_t prevTimestamp;
        timestamp = duration_cast<microseconds>(high_resolution_clock::now()
                                                .time_since_epoch()).count();

        debug << "Initialization done, running controller \""
              << controller->getName() << "\"."<< endl;

        while(runMainLoop)
        {
            // Update the timestamp and compute the dt.
            auto loopStartTime = high_resolution_clock::now();
            prevTimestamp = timestamp;
            timestamp = duration_cast<microseconds>(loopStartTime.time_since_epoch()
                                                    ).count();
            float dt = USEC_TO_SEC(timestamp-prevTimestamp);

            // If the dt seems to be incoherent (may happen when the time is
            // set), use the nominal value to avoid further computation errors.
            if((dt < MAIN_LOOP_PERIOD / DT_INCONSISTENCY_FACTOR) ||
               (dt > MAIN_LOOP_PERIOD * DT_INCONSISTENCY_FACTOR))
            {
                dt = MAIN_LOOP_PERIOD;
            }

            // Main loop period jitter test.
    #if MEASURE_LOOP_TIME
            loopTimeMonitor.update(timestamp);
    #endif

            // Update the modules.
            controller->update(dt);
            led.update(dt);

            //trunkStateEstimator.update(dt);

            syncVars.update(dt);
            syncVars.log(timestamp);
            communication->update(timestamp);

            // Disarm the motorboards and exit the program if the battery level
            // is critical, to minimize the power consumption and preserve the
            // battery.
    #if KILL_IF_BATTERY_CRITICAL
            batteryVoltageFiltered.update(battery.getBatteryLevel(), dt);
            if(batteryVoltageFiltered.get() < 0.0f)
            {
                for(MotorBoard *mb : controller->getMotorBoards())
                    mb->disarmBridges();
                debug << "Battery voltage too low: emergency stop!" << endl;
                break;
            }
    #endif

            // Sleep the thread such as the while loop has a fixed period.
            scheduler.yield();
        }

        // Disable the real-time scheduler, in order to perform the future
        // objects destruction tasks without time constraints. This also allows
        // to execute system commands again.
        scheduler.disable();

        // Shutdown the computer if required.
        if(shutdownComputer)
        {
            debug << "Computer shutdown now." << endl;
            execBashCommand("/sbin/shutdown now -hP");
        }

        // Indicate clean exit.
        led.disableControl();
        debug << endl << "End of program." << endl;

        return 0;
    }
    catch(const runtime_error &e)
    {
        // Print the runtime error message.
        debug << "A runtime error occured: " << e.what() << endl;

        // Indicate runtime error.
        return 1;
    }
}

/**
 * @}
 */
