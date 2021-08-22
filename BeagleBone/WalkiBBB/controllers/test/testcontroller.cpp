#include "testcontroller.h"

#include "../../lib/debugstream.h"

using namespace std;
using namespace chrono;

/**
 * @brief Constructor.
 * @param peripherals peripherals initialized by main().
 */
TestController::TestController(PeripheralsSet peripherals) :
    Controller("test", peripherals)
{

}

/**
 * @brief Destructor.
 */
TestController::~TestController()
{

}

/**
 * @brief Updates the controller.
 * @param dt timestep (time elapsed since the last call to this method) [s].
 */
void TestController::update(float)
{

}
