#ifndef DEF_LIB_SYNCVAR_SYNCVAR_H
#define DEF_LIB_SYNCVAR_SYNCVAR_H

#include <vector>
#include <fstream>
#include <string>
#include <typeinfo>
#include <functional>
#include <cstring>
#include <cstdint>
#include <type_traits>

#include "../../public_definitions.h"

class Peripheral;

/**
 * @defgroup SyncVar SyncVar
 * @brief Set of classes to remotely monitor variables in the program.
 *
 * Create one SyncVar per variable to be monitored, then add them all to the
 * unique SyncVarManager. The goal is to make a list ready-to-use for the
 * Communication module, to allow a remote client to monitor and modify the
 * variables values.
 * If a class has several SyncVars, it may be useful to store them into a
 * SyncVarList.
 *
 * @ingroup Lib
 * @{
 */

/**
 * @brief Represents a variable to be controlled from a remote computer.
 * @ingroup SyncVar
 */
class SyncVar
{
public:
    SyncVar(std::string name, std::string unit, VarType type, VarAccess access,
            bool logToFile, float changeTime);
    SyncVar(const SyncVar&) = delete;
    virtual ~SyncVar();
    SyncVar() = delete;
    SyncVar& operator=(const SyncVar&) = delete;

    /**
     * @brief Duplicates a SyncVarAddr object.
     * @return a pointer to the duplicated object.
     */
    virtual SyncVar *clone() const = 0;

    /**
     * @brief Updates the value of this SyncVar.
     * This method has an effect only if the variable is being smoothly changed
     * (changeTime > 0).
     * @param dt time elapsed since the last call to this method [s].
     */
    virtual void update(float dt) = 0;

    void setName(std::string name);
    void setLogToFile(bool logToFile);
    void setChangeTime(float changeTime);

    std::string getName() const;
    std::string getUnit() const;
    VarType getType() const;
    VarAccess getAccess() const;
    uint32_t getLength() const;
    bool getLogToFile() const;

    /**
     * @brief Sets the SyncVar variable value from a byte array.
     * @param v the byte array to set the value.
     * @param startIndex the value start index in the byte array.
     * @return the given value of the variable, as a human-readable string.
     */
    virtual std::string setData(std::vector<uint8_t> v, int startIndex) = 0;

    /**
     * @brief Gets the address of the SyncVar variable.
     * @return a pointer to the SyncVar value.
     * @warning The address returned by this function may not be the actual
     * monitored variable, it could be a intermediate variable. So it is not
     * recommended to store this address, instead getData() should be called
     * every time.
     */
    virtual uint8_t* getData() = 0;

    /**
     * @brief Gets the value of the SyncVar variable, printed on a string.
     * In the case of a variable being changed smoothly, this will actually
     * return the target value, not the current value.
     * @return a string with the SyncVar value printed.
     */
    virtual std::string getValueString() const = 0;

protected:
    void setUnit(std::string unit);

    /// @cond DOXYGEN_IGNORE
    static VarType getType(const volatile bool&) { return VarType::BOOL; }
    static VarType getType(const volatile uint8_t&) { return VarType::UINT8; }
    static VarType getType(const volatile int8_t&) { return VarType::INT8; }
    static VarType getType(const volatile uint16_t&) { return VarType::UINT16; }
    static VarType getType(const volatile int16_t&) { return VarType::INT16; }
    static VarType getType(const volatile uint32_t&) { return VarType::UINT32; }
    static VarType getType(const volatile int32_t&) { return VarType::INT32; }
    static VarType getType(const volatile uint64_t&) { return VarType::UINT64; }
    static VarType getType(const volatile int64_t&) { return VarType::INT64; }
    static VarType getType(const volatile float&) { return VarType::FLOAT32; }
    static VarType getType(const volatile double&) { return VarType::FLOAT64; }

    static VarType getType(const bool&) {return VarType::BOOL;}
    static VarType getType(const uint8_t&) {return VarType::UINT8;}
    static VarType getType(const int8_t&) {return VarType::INT8;}
    static VarType getType(const uint16_t&) {return VarType::UINT16;}
    static VarType getType(const int16_t&) {return VarType::INT16;}
    static VarType getType(const uint32_t&) {return VarType::UINT32;}
    static VarType getType(const int32_t&) {return VarType::INT32;}
    static VarType getType(const uint64_t&) {return VarType::UINT64;}
    static VarType getType(const int64_t&) {return VarType::INT64;}
    static VarType getType(const float&) {return VarType::FLOAT32;}
    static VarType getType(const double&) {return VarType::FLOAT64;}
    /// @endcond

