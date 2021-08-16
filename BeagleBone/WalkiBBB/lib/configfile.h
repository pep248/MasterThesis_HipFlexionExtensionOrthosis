#ifndef DEF_LIB_CONFIGFILE_H
#define DEF_LIB_CONFIGFILE_H

#include <string>
#include <sstream>
#include <map>

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "vecn.h"

/**
 * @defgroup ConfigFile ConfigFile
 * @brief Loads parameters to a text-based configuration file.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Loads parameters to a text-based configuration file.
 */
class ConfigFile
{
public:
    ConfigFile(std::string filename);
    ~ConfigFile();

    bool contains(std::string name) const;

    template<typename T> T get(std::string name, T defaultValue);
    template<typename T> void set(std::string name, T newValue);

private:
    static int wordsCount(std::string str);

    const std::string configFileName;
    std::map<std::string, std::string> parameters;
};

template<>
std::string ConfigFile::get(std::string name, std::string defaultValue);

/**
 * @brief Gets the value of a parameter.
 * @param name parameter name, as written in the file.
 * @param defaultValue value to be returned, in case name is not found.
 * @return the value of the desired parameter, or defaultValue if name could not
 * be found.
 */
template<typename T> T ConfigFile::get(std::string name, T defaultValue)
{
    if(contains(name))
    {
        std::istringstream iss(parameters[name]);
        T value;
        iss >> value;

        return value;
    }
    else
        return defaultValue;
}

/**
 * @brief Sets the value of a parameter.
 * @param name parameter name, as written in the file.
 * @param newValue desired value of the parameter.
 * @remark The file will be actually be written when the ConfigFile object is
 * destroyed.
 */
template<typename T> void ConfigFile::set(std::string name, T newValue)
{
    std::ostringstream oss;
    oss << newValue;
    parameters[name] = oss.str();
}

/**
 * @}
 */

#endif
