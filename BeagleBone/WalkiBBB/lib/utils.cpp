#include "utils.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <numeric>
#include <dirent.h>
#include <sys/statvfs.h>
#include "debugstream.h"
#include "vec2.h"

#include "../config/config.h"

using namespace std;
using namespace chrono;

#if DEBIAN_VERSION == 7
const string SLOTS_PATH("/sys/devices/bone_capemgr.*/slots");
#elif DEBIAN_VERSION == 8
const string SLOTS_PATH("/sys/devices/platform/bone_capemgr/slots");
#endif
const auto OVERLAY_LOAD_TIMEOUT = chrono::milliseconds(5000);
const auto DEVICE_FILE_WRITE_TIMEOUT = chrono::milliseconds(5000);

/**
 * @brief Executes a bash command.
 * @param command the command to execute with "/bin/bash -c".
 * @param timeout maximum time for the operation to complete [s]. If the process
 * did not exit within the specified timeout, it will be terminated. To disable
 * the timeout, use a value equal or less than zero.
 * @note This function blocks until the command has ended.
 */
void Utils::execBashCommand(std::string command, float timeout)
{
    if(timeout > 0.0f)
        command = "timeout " + to_string(timeout) + "s " + command;

    string shCommand = "/bin/bash -c \"" + command + "\"";
    shCommand += " 2>/dev/null"; // Suppress stderr output.

    FILE *fp = popen(shCommand.c_str(), "r");

    if(fp != nullptr)
        pclose(fp);
}

/**
 * @brief Executes a bash command, and returns its output.
 * @param command the command to execute with "/bin/bash -c".
 * @param timeout maximum time for the operation to complete [s]. If the process
 * did not exit within the specified timeout, it will be terminated. To disable
 * the timeout, use a value equal or less than zero.
 * @return the command stdout output. It will be an empty string in case of
 * failure.
 * @note the command stderr output is ignored.
 * @note This function blocks until the command has ended.
 */
std::string Utils::execBashCommandWithResult(std::string command, float timeout)
{
    if(timeout > 0.0f)
        command = "timeout " + to_string(timeout) + "s " + command;

    string shCommand = "/bin/bash -c \"" + command + "\"";
    shCommand += " 2>/dev/null"; // Suppress stderr output.

    FILE *fp = popen(shCommand.c_str(), "r");

    if(fp == nullptr)
        return ""; // Error, return an empty string.
    else
    {
        char buf[2];
        string result;

        while(fgets(buf, 2, fp))
            result += buf[0];

        pclose(fp);

        return result;
    }
}

/**
 * @brief Loads a device tree overlay.
 * Loads an overlay, if it is not already loaded. If the fileToCheck argument is
 * provided, then this function block until the file with the given filename
 * appears. If wait time reaches OVERLAY_LOAD_TIMEOUT, then an exception is
 * thrown.
 * @param overlayName the name of the overlay to load.
 * @param fileToCheck filename of the optionnal file to check. If the string is
 * empty, or this argument missing, then no check is performed, and the function
 * will return immediately.
 */
void Utils::loadOverlay(std::string overlayName, std::string fileToCheck)
{
    // Load the overlay only if it was not already.
    string overlaysStr = execBashCommandWithResult("cat " + SLOTS_PATH);

    if(overlaysStr.find(overlayName) == string::npos)
        execBashCommand("echo " + overlayName + " > " + SLOTS_PATH);

    // Block until the overlay is loaded.
    if(!fileToCheck.empty())
    {
        auto startTime = steady_clock::now();

        while(!fileExists(fileToCheck))
        {
            if(steady_clock::now() - startTime < OVERLAY_LOAD_TIMEOUT)
                this_thread::sleep_for(milliseconds(100));
            else
            {
                throw runtime_error("Could not load the overlay " +
                                    overlayName + ": timeout.");
            }
        }
    }
}

/**
 * @brief Checks if a file exists.
 * @param path filepath to check.
 * @return true if the file exists, false otherwise.
 */