    std::string name; ///< Full name, including the prefix.
    std::string unit; ///< Value unit.
    VarType type; ///< Variable type.
    VarAccess access; ///< Variable access.
    uint32_t length; ///< Variable bytes size.
    bool logToFile; ///< Indicates whether it should be logged to file by SyncVarManager or not.
    float changeTime; ///< Duration of a smooth change of the SyncVar value [s]. Disabled if zero (instantaneous change).
    float changeCurrentProgressTime; ///< Time since the smooth change of the variable started [s].
};


/**
 * @brief Helper class for a list of SyncVar objects.
 * This class offers two main enhancements over a vector<SyncVar*>:
 * - it is possible to add all the items of a SyncVarList to another
 * SyncVarList, modifying automatically the added items names with the given
 * prefix.
 * - the destructor takes care of properly destructing all the SyncVars.
 */
class SyncVarList : public std::vector<SyncVar*>
{
public:
    SyncVarList() = default;
    SyncVarList(const SyncVarList& o);
    ~SyncVarList();
    void operator=(const SyncVarList& o);
    SyncVar* getFromName(std::string name);
    std::vector<const SyncVar*> getAllOfAccess(VarAccess access,
                                               bool ofThisType = true) const;
    std::vector<SyncVar*> getAllOfAccess(VarAccess access,
                                         bool ofThisType = true);

    void add(std::string prefix, const SyncVarList& vars);

    template<typename T>
    void add(std::string name, std::string unit, T& var, VarAccess access,
             bool logToFile, float changeTime = 0.0f);
    template<typename T>
    void add(std::string name, std::string unit,
             std::function<T(void)> getter, std::function<void (T)> setter,
             bool logToFile, float changeTime = 0.0f);

};

#include "syncvaraddr.h"
#include "syncvarfunc.h"

/**
 * @brief Creates a new SyncVarAddr and adds it to the list.
 * @param name SyncVar human-readable name. It can include slashes "/" to build
 * a hierarchy, like a filepath.
 * @param unit the unit of the SyncVar, without brackets.
 * @param var reference to the variable to monitor.
 * @param access the access rights of the SyncVar.
 * @param logToFile true if the SyncVar value should be logged to the variables
 * logfile (.wkv), false otherwise.
 * @param changeTime smooth change time [s], or zero to change instantaneously
 * the value.
 * @note the name cannot be longer than SYNCVAR_NAME_COMM_LENGTH. If the given
 * string is longer, the last characters will be removed in order to fit.
 * @note the unit string cannot be longer than SYNCVAR_UNIT_COMM_LENGTH. If the
 * given string is longer, the last characters will be removed in order to fit.
 * @note If the SyncVars list has been locked, then this function does nothing.
 */
template<typename T>
void SyncVarList::add(std::string name, std::string unit, T& var,
                      VarAccess access, bool logToFile, float changeTime)
{
    push_back(makeSyncVar(name, unit, var, access, logToFile, changeTime));
}

/**
 * @brief Creates a new SyncVarFunc and adds it to the list.
 * @param name SyncVar human-readable name. It can include slashes "/" to build
 * a hierarchy, like a filepath.
 * @param unit the unit of the SyncVar, without brackets.
 * @param getter getter function, most likely a lambda. nullptr can be passed
 * instead, if the variable should not be read.
 * @param setter setter function, most likely a lambda. nullptr can be passed
 * instead, if the variable should not be written.
 * @param logToFile true if the SyncVar value should be logged to the variables
 * logfile (.wkv), false otherwise.
 * @param changeTime smooth change time [s], or zero to change instantaneously
 * the value.
 * @note the name cannot be longer than SYNCVAR_NAME_COMM_LENGTH. If the given
 * string is longer, the last characters will be removed in order to fit.
 * @note the unit string cannot be longer than SYNCVAR_UNIT_COMM_LENGTH. If the
 * given string is longer, the last characters will be removed in order to fit.
 * @note If the SyncVars list has been locked, then this function does nothing.
 */
template<typename T>
void SyncVarList::add(std::string name, std::string unit,
                      std::function<T ()> getter,
                      std::function<void (T)> setter,
                      bool logToFile, float changeTime)
{
    push_back(makeSyncVar(name, unit, getter, setter, logToFile, changeTime));
}

/**
 * @}
 */

#endif
