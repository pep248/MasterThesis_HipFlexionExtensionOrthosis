#include "controller.h"

using namespace std;

/**
 * @brief Constructor.
 * @param name short description of the controller.
 * @param peripherals peripherals initialized by main().
 */
Controller::Controller(string name, PeripheralsSet peripherals) : name(name)
{
    this->peripherals = peripherals;
}

/**
 * @brief Destructor.
 */
Controller::~Controller()
{

}

/**
 * @brief Gets the controller name.
 * @return a short description of the controller.
 */
string Controller::getName() const
{
    return name;
}

/**
 * @brief Gets the SyncVars of the controller.
 * @return a SyncVarList containing the SyncVars of the controller.
 */
SyncVarList Controller::getVars()
{
    return syncVars;
}
