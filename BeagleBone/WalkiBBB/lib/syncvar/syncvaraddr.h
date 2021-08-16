#ifndef DEF_LIB_SYNCVAR_SYNCVARADDR_H
#define DEF_LIB_SYNCVAR_SYNCVARADDR_H

#include "syncvar.h"

/**
 * @brief SyncVar that accesses a monitored variable through its address.
 * @ingroup SyncVar
 */
template<typename T>
class SyncVarAddr : public SyncVar
{
public:
    SyncVarAddr(std::string name, std::string unit, T& var, VarAccess access,
                bool logToFile, float changeTime);

    SyncVarAddr<T> *clone() const override;

    void update(float dt) override;

    std::string setData(std::vector<uint8_t> v, int startIndex) override;
    uint8_t *getData() override;

    std::string getValueString() const override;

private:
    SyncVarAddr(std::string name, std::string unit, VarType type,
                VarAccess access, uint8_t *address, uint32_t length,
                bool logToFile, float changeTime);
    void update(float dt, std::false_type); // Normal case.
    void update(float, std::true_type); // Specialization for bool (instant change).

    uint8_t* address; ///< Variable address.
    std::remove_cv_t<T> initialValue; ///< Initial value when the smooth change started.
    std::remove_cv_t<T> targetValue; ///< Target value to reach when the smooth change is enabled (changeTime > 0.0).
};

/**
 * @brief SyncVarAddr constructor for a generic variable.
 * @param name variable name, including the prefix.
 * @param unit unit of the variable.
 * @param var variable to monitor.
 * @param access variable access.
 * @param logToFile true if the SyncVar value should be logged to the variables
 * logfile (.wkv), false otherwise.
 * @param changeTime smooth change time [s], or zero to change instantaneously
 * the value.
 * @note The name cannot be longer than SYNCVAR_NAME_COMM_LENGTH. If the given
 * string is longer, the last characters will be removed in order to fit.
 * @note the unit string cannot be longer than SYNCVAR_UNIT_COMM_LENGTH. If the
 * given string is longer, the last characters will be removed in order to fit.
 */
template<typename T>
SyncVarAddr<T>::SyncVarAddr(std::string name, std::string unit, T &var,
                            VarAccess access, bool logToFile,
                            float changeTime) :
    SyncVar(name, unit, getType(var), access, logToFile, changeTime)
{
    address = (uint8_t*)&var;
    targetValue = var;

    if(type == VarType::BOOL)
        length = 1;
    else
        length = sizeof(T);
}

/**
 * @brief Duplicates a SyncVarAddr object.
 * @return a pointer to the duplicated object.
 */
template<typename T>
SyncVarAddr<T>* SyncVarAddr<T>::clone() const
{
    return new SyncVarAddr(name, unit, type, access, address, length,
                           logToFile, changeTime);
}

/**
 * @brief Updates the value of this SyncVar.
 * This method has an effect only if the variable is being smoothly changed
 * (changeTime > 0).
 * @param dt time elapsed since the last call to this method [s].
 */
template<typename T>
void SyncVarAddr<T>::update(float dt)
{
    if(changeTime > 0.0f && changeCurrentProgressTime < changeTime)
        update(dt, std::is_same<std::remove_cv_t<T>, bool>());
}

/**
 * @brief Updates the value of this SyncVar (standard implementation).
 * This method has an effect only if the variable is being smoothly changed
 * (changeTime > 0).
 * @param dt time elapsed since the last call to this method [s].
 */
template<typename T>
void SyncVarAddr<T>::update(float dt, std::false_type)
{
    changeCurrentProgressTime += dt;

    typename std::remove_const<T>::type updatedValue;

    if(changeCurrentProgressTime < changeTime)
    {
        float progress = changeCurrentProgressTime / changeTime;
        updatedValue = (T)((1.0f - progress) * initialValue +
                            progress * targetValue);
    }
    else
        updatedValue = targetValue;

    memcpy((uint8_t*)address, (uint8_t*)&updatedValue, sizeof(T));
}

/**
 * @brief Updates the value of this SyncVar (special implementation for bool).
 * This method changes instantly the value to the target value, even if
 * changeTime > 0.
 * @param dt time elapsed since the last call to this method [s], actually
 * ignored.
 */
template<typename T>
void SyncVarAddr<T>::update(float, std::true_type)
{
    // A boolean changes instantly.
    changeCurrentProgressTime += changeTime;

    T updatedValue = targetValue;
    memcpy((uint8_t*)address, (uint8_t*)&updatedValue, sizeof(T));
}

/**
 * @brief Sets the SyncVar variable value from a byte array.
 * @param v the byte array to set the value.
 * @param startIndex the value start index in the byte array.
 * @return the given value of the variable, as a human-readable string.
 */
template<typename T>
std::string SyncVarAddr<T>::setData(std::vector<uint8_t> v, int startIndex)
{
    if(v.size() >= startIndex + length)
    {
        memcpy((uint8_t*)&targetValue, &v[startIndex], length);

        if(changeTime > 0.0f)
        {
            // Smooth change.
            initialValue = *(T*)address;
            changeCurrentProgressTime = 0.0f;
        }
        else
            memcpy((uint8_t*)address, &v[startIndex], length); // Instant change.
    }
    else
        std::runtime_error("SyncVarAddr::setData: not enough bytes in array.");

    return getValueString();
}

/**
 * @brief Gets the address of the SyncVar variable.
 * @return a pointer to the SyncVar value.
 */
template<typename T>
uint8_t* SyncVarAddr<T>::getData()
{
    return (uint8_t*)address;
}

/**
 * @brief Gets the value of the SyncVar variable, printed on a string.
 * In the case the variable being changed smoothly, this will actually return
 * the target value, not the current value.
 * @return a string with the SyncVar value printed.
 */
template<typename T>
std::string SyncVarAddr<T>::getValueString() const
{
    if(type == VarType::STRING)
        return std::string((char*)&targetValue);
    else
        return std::to_string(targetValue);
}

/**
 * @brief Alternative constructor to allow duplicating an existing object.
 * @param name variable name, including the prefix.
 * @param unit unit of the variable.
 * @param type type of the variable.
 * @param access variable access.
 * @param address memory addresss of the variable.
 * @param length size of the type of the variable.
 * @note The name cannot be longer than SYNCVAR_NAME_COMM_LENGTH. If the given
 * string is longer, the last characters will be removed in order to fit.
 * @note the unit string cannot be longer than SYNCVAR_UNIT_COMM_LENGTH. If the
 * given string is longer, the last characters will be removed in order to fit.
 */
template<typename T>
SyncVarAddr<T>::SyncVarAddr(std::string name, std::string unit, VarType type,
                            VarAccess access, uint8_t* address, uint32_t length,
                            bool logToFile, float changeTime) :
    SyncVar(name, unit, type, access, logToFile, changeTime)
{
    targetValue = *(T*)address;
    this->address = address;
    this->length = length;
}

/**
 * @brief Convenience function to create a SyncVarAddr object.
 * @ingroup SyncVar
 */
template<typename T>
SyncVar* makeSyncVar(std::string name, std::string unit, T& var,
                     VarAccess access, bool logToFile, float changeTime = 0.0f)
{
    return new SyncVarAddr<T>(name, unit, var, access, logToFile, changeTime);
}

#endif