bool Utils::fileExists(std::string path)
{
    string s = execBashCommandWithResult("[ -e \"" + path + "\" ] && echo 1");

    return (s.find("1") != string::npos);
}

/**
 * @brief Checks if a directory exists.
 * @param path directory path to check.
 * @return true if the directory exists, false otherwise.
 */
bool Utils::directoryExists(std::string path)
{
    string s = execBashCommandWithResult("[ -d \"" + path + "\" ] && echo 1");

    return (s.find("1") != string::npos);
}

/**
 * @brief Return the list of the files in the given directory.
 * @param path directory filepath.
 * @return a vector with the filenames of the files in the given directory.
 */
std::vector<std::string> Utils::listFiles(std::string path)
{
    DIR *directory;
    dirent *file;

    vector<string> filenames;

    directory = opendir(path.c_str());
    if(directory != nullptr)
    {
        while((file = readdir(directory)) != nullptr)
        {
            string filename = file->d_name;

            if(filename != "." && filename != "..")
                filenames.push_back(filename);
        }

        closedir(directory);
    }

    return filenames;
}

/**
 * @brief Splits a string with the given separator.
 * @param str the string to split.
 * @param separator the separator for splitting.
 * @param keepEmpty if true, empty words will be kept, if false, they will be
 * discarded.
 * @return The list of splitted words.
 */
vector<string> Utils::split(const std::string &str, char separator,
                            bool keepEmpty)
{
    vector<string> parts;
    istringstream iss(str);
    string part;

    while(getline(iss, part, separator))
    {
        if(keepEmpty || !part.empty())
            parts.push_back(part);
    }

    return parts;
}

/**
 * @brief Prints a date to a string.
 * @param date date to convert to a string.
 * @param format format description, using the same syntax as strftime().
 * @return The date printed as a string.
 */
std::string Utils::dateToStr(std::chrono::time_point<std::chrono::system_clock> date,
                             std::string format)
{
    auto now = chrono::system_clock::to_time_t(date);

    std::tm* nowTm = localtime(&now);
    char strBuff[100];
    strftime(strBuff, 100, format.c_str(), nowTm);

    return string(strBuff);
}

/**
 * @brief Evaluates a "piecewise polynomial" at the given point.
 * Evaluates a polynom c[0]*x^N + c[1]*x^(N-1) + ... + c[N-1]*x + c[N], at a
 * given point x. The polynom coefficents can change, depending on which part
 * (separated by the given breaks) x belongs to.
 * @param coefsMatrix vector of vectors of polynomial coefficients. The
 * coefficients are in the descending order of the power of x, so
 * coefsMatrix[*].back() is the constant.
 * @param breaks vector of the "breaks" values, in the ascending order. Its
 * size should be coefsMatrix.size()+1.
 * @param x the evaluation point.
 * @return the value of the piecewise polynomial, evaluated at the point x.
 */
float Utils::evalPiecewisePolynomial(vector<vector<float>> coefsMatrix,
                                     vector<float> breaks, float x)
{
    // Determine the right coefficients set by finding the correct break.
    int coefsSetIndex;

    if(x < breaks[0]) // x is out-of-range (too small) => clamp to the mininum break value.
        coefsSetIndex = 0;
    else if(x > breaks.back()) // x is out-of-range (too large) => clamp to the maximum break value.
        coefsSetIndex = breaks.size()-2;
    else // x is in the given range.
    {
        coefsSetIndex = 0;

        for(unsigned int i=1; i<breaks.size(); i++)
        {
            if(breaks[i] >= x)
            {
                coefsSetIndex = i-1;
                break;
            }
        }
    }

    x -= breaks[coefsSetIndex];
    vector<float> &coefs = coefsMatrix[coefsSetIndex];

    // Evaluate the polynomial.
    return evalPolynomial(coefs, x);
}

