#ifndef DEF_CONTROLLERS_TEST_TESTCONTROLLER_H
#define DEF_CONTROLLERS_TEST_TESTCONTROLLER_H

#include "../controller.h"

/**
 * @defgroup TestController Test controller
 * @brief Empty controller, to be used for temporary short tests.
 * @ingroup Controllers
 * @{
 */

#define MAIN_LOOP_PERIOD 0.002f ///< Main loop period [s].

/**
 * @brief Empty controller, to be used for temporary short tests.
 * @ingroup Controllers
 */
class TestController : public Controller
{
public:
    TestController(PeripheralsSet peripherals);
    ~TestController();

    void update(float dt) override;

private:
};

typedef TestController SelectedController; ///< Sets the application controller as TestController.

/**
 * @}
 */

#endif
