#ifndef DEF_LIB_SYNCVAR_SYNCVARFUNC_H
#define DEF_LIB_SYNCVAR_SYNCVARFUNC_H

#include "syncvar.h"

/**
 * @brief SyncVar that accesses a monitored variable through a getter/setter.
 * @ingroup SyncVar
 */
template<typename T>
class SyncVarFunc : public SyncVar
{
public:
    SyncVarFunc(std::string name, std::string unit,
                std::function<T(void)> getter,
                std::function<void(T)> setter, bool logToFile,
                float changeTime);
    SyncVar *clone() const override;

    void update(float dt) override;

    std::string setData(std::vector<uint8_t> v, int startIndex) override;
    uint8_t *getData() override;
    std::string getValueString() const override;

private:
    void update(float dt, std::false_type); // Normal case.
    void update(float, std::true_type); // Specialization for bool (instant change).

    std::function<T(void)> getter;
    std::function<void(T)> setter;
    std::remove_cv_t<T> tempVarValue; ///< Cache used to hold the value returned by getData().
    std::remove_cv_t<T> initialValue; ///< Initial value when the smooth change started.
    std::remove_cv_t<T> targetValue; ///< Target value to reach when the smooth change is enabled (changeTime > 0.0).
};

/**
 * @brief SyncVarFunc constructor for a generic variable.
 * @param name variable name, including the prefix.
 * @param unit unit of the variable.
 * @param getter getter function, most likely a lambda. nullptr can be passed
 * instead, if the variable should not be read.
 * @param setter setter function, most likely a lambda. nullptr can be passed
 * instead, if the variable should not be written.
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
SyncVarFunc<T>::SyncVarFunc(std::string name, std::string unit,
                            std::function<T(void)> getter,
                            std::function<void (T)> setter, bool logToFile,
                            float changeTime) :
    SyncVar(name, unit, getType((T)0),
            getter ? (setter ? VarAccess::READWRITE : VarAccess::READ) :
                     (setter ? VarAccess::WRITE : VarAccess::NONE), logToFile,
            changeTime)
{
    this->getter = getter;
    this->setter = setter;

    if(getter == nullptr)
        targetValue = (T)0;
    else
        targetValue = getter();

    if(getter == nullptr && changeTime > 0.0f)
        throw std::runtime_error("SyncVarFunc: the variable cannot be writeonly if changeTime > 0.");

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
SyncVar *SyncVarFunc<T>::clone() const
{
    return new SyncVarFunc<T>(name, unit, getter, setter, logToFile,
                              changeTime);
}

/**
 * @brief Updates the value of this SyncVar.
 * This method has an effect only if the variable is being smoothly changed
 * (changeTime > 0).
 * @param dt time elapsed since the last call to this method [s].
 */
template<typename T>
void SyncVarFunc<T>::update(float dt)
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
void SyncVarFunc<T>::update(float dt, std::false_type)
{
    changeCurrentProgressTime += dt;

    if(changeCurrentProgressTime < changeTime)
    {
        float progress = changeCurrentProgressTime / changeTime;
        setter((T)((1.0f - progress) * initialValue + progress * targetValue));
    }
    else
        setter(targetValue);
}

/**
 * @brief Updates the value of this SyncVar (special implementation for bool).
 * This method changes instantly the value to the target value, even if
 * changeTime > 0.
 * @param dt time elapsed since the last call to this method [s], actually
 * ignored.
 */
template<typename T>
void SyncVarFunc<T>::update(float, std::true_type)
{
    // A boolean changes instantly.
    changeCurrentProgressTime += changeTime;
    setter(targetValue);
}

/**
 * @brief Sets the SyncVar variable value from a byte array.
 * @param v the byte array to set the value.
 * @param startIndex the value start index in the byte array.
 * @return the target value of the variable, as a human-readable string.
 */
template<typename T>
std::string SyncVarFunc<T>::setData(std::vector<uint8_t> v, int startIndex)
{
    if(v.size() >= startIndex + length)
    {
        memcpy(&targetValue, &v[startIndex], sizeof(T));

        if(changeTime > 0.0f)
        {
            // Smooth change.
            changeCurrentProgressTime = 0.0f;
            initialValue = getter();
        }
        else
        {
            // Instant change.
            memcpy(&targetValue, &v[startIndex], sizeof(T));
            setter(targetValue);
        }
    }
    else
        std::runtime_error("SyncVarFunc::setData: not enough bytes in array.");

    return std::to_string(targetValue);
}

/**
 * @brief Gets the address of the SyncVar variable.
 * @return a pointer to the SyncVar value.
 * @warning As the variable can only be accessed through the getter/setter, the
 * returned address is actually a local copy of the monitored variable value,
 * which is updated only when getData() is called. So, it not recommended to
 * store this address, instead getData() should be called every time.
 */
template<typename T>
uint8_t* SyncVarFunc<T>::getData()
{
    if(getter != nullptr)
        tempVarValue = getter();
    else
        tempVarValue = (T)0;

    return (uint8_t*)&tempVarValue;
}

/**
 * @brief Gets the value of the SyncVar variable, printed on a string.
 * @return a string with the SyncVar target value printed.
 * @remark For a WRITEONLY SyncVar, calling this method will return zero.
 */
template<typename T>
std::string SyncVarFunc<T>::getValueString() const
{
    return std::to_string(targetValue);
}

/**
 * @brief Convenience function to create a SyncVarFunc object.
 * @ingroup SyncVar
 */
template<typename T>
SyncVar* makeSyncVar(std::string name, std::string unit,
                     std::function<T(void)> getter,
                     std::function<void (T)> setter, bool logToFile,
                     float changeTime = 0.0f)
{
    return new SyncVarFunc<T>(name, unit, getter, setter, logToFile,
                              changeTime);
}

#endif
