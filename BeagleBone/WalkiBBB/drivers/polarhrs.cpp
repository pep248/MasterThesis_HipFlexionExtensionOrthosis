#include "polarhrs.h"
#include "../lib/utils.h"

#include "../lib/debugstream.h"

using namespace std;
using namespace chrono;
using namespace Utils;

#define OPERATION_TIMEMOUT_S 3.0f // [s].
#define OPERATION_TIMEOUT duration<float>(OPERATION_TIMEMOUT_S) // [s].
#define RR_VALUES_PRESENT_BIT (1<<4)

/**
 * @brief Constructor.
 * @param deviceMacAddress MAC address of the device. It should be six numbers
 * in hexadecimal format (two digits), separated by ":". The given string can
 * also be empty, in this case the driver will not try to connect to the sensor,
 * and the Peripheral will be in DISABLED state.
 * @param acquireBatteryCommand gatttool command to acquire the battery level.
 * @param startHrAcquisitionCommand gatttool command to start streaming the HR
 * values and listen to them.
 * @param stopHrAcquisitionCommand gatttool command to stop streaming the HR
 * values.
 * @param hrDataFormat format of the HR packets received from the HR sensor, to
 * be used by sscanf().
 */
PolarHRS::PolarHRS(std::string deviceMacAddress,
                   std::string acquireBatteryCommand,
                   std::string startHrAcquisitionCommand,
                   std::string stopHrAcquisitionCommand,
                   std::string hrDataFormat) :
    acquireBatteryCommand(acquireBatteryCommand),
    startHrAcquisitionCommand(startHrAcquisitionCommand),
    stopHrAcquisitionCommand(stopHrAcquisitionCommand),
    hrDataFormat(hrDataFormat)
{
    debug<<"Connecting to Polar Heartrate Sensor...\n";
    lastHeartBeatRate = 0.0f;
    file = nullptr;
    acquisitionThread = nullptr;

    // If the given MAC address is empty, do not try to establish a link with
    // the sensor.
    if(deviceMacAddress.empty())
    {
        state = PeripheralState::DISABLED;
        return;
    }

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar("heartbeat_rate", "bpm", lastHeartBeatRate,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("detection_status", "", sensorStatus,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("rr_interval_1", "s/1024", rrIntervals[0],
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("rr_interval_2", "s/1024", rrIntervals[1],
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("rr_interval_3", "s/1024", rrIntervals[2],
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("rr_interval_4", "s/1024", rrIntervals[3],
                                   VarAccess::READ, true));

    // Enable the Bluetooth.
    execBashCommand("hciconfig hci0 up");

    string hciConfigDevOutput;
    auto startTime = steady_clock::now();

    while(hciConfigDevOutput.find("hci0") == string::npos)
    {
        hciConfigDevOutput = execBashCommandWithResult("hciconfig dev");
        this_thread::sleep_for(milliseconds(100));

        if(steady_clock::now() - startTime > OPERATION_TIMEOUT)
        {
            state = PeripheralState::FAULT;
            return;
        }
    }

    // Acquire the battery level.
    string batteryStr = execBashCommandWithResult(acquireBatteryCommand,
                                                  OPERATION_TIMEMOUT_S);

    uint8_t rawBatteryLevel;
    int n = sscanf(batteryStr.c_str(), "Characteristic value/descriptor: %hhx",
                   &rawBatteryLevel);

    if(n == 1)
        initBatteryLevel = (float)rawBatteryLevel;
    else
    {
        state = PeripheralState::FAULT;
        return;
    }

    // Start the continuous update thread.
    acquisitionThread = new thread(&PolarH7::acquireContinuously, this);

    // Check that the sensor works.
    startTime = steady_clock::now();

    while(state != PeripheralState::ACTIVE)
    {
        this_thread::sleep_for(milliseconds(100));

        if(steady_clock::now() - startTime > OPERATION_TIMEOUT)
        {
            state = PeripheralState::FAULT;
            return;
        }
    }
}

/**
 * @brief Destructor.
 */
PolarHRS::~PolarHRS()
{
    // Stop the acquisition thread.
    if(acquisitionThread != nullptr)
    {
        execBashCommand("killall -SIGINT gatttool");

        keepAcquiring = false;
        acquisitionThread->join();
        delete acquisitionThread;
        acquisitionThread = nullptr;
    }
}

/**
 * @brief Gets the heartbeat rate.
 * @return the latest value of the heartbeat rate received from the sensor
 * [bpm].
 */
float PolarHRS::getHeartBeatRate()
{
    return lastHeartBeatRate;
}

/**
 * @brief Gets the battery level at the time of the creation of the object.
 * @return the battery level [%], or -1 if it could not be retrieved.
 */
float PolarHRS::getBatteryLevel()
{
    return initBatteryLevel;
}

/**
 * @brief Continuously listen to the device and interprets its messages.
 * This function blocks until keepAcquiring is set to false, and should be run
 * in its own thread.
 */
void PolarHRS::acquireContinuously()
{
    keepAcquiring = true;

    // Setup the sensor to send HR data periodically.
    FILE *fp = popen(("trap '' TERM HUP; " + startHrAcquisitionCommand +
                      " 2>/dev/null").c_str(), "r");

    if(fp == nullptr)
    {
        state = PeripheralState::FAULT;
        return;
    }

    // Read the output line per line.
    char line[256];
    uint8_t rawHeartBeatRate;

    if(fp == nullptr || state == PeripheralState::FAULT)
        return;

    while(keepAcquiring)
    {
        if(fgets(line, sizeof(line)-1, fp) == nullptr)
        {
            debug << "PolarHRS: read error." << endl;

            // Read error, stop the acquisition.
            state = PeripheralState::FAULT;
            break;
        }
        else
        {
            uint8_t rawRr[8];

            int n = sscanf(line, hrDataFormat.c_str(),
                           &sensorStatus, &rawHeartBeatRate,
                           &rawRr[0], &rawRr[1], &rawRr[2], &rawRr[3],
                           &rawRr[4], &rawRr[5], &rawRr[6], &rawRr[7]);

            for(int i=0; i<4; i++)
                rrIntervals[i] = UINT16_MAX;

            if(n >= 4)
                rrIntervals[0] = (((uint16_t)rawRr[1])<<8) | ((uint16_t)rawRr[0]);
            if(n >= 6)
                rrIntervals[1] = (((uint16_t)rawRr[3])<<8) | ((uint16_t)rawRr[2]);
            if(n >= 8)
                rrIntervals[2] = (((uint16_t)rawRr[5])<<8) | ((uint16_t)rawRr[4]);
            if(n >= 10)
                rrIntervals[3] = (((uint16_t)rawRr[7])<<8) | ((uint16_t)rawRr[6]);

            if(n >= 2)
            {
                state = PeripheralState::ACTIVE;
                lastHeartBeatRate = (float)rawHeartBeatRate;
            }
            else
                lastHeartBeatRate = 0.0f;
        }
    }

    // Close the pipe.
    pclose(fp);

    // Disable the continuous sending of the heartbeat rate.
    execBashCommand(stopHrAcquisitionCommand, OPERATION_TIMEMOUT_S);
}

/**
 * @brief Updates the sensor status.
 * This method exist only to be able to inherit from Peripheral, but is actually
 * not used, and does nothing.
 */
void PolarHRS::update(float)
{
    // Unused.
}

/**
 * @brief Constructor.
 * @param deviceMacAddress MAC address of the device. It should be six numbers
 * in hexadecimal format (two digits), separated by ":". The given string can
 * also be empty, in this case the driver will not try to connect to the sensor,
 * and the Peripheral will be in DISABLED state.
 */
PolarH7::PolarH7(string deviceMacAddress) :
    PolarHRS(deviceMacAddress,
             "gatttool -b " + deviceMacAddress + " --char-read --handle=0x0027",
             "gatttool -b " + deviceMacAddress
                + " --char-write-req --handle=0x0013 --value=0100 --listen",
             "gatttool -b " + deviceMacAddress
                + " --char-write-req --handle=0x0013 --value=0000",
             "Notification handle = 0x0012 value: %hhx %hhx %lx %lx %lx %lx")
{

}

/**
 * @brief Constructor.
 * @param deviceMacAddress MAC address of the device. It should be six numbers
 * in hexadecimal format (two digits), separated by ":". The given string can
 * also be empty, in this case the driver will not try to connect to the sensor,
 * and the Peripheral will be in DISABLED state.
 */
PolarH10::PolarH10(string deviceMacAddress) :
    PolarHRS(deviceMacAddress,
             "gatttool -t random -b " + deviceMacAddress
                + " --char-read --handle=0x0041",
             "gatttool -t random -b " + deviceMacAddress
                + " --char-write-req --handle=0x0011 --value=0100 --listen",
             "gatttool -t random -b " + deviceMacAddress
                + " --char-write-req --handle=0x0011 --value=0000",
             "Notification handle = 0x0010 value: %hhx %hhx %x %x %x %x %x %x %x %x")
{

}
