#ifndef ECHOING_H
#define ECHOING_H

#include <chrono>

#include "../controller.h"

#include "../../drivers/motorboard.h"
#include "../../drivers/mpu60x0.h"
#include "../../drivers/spi.h"
#include "../../drivers/adc.h"
#include "../../drivers/crutchboard.h"          //For bluetooth crutches
#include "../../drivers/ledstatusindicator.h"
//#include "../../lib/stateestimator.h"
#include "../../lib/hexaloadcells.h"
#include "../../lib/beeper.h"
//#include "../../lib/vibrator.h"
#include "../../lib/binarysignalfilter.h"
#include "../twiice/button.h"                   //For bluetooth crutches


#define TWIICE_VERSION 2 ///< 1: TWIICE 2016, 2: TWIICE 2018 (Dalton).
#define USE_BLUETOOTH_CRUTCHES 0 ///< Set to 1 if the Bluetooth crutches are used, otherwise set to 0.

#define MAIN_LOOP_PERIOD 0.002f ///< Main loop period [s].
#define LOAD_CELLS_UPDATE_PERIOD 2000 // [us].

#define MAX_COMMUNICATION_ERRORS 100

enum ControllerState{INACTIVE, DOUBLE_STANCE, RECORDING, PLAYING, EMERGENCY_STOP, UNDEFINED = -1};

class Echoing : public Controller
{
public:
    /************************** General variables and functions ******************************/
    Echoing(PeripheralsSet peripherals);
    ~Echoing() override;
    void update(float dt) override;

    /*********************** Variables and functions specific to echoing *********************/
    void armMotorBoards();
    void disarmMotorBoards();

    void freeRightLeg();
    void freeLeftLeg();
    void freeAllButPareticKnee();

    void recordRightLegTraj();
    void recordLeftLegTraj();

    void beginTrajPlayback();
    void playTrajectory();

private:
    /*************************** General variables and functions ******************************/
    void updateLoadCells();
    std::thread *loadCellsThread;
    volatile bool stopLoadCellsThread;

    Gpio triggerGpio, exitModeGpio, modeUpGpio, modeDownGpio;

    MotorBoard leftMotorBoard, rightMotorBoard;
    MotorBoardAxis leftHipJoint, leftKneeJoint, rightHipJoint, rightKneeJoint;

    Beeper beeper;

    SpiChannel leftLoadCellsSpiChannel, rightLoadCellsSpiChannel,
               leftFootImuSpiChannel, rightFootImuSpiChannel;
    HexaLoadCells leftLoadCells, rightLoadCells;

    LedStatusIndicator &rgbLed;

    CrutchBoard *leftCrutch, *rightCrutch;
    Button triggerWButton, quitWButton, modeUpWButton, modeDownWButton,
           increaseSlopeWButton, decreaseSlopeWButton,
           riseLeftLegWButton, riseRightLegWButton;

    BinarySignalFilter actionTriggerButton, exitModeButton,
                       modeUpButton, modeDownButton;

    int communicationErrorsCounter; // [].

    /*********************** Variables and functions specific to echoing *********************/
    SyncVarList echoingSyncVars;

    bool areMotorsArmed, clearToArm;
    float buttonPressTime;

    ControllerState prevState, currentState;

    std::vector<float> recordedHipTraj, recordedKneeTraj;       //float vectors for recording the trajectory of the sound side

    bool isRightSideParetic;                                    //determine the paretic side

    float pareticKneeStanceAngle;                               //used to lock the paretic knee during stance
    float rightFootLoad, leftFootLoad, footLoadStanceThreshold;

    float transitionToPlayDuration, transitionToPlaySpeed, transitionToPlayTimer;

    float beepStrength;

    bool loadCellsOverride;              //TEMPORARY! Used to enter foot loads manually for testing purposes
};

typedef Echoing SelectedController;

#endif // ECHOING_H
