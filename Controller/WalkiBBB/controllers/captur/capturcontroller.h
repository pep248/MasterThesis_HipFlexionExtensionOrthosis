#ifndef DEF_CONTROLLERS_CAPTUR_CAPTURCONTROLLER_H
#define DEF_CONTROLLERS_CAPTUR_CAPTURCONTROLLER_H

#include "../controller.h"
#include "../../drivers/amt20.h"
#include "../../drivers/mpu60x0.h"
#include "../../lib/quadloadcells.h"
#include "../../lib/stateestimator.h"
#include "../../drivers/crutchboard.h"

/**
 * @defgroup CapturController CapturController
 * @brief Controller for the Captur passive exoskeleton.
 * @ingroup Controllers
 * @{
 */

#define MAIN_LOOP_PERIOD 0.010f ///< Main loop period [s].

/**
 * @brief Controller for the Captur passive exoskeleton.
 */
class CapturController : public Controller
{
public:
    CapturController(PeripheralsSet peripherals);
    ~CapturController() override;

    void update(float dt) override;

    void setEncodersZero();

private:
    SpiChannel leftHipEncoderSpiChannel, leftKneeEncoderSpiChannel,
               rightHipEncoderSpiChannel, rightKneeEncoderSpiChannel,
               leftFootLoadCellsSpiChannel, rightFootLoadCellsSpiChannel,
               leftFootImuSpiChannel, rightFootImuSpiChannel;
    Amt20 leftHipEncoder,
          leftKneeEncoder,
          rightHipEncoder,
          rightKneeEncoder;
    float leftHipAngle, leftKneeAngle, rightHipAngle, rightKneeAngle;
    QuadLoadCells leftFootLoadCells, rightFootLoadCells;
    Mpu6050 &backImu;
    Mpu6000 leftFootImu, rightFootImu;
    float trunkPitch, trunkRoll; // [deg].
    StateEstimator trunkOrientationEstimator,
                   leftFootEstimator, rightFootEstimator;
    CrutchBoard leftCrutch, rightCrutch;
};

typedef CapturController SelectedController; ///< The main() should use this controller, if this file is included.

/**
 * @}
 */

#endif
