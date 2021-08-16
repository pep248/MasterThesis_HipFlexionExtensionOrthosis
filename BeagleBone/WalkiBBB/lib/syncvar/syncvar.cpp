#include "syncvar.h"

#include <stdexcept>

#include "../debugstream.h"
#include "../peripheral.h"
#include "../utils.h"

using namespace std;

/**
 * @brief SyncVar constructor.
 * @param name SyncVar human-readable name. It can include slashes "/" to build
 * a hierarchy, like a filepath.
 * @param unit the unit of the SyncVar, without brackets.
 * @param type the type of the SyncVar.
 * @param access the access rights of the SyncVar.
 * @param logToFile true if the SyncVar value should be logged to the variables
 * logfile (.wkv), false otherwise.
 * @param changeTime smooth change time [s], or zero to change instantaneously
 * the value.
 */
SyncVar::SyncVar(std::string name, std::string unit, VarType type,
                 VarAccess access, bool logToFile, float changeTime)
{
    setName(name);
    setUnit(unit);
    this->type = type;
    this->access = access;
    this->logToFile = logToFile;
    this->changeTime = changeTime;
    changeCurrentProgressTime = changeTime; // Reached final value.
}

/**
 * @brief SyncVar destructor.
 */
SyncVar::~SyncVar()
{

}

/**
 * @brief Sets the SyncVar name.
 * @param name the new name.
 * @note the name cannot be longer than SYNCVAR_NAME_COMM_LENGTH. If the given
 * string is longer, the last characters will be removed in order to fit.
 */
void SyncVar::setName(string name)
{
    if(name.size() > SYNCVAR_NAME_COMM_LENGTH - 1)
        name.resize(SYNCVAR_NAME_COMM_LENGTH - 1);

    this->name = name;
}

/**
 * @brief Sets whether the SyncVar should be logged to file or not.
 * @param logToFile true if the variable should be logged to file, false
 * otherwise.
 */
void SyncVar::setLogToFile(bool logToFile)
{
    this->logToFile = logToFile;
}

/**
 * @brief Set the smooth change duration of the SyncVar value.
 * @param changeTime change time [s], or zero to change instantaneously the
 * value.
 */
void SyncVar::setChangeTime(float changeTime)
{
    this->changeTime = changeTime;
}

/**
 * @brief Gets the SyncVar name.
 * @return the SyncVar name.
 */
string SyncVar::getName() const
{
    return name;
}

/**
 * @brief Gets the unit of the SyncVar.
 * @return the unit of the SyncVar.
 */
string SyncVar::getUnit() const
{
    return unit;
}

/**
 * @brief Gets the type of the SyncVar.
 * @return the type of the SyncVar.
 */
VarType SyncVar::getType() const
{
    return type;
}

/**
 * @brief Gets the SyncVar access.
 * @return the SyncVar access.
 */
VarAccess SyncVar::getAccess() const
{
    return access;
}

/**
 * @brief Gets the SyncVar variable size.
 * @return the SyncVar variable size.
 */
uint32_t SyncVar::getLength() const
{
    return length;
}

/**
 * @brief Gets if the SyncVar should be logged to file.
 * @return true if the SyncVar should be logged to the file, false otherwise.
 */
bool SyncVar::getLogToFile() const
{
    return logToFile;
}

/**
 * @brief Set the SyncVar unit.
 * @param unit the new unit string, without the brackets.
 * @note the unit string cannot be longer than SYNCVAR_UNIT_COMM_LENGTH. If the
 * given string is longer, the last characters will be removed in order to fit.
 */
void SyncVar::setUnit(string unit)
{
    if(unit.size() > SYNCVAR_UNIT_COMM_LENGTH -1)
        unit.resize(SYNCVAR_UNIT_COMM_LENGTH - 1);

    this->unit = unit;
}

/**
 * @brief SyncVarList copy constructor.
 * Creates a SyncVarList from another one. All the SyncVars will be copied.
 */
SyncVarList::SyncVarList(const SyncVarList &o) : vector<SyncVar*>()
{
    add("", o);
}

/**
 * @brief SyncVarList destructor.
 * Destructs all the SyncVars in the list.
 */
SyncVarList::~SyncVarList()
{
    for(SyncVar *sv : *this)
        delete sv;
}

/**
 * @brief Sets the content of this list as the same as an other SyncVarList.
 * @param o the SyncVarList to copy.
 */
void SyncVarList::operator=(const SyncVarList &o)
{
    // Clear the current content.
    for(SyncVar *sv : *this)
        delete sv;
    clear();

    // Copy the content of the other list.
    add("", o);
}

/**
 * @brief Gets a SyncVar in the list from its name.
 * @param name name of the variable to find.
 * @return the address of the SyncVar with the matcing name.
 * @throw A runtime_error is thrown if the given name was not found in the list.
 */
SyncVar* SyncVarList::getFromName(string name)
{
    for(SyncVar *sv : *this)
    {
        if(sv->getName() == name)
            return sv;
    }

    throw runtime_error("SyncVarList::getFromName(): SyncVar name \"" +
                        name +  "\" not found.");
}

/**
 * @brief Gets all the variables with the given access type (const version).
 * For example, to get all the variable that are write-only, use
 * getAllOfAccess(VarAccess::WRITE). To get all the variables that are not
 * write-only, use getAllOfAccess(VarAccess::WRITE, false).
 * @param access variable access type (READ, WRITE or READWRITE).
 * @param ofThisType true if the returned variables should match the given
 * access, false if the returned should not match the given access.
 * @return an array of pointers to constant SyncVars.
 */
std::vector<const SyncVar *> SyncVarList::getAllOfAccess(VarAccess access,
                                                         bool ofThisType) const
{
    vector<const SyncVar *> selectedVars;

    for(auto &sv : *this)
    {
        if((sv->getAccess() == access) == ofThisType)
            selectedVars.push_back(sv);
    }

    return selectedVars;
}

/**
 * @brief Gets all the variables with the given access type.
 * For example, to get all the variable that are write-only, use
 * getAllOfAccess(VarAccess::WRITE). To get all the variables that are not
 * write-only, use getAllOfAccess(VarAccess::WRITE, false).
 * @param access variable access type (READ, WRITE or READWRITE).
 * @param ofThisType true if the returned variables should match the given
 * access, false if the returned should not match the given access.
 * @return an array of pointers to SyncVars.
 */
vector<SyncVar *> SyncVarList::getAllOfAccess(VarAccess access, bool ofThisType)
{
    vector<SyncVar *> selectedVars;

    for(auto &sv : *this)
    {
        if((sv->getAccess() == access) == ofThisType)
            selectedVars.push_back(sv);
    }

    return selectedVars;
}

/**
 * @brief Adds the given SyncVars to the list, prefixing their names.
 * All the SyncVars from the given list are copied, then the copies are renamed
 * with the given prefix, then the modified copies are added to this list.
 * @param prefix text to prepend to all the SyncVars names.
 * @param vars list of SyncVars to add to this list. Actually, the SyncVars will
 * all be copied, and the copy will be renamed and added to the list.
 */
void SyncVarList::add(std::string prefix, const SyncVarList& vars)
{
    for(SyncVar *sv : vars)
    {
        push_back(sv->clone());
        back()->setName(prefix + back()->getName());
    }
}
