#include "configfile.h"

#include <fstream>
#include <algorithm>

#include "debugstream.h"

using namespace std;

#ifdef __arm__
#define CONFIG_FILES_DIR "/root/WalkiSoftware/BeagleBone/config/" ///< Config files directory.
#else
#define CONFIG_FILES_DIR "config/" ///< Config files directory.
#endif

/**
 * @brief Constructor.
 * @param filename filename of the configuration file to load.
 */
ConfigFile::ConfigFile(std::string filename) :
    configFileName(CONFIG_FILES_DIR + filename)
{
    // Open the configuration file, and read it line by line.
    ifstream configFile(configFileName);

    while(configFile.good() && !configFile.eof())
    {
        // Get the next line of the file.
        string line;
        getline(configFile, line);

        // Split the line between the parameter name and its value.
        auto separatorIndex = line.find(' ');

        if(separatorIndex == string::npos)
            continue;

        string parameterName = line.substr(0, separatorIndex);
        string parameterValue = line.substr(separatorIndex+1, string::npos);

        // Add the pair to the map.
        parameters[parameterName] = parameterValue;
    }

    // Print the content of the file.
    debug << "Configuration file " << filename << " content:" << endl;

    for(const auto &p : parameters)
        debug << p.first << ": " << p.second << endl;

    debug << endl;
}

/**
 * @brief Destructor. Saves the parameters back in the configuration file.
 */
ConfigFile::~ConfigFile()
{
    // Open the configuration file, and write it line by line.
    ofstream configFile(configFileName);

    if(!configFile.good())
        return;

    for(auto p : parameters)
        configFile << p.first << " " << p.second << endl;
}

/**
 * @brief Indicates if a parameter exists in the list.
 * @param name name of the parameter, as written in the file.
 * @return true if the parameter exists, false otherwise.
 */
bool ConfigFile::contains(string name) const
{
    return parameters.find(name) != parameters.end();
}

/**
 * @brief Gets the value of a parameter of type string.
 * @param name parameter name, as written in the file.
 * @param defaultValue value to be returned, in case name is not found.
 * @return the value of the desired parameter, or defaultValue if name could not
 * be found.
 */
template<>
std::string ConfigFile::get(std::string name, std::string defaultValue)
{
    if(contains(name))
        return parameters[name];
    else
        return defaultValue;
}

/**
 * @brief Counts the number of words (separated by spaces) in the given string.
 * @param str the string to count the words.
 * @return the words count.
 */
int ConfigFile::wordsCount(string str)
{
    return std::count(str.begin(), str.end(), ' ') + 1;
}
