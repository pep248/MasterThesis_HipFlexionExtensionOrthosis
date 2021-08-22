#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <initializer_list>
#include <algorithm>

/**
 * @defgroup Utils Utils
 * @brief Miscellaneous utility functions.
 * @ingroup Lib
 * @{
 */

template<typename T> class Vec2;

template<typename T> void unused(T &&) { } ///< Prevents "unused variable" compiler warning.

#define PI 3.14159265359f ///< Pi number constant.
#define DEG_TO_RAD(x) ((x)*PI/180.0f) ///< Converts a angle in degrees to radians.
#define RAD_TO_DEG(x) ((x)/PI*180.0f) ///< Converts a angle in radians to degrees.
#define USEC_TO_SEC(x) (((float)(x))/1000000.0f) ///< Converts a time in microseconds to seconds. The given number can be an integer and will be cast to float.
#define SEC_TO_USEC(x) (uint64_t)((x)*1000000.0f) ///< Converts a time in seconds to microseconds. The result is cast to uint64_t.
#define NSEC_TO_SEC(x) (((float)(x))/1000000000.0f) ///< Converts a time in nanoseconds to seconds. The given number can be an integer and will be cast to float.
#define SEC_TO_NSEC(x) (uint64_t)((x)*1000000000.0f) ///< Converts a time in seconds to nanoseconds. The result is cast to uint64_t.
#define RPM_TO_DEG_PER_SEC(x) ((x)/60.0f*360.0f) ///< Converts an angular speed in revolutions per minute to degrees per second.
#define DEG_PER_SEC_TO_RPM(x) ((x)/360.0f*60.0f) ///< Converts an angular speed in degrees per second to revolutions per minute.

/**
 * @brief Gets the sign of the given float number.
 * @param x number to get the sign from.
 * @return +1.0f if x is positive, or -1.0f if x is negative.
 */
#define SIGNF(x) copysignf(1.0f, x)

/**
 * @brief Decimates the calls to the given command.
 * Actually executes the given command once out of decimFactor times, otherwise
 * does nothing.
 * @param decimFactor decimation factor.
 * @param action the command to execute.
 */