/**
 * @brief Interpolates a points array at the given X value.
 * @param x1 X coordinates of the points array.
 * @param y1 Y coordinates of the points array.
 * @param x2 X coordinate of the interpolated point.
 * @return Y coordinate of the interpolated point.
 */
float Utils::linearInterpolation(vector<float> x1, vector<float> y1, float x2)
{
    // Check the input arrays.
    if(x1.size() < 2 || x1.size() != y1.size())
        throw runtime_error("linearInterpolation(): bad vectors size.");

    // Clamp the value if it is out of range.
    x2 = clamp(x2, x1.front(), x1.back());

    // Determine the relevant segment.
    // TODO: dichotomic search?
    int nSegments = x1.size() - 1;
    int segmentIndex = 0;
    while (segmentIndex < nSegments - 1 && x2 > x1[segmentIndex + 1])
        segmentIndex++;

    // Determine the weights for the linear interpolation.
    float t = (x2 - x1[segmentIndex]) /
              (x1[segmentIndex + 1] - x1[segmentIndex]);

    // Evaluate the interpolated value.
    return y1[segmentIndex] * (1.0f - t) + y1[segmentIndex + 1] * t;
}

/**
 * @brief Constructor.
 * @param loopTimeSamplesCount number of samples to accumulate before computing
 * the average period.
 */
Utils::LoopTimeMonitor::LoopTimeMonitor(int loopTimeSamplesCount) :
    loopTimeSamplesCount(loopTimeSamplesCount)
{
    loopTimeCounter = -1000; // Intial warmup period without measuring.
    loopTimeSamples.resize(loopTimeSamplesCount);
    prevTimestamp = 0;
}

/**
 * @brief Adds a new period sample.
 * Adds a new period sample to the buffer. If loopTimeSamplesCount samples have
 * been collected, the average loop time will be computed and displayed.
 * @param timestamp current timestamp [us].
 */
void Utils::LoopTimeMonitor::update(uint64_t timestamp)
{
    if(loopTimeCounter >= 0 && loopTimeCounter < loopTimeSamplesCount)
        loopTimeSamples[loopTimeCounter] = USEC_TO_SEC(timestamp-prevTimestamp);
    else if(loopTimeCounter == loopTimeSamplesCount)
    {
        float sum = accumulate(&loopTimeSamples[0],
                               &loopTimeSamples[loopTimeSamplesCount], 0.0f);
        float mean = sum / (float)loopTimeSamplesCount;

        float stdDev = 0.0f;

        for(int i=0; i<loopTimeSamplesCount; i++)
            stdDev += pow(loopTimeSamples[i]-mean, 2.0f);

        stdDev = sqrt(stdDev / (float)(loopTimeSamplesCount+1));

        cout << "Mean: " << mean << " s, std: " << stdDev << " s." << endl;

        loopTimeCounter = -100;
    }

    prevTimestamp = timestamp;

    loopTimeCounter++;
}

/**
 * @brief Set the value of a file, with check and auto-retrying.
 * Try to write the desired value to the specified file, retrying until the
 * file has the desired value, or the maximal time has been reached (timeout).
 * @param path path of the file to write.
 * @param value desired value of the file.
 * @return true if the file could be written (and checked successfully), false
 * if the operation failed.
 */
bool Utils::writeToDeviceFile(std::string path, std::string value)
{
    auto startTime = steady_clock::now();

    // Try to write to the device until its content matches the desired value,
    // or until timeout.
    while(steady_clock::now() - startTime < DEVICE_FILE_WRITE_TIMEOUT)
    {
        if(fileExists(path))
        {
            // Write to the file.
            string command = "echo " + value + " > " + path;
            execBashCommand(command);

            // Read its actual value.
            string fileValue = execBashCommandWithResult("cat " + path);

            if(fileValue.back() == '\n')
                fileValue.pop_back();

            if(fileValue == value)
                return true;
        }

        this_thread::sleep_for(milliseconds(100));
    }

    return false;
}

/**
 * @brief Gets the amount of free space on the filesystem.
 * @param filepath filepath of any file on the desired filesystem.
 * @return the amount of free space [B], or -1 if this method failed.
 */
uint64_t Utils::getFilesystemFreeSpace(std::string filepath)
{
    struct statvfs fsInfos;
    int errorCode = statvfs(filepath.c_str(), &fsInfos);

    if(errorCode == 0)
        return ((uint64_t)fsInfos.f_bavail) * (uint64_t)fsInfos.f_bsize;
    else
        return -1;
}

/**
 * @brief Gets the CPU temperature.
 * @return the CPU temperature [°C], or 0 in case of failure.
 */
float Utils::getCpuTemperature()
{
    string str = execBashCommandWithResult("cat /sys/class/hwmon/hwmon0/device/"
                                           "temp1_input");

    try
    {
        return stof(str) / 1000.0f;
    }
    catch(invalid_argument&)
    {
        return 0.0f;
    }
}

/**
 * @brief Constructor.
 * Creates a DurationClock object, and starts counting time.
 * @param tag a text string to be displayed by printDuration(), for easier
 * identification.
 * @param threshold minimum duration time to be displayed. With a value of zero,
 * the message will always be displayed.
 */
Utils::DurationClock::DurationClock(string tag, milliseconds threshold)
{
    this->tag = tag;
    this->threshold = threshold;
    startTime = high_resolution_clock::now();
}

/**
 * @brief Prints the duration since the object creation.
 * @remark If the measured time is lower than the threshold given in the
 * constructor, printDuration() will not display a message.
 */
void Utils::DurationClock::printDuration()
{
    auto duration = high_resolution_clock::now() - startTime;

    if(duration >= threshold)
    {
        debug << tag << ": " << duration_cast<milliseconds>(duration).count()
              << endl;
    }
}

/**
 * @brief Computes the intersection points of two circles.
 * Adapted from https://stackoverflow.com/a/3349134/3501166.
 * @param p0 position of the center of the first circle.
 * @param r0 radius of the first circle.
 * @param p1 position of the center of the second circle.
 * @param r1 radius of the second circle.
 * @param intersect0 will be set to the first intersection position.
 * @param intersect1 will be set to the second intersection position.
 * @return true if the two circles actually intersect, false otherwise.
 */
bool Utils::circlesIntersect(Vec2<float> p0, float r0, Vec2<float> p1, float r1,
                             Vec2<float> &intersect0, Vec2<float> &intersect1)
{
    // Check that the circles actually intersect.
    float dist = (p0 - p1).length();

    if(dist >= r0 + r1 || // Circles too far: no intersection.
       dist <= fabs(r0 - r1)) // One circle fully contained into the other: no intersection.
    {
        return false;
    }

    // Compute the two intersection points.
    float a = (r0*r0 - r1*r1 + dist*dist) / (2.0f*dist);
    float h = sqrtf(r0*r0 - a*a);
    Vec2f p2 = p0 + a * (p1-p0) / dist;

    Vec2f hv( h * (p1.y-p0.y) / dist,
             -h * (p1.x-p0.x) / dist);

    intersect0 = p2 + hv;
    intersect1 = p2 - hv;

    return true;
}

/**
 * @brief Set the variable to reach the defined target, with limited rate.
 * @param currentValue reference to the current value. It will be written by the
 * new value.
 * @param targetValue target value to reach.
 * @param maxRate maximum rate to reach the target value. The currentValue value
 * will not change more than maxRate * dt.
 * @param dt time elapsed since the last call to this function [s].
 * @return
 */
void Utils::reachValueWithLimitedRate(float &currentValue, float targetValue,
                                      float maxRate, float dt)
{
    if(currentValue < targetValue)
    {
        currentValue += maxRate * dt;
        if(currentValue > targetValue)
            currentValue = targetValue;
    }
    else if(currentValue > targetValue)
    {
        currentValue -= maxRate * dt;
        if(currentValue < targetValue)
            currentValue = targetValue;
    }
}