#define DECIM_CALL(decimFactor, action) \
    static int decim##__COUNTER__ = 0; \
    if(decim##__COUNTER__++ == decimFactor) { decim##__COUNTER__=0; action }

/**
 * @brief Miscellaneous utility functions.
 * @ingroup Lib
 */
namespace Utils
{
    void execBashCommand(std::string command, float timeout = 0.0f);
    std::string execBashCommandWithResult(std::string command,
                                          float timeout = 0.0f);
    void loadOverlay(std::string overlayName,
                     std::string fileToCheck = "");
    bool fileExists(std::string path);
    bool directoryExists(std::string path);
    std::vector<std::string> listFiles(std::string path);
    bool writeToDeviceFile(std::string path, std::string value);
    uint64_t getFilesystemFreeSpace(std::string filepath);
    float getCpuTemperature();

    std::vector<std::string> split(const std::string &str,
                                   char separator,
                                   bool keepEmpty=false);

    std::string dateToStr(std::chrono::time_point<std::chrono::system_clock> date,
                          std::string format);

    /**
     * @brief Evaluates a polynomial at the given point.
     * Evaluates a polynom c[0]*x^N + c[1]*x^(N-1) + ... + c[N-1]*x + c[N], at a
     * given point x.
     * @param coefs vector/array of polynom coefficients. The coefficients are
     * in the descending order of the power of x, so coefs.back() is the
     * constant.
     * @param x the evaluation point.
     * @return the value of the polynomial, evaluated at the point x.
     */
    template<typename T>
    float evalPolynomial(T coefs, float x)
    {
        float result = coefs.back();
        float multiplier = x;

        for(int i=(signed)coefs.size()-2; i>=0; i--)
        {
            result += coefs[i] * multiplier;
            multiplier *= x;
        }

        return result;
    }

    float evalPiecewisePolynomial(std::vector<std::vector<float>> coefsMatrix,
                                  std::vector<float> breaks, float x);
    float linearInterpolation(std::vector<float> x1, std::vector<float> y1,
                              float x2);
    bool circlesIntersect(Vec2<float> p0, float r0, Vec2<float> p1, float r1,
                          Vec2<float> &intersect0, Vec2<float> &intersect1);
    void reachValueWithLimitedRate(float &currentValue, float targetValue,
                                    float maxRate, float dt);

    /**
     * @brief Computes the average of all the values in the array.
     * @param v the array to average.
     * @return the average value.
     */
    template<typename T>
    T average(const std::vector<T> &v)
    {
        T sum = accumulate(v.begin(), v.end(), (T)0);
        return sum / (T) v.size();
    }

    /**
     * @brief Computes a low-pass filtered array using a moving average.
     * @param v the array to smooth.
     * @param filtWindowWidth number of samples for the average (filtering
     * window width).
     * @return the averaged array.
     */
    template<typename T>
    std::vector<T> runningMean(const std::vector<T> &v, int filtWindowWidth)
    {
        const int halfFiltWidth = filtWindowWidth / 2;
        const int nSamples = v.size();
        std::vector<T> filt(nSamples);

        for(int i=0; i<nSamples; i++)
        {
            int beginIndex = i - halfFiltWidth;
            if(beginIndex < 0)
                beginIndex = 0;

            int endIndex = i + halfFiltWidth;
            if(endIndex >= nSamples)
                endIndex = nSamples - 1;

            filt[i] = accumulate(v.begin()+beginIndex, v.begin()+endIndex+1,
                                 (T)0);
            filt[i] /= (T)(endIndex-beginIndex+1);
        }

        return filt;
    }

    /**
     * @brief Convert a number from one range to another.
     * @param num number to convert.
     * @param inMin inferior bound of the starting range.
     * @param inMax superior bound of the starting range.
     * @param outMin inferior bound of the final range.
     * @param outMax superior bound of the final range.
     */
    template<typename T>
    inline T map(T num, T inMin, T inMax, T outMin, T outMax)
    {
        return (num-inMin) / (inMax-inMin) * (outMax-outMin) + outMin;
    }

    /**
     * @brief Convert a number from one range to another, constrained in range.
     * @param num number to convert.
     * @param inMin inferior bound of the starting range.
     * @param inMax superior bound of the starting range.
     * @param outMin inferior bound of the final range. The result will be
     * clamped to this value.
     * @param outMax superior bound of the final range. The result will be
     * clamped to this value.
     */
    template<typename T>
    inline T mapClamp(T num, T inMin, T inMax, T outMin, T outMax)
    {
        float result = map(num, inMin, inMax, outMin, outMax);

        return std::clamp(result, outMin, outMax);
    }

    /**
     * @brief Constrain a periodic value between its min and max.
     * The given number will be modified by adding or substracting the period
     * (difference between max and min), until its fits between min and max.
     * @param num number that will be modified to fit between min and max.
     * @param min minimum value for num.
     * @param max maximum value for num.
     * @warning max must be greater than min, since no checks are performed.
     */
    template<typename T, typename std::enable_if<!std::is_integral<T>::value>
                                                     ::type* = nullptr>
    inline void constrainPeriodic(T &num, T min, T max)
    {
        T period = max - min;

        while(num < min)
            num += period;

        while(num > max)
            num -= period;
    }

     /**
     * @brief Constrain a periodic value between its min and max.
     * The given number will be modified by adding or substracting the period
     * (difference between max and min), until its fits between min and max.
     * This version is optimized for integer numbers.
     * @param num number that will be modified to fit between min and max.
     * @param min minimum value for num.
     * @param max maximum value for num.
     * @warning max must be greater than min, since no checks are performed.
     */
    template<typename T, typename std::enable_if<std::is_integral<T>::value>
                                                    ::type* = nullptr>
    inline void constrainPeriodic(T &num, T min, T max)
    {
        num = (num - min) % (max - min);

        if (num < 0)
            num += (max - min);

        num += min;
    }

    /**
     * @brief Constrain a periodic value between its min and max.
     * The given number will be modified by adding or substracting the period
     * (difference between max and min), until its fits between min and max.
     * This version is optimized for float numbers.
     * @param num number that will be modified to fit between min and max.
     * @param min minimum value for num.
     * @param max maximum value for num.
     * @warning max must be greater than min, since no checks are performed.
     */
    template<>
    inline void constrainPeriodic<float>(float &num, float min, float max)
    {
        num = fmodf(num - min, max - min);

        if (num < 0.0f)
            num += (max - min);

        num += min;
    }

    /**
     * @brief Constrain a periodic value between its min and max.
     * The given number will be modified by adding or substracting the period
     * (difference between max and min), until its fits between min and max.
     * This version is optimized for double numbers.
     * @param num number that will be modified to fit between min and max.
     * @param min minimum value for num.
     * @param max maximum value for num.
     * @warning max must be greater than min, since no checks are performed.
     */
    template<>
    inline void constrainPeriodic<double>(double &num, double min, double max)
    {
        num = fmod(num - min, max - min);

        if (num < 0.0)
            num += (max - min);

        num += min;
    }

    /**
     * @brief Change the value of a signed number represented by two positives.
     * @param increase amount to add to the signed number by the two positive
     * unsigned numbers positiveUnsigned and negativeUnsigned.
     * @param positiveUnsigned unsigned positive number holding the signed
     * number value in the case it is positive, or zero if it is negative. It
     * will be updated by the increase.
     * @param negativeUnsigned unsigned positive number holding the signed
     * number value in the case it is negative, or zero if it is positive. It
     * will be updated by the increase.
     */
    template<typename T>
    void signedIncreaseToTwoUnsigned(T increase,
                                     T &positiveUnsigned, T &negativeUnsigned)
    {
        if(increase > (T)0)
        {
            if(negativeUnsigned >= increase)
                negativeUnsigned -= increase;
            else
            {
                positiveUnsigned += (increase - negativeUnsigned);
                negativeUnsigned = (T)0;
            }
        }
        else
        {
            if(positiveUnsigned >= -increase)
                positiveUnsigned -= (-increase);
            else
            {
                negativeUnsigned += (-increase - positiveUnsigned);
                positiveUnsigned = (T)0;
            }
        }
    }

    /**
     * @brief Stores loop times, and display average automatically.
     */
    class LoopTimeMonitor
    {
    public:
        LoopTimeMonitor(int loopTimeSamplesCount);
        void update(uint64_t timestamp);

    private:
        const int loopTimeSamplesCount; ///< Number of samples to store, before computing their average.
        int loopTimeCounter; ///< Current number of samples collected.
        std::vector<float> loopTimeSamples; ///< Array containing all the samples.
        uint64_t prevTimestamp; ///< Previous timestamp [us].
    };

    /**
     * @brief Measures and prints the duration of program part.
     */
    class DurationClock
    {
    public:
        DurationClock(std::string tag,
                      std::chrono::milliseconds threshold);
        void printDuration();

    private:
        std::string tag; ///< Tag to be printed along with the measured duration, for easier identification.
        std::chrono::milliseconds threshold; ///< Minimum duration to be printed [ms].
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime; ///< Begin time.
    };
}

/**
 * @}
 */

#endif
