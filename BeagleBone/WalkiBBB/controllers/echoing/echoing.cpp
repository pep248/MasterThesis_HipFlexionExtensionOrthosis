/*** To-Do List For The Final Version:
 * - Add safety features
 * - Check if filtering of the recorded position is needed
 * - Remove loadCellsOverride (from .h and .cpp)
 * - Change syncVars permission for rightFootLoad and leftFootLoad to READ from READWRITE
 * - Finalize the beep sounds!
 * - Change TWIICE version
 * - Resolve the issue of switching back and forth between DBL_ST and RECORDING at the threshold
 ******************************************* */

#include "echoing.h"

#include "../../lib/debugstream.h"


using namespace std;
using namespace chrono;
using namespace Utils;      //used for bluetooth crutches

Echoing::Echoing(PeripheralsSet peripherals) :
    Controller("TWIICE Echoing for Stroke", peripherals),
    triggerGpio(GPIO1_28),
    exitModeGpio(GPIO1_29),
    modeUpGpio(GPIO2_22),
    modeDownGpio(GPIO2_24),
    leftMotorBoard(UART_PORT_A, 0.005f),
    rightMotorBoard(UART_PORT_B, 0.005f),
#if TWIICE_VERSION == 1
    leftHipJoint(rightMotorBoard, JOINT_B, 0.0f),
    leftKneeJoint(leftMotorBoard, JOINT_A, 0.0f),
    rightHipJoint(leftMotorBoard, JOINT_B, 0.0f),
    rightKneeJoint(rightMotorBoard, JOINT_A, 0.0f),
#elif TWIICE_VERSION == 2
    leftHipJoint(leftMotorBoard, JOINT_A, -50.0f),
    leftKneeJoint(leftMotorBoard, JOINT_B, 0.0f),
    rightHipJoint(rightMotorBoard, JOINT_B, -50.0f),
    rightKneeJoint(rightMotorBoard, JOINT_A, 0.0f),
#endif
    beeper(PWM_1A),
    leftLoadCellsSpiChannel(*peripherals.spiBus, SpiBus::CS_LEFT_ADC),
    rightLoadCellsSpiChannel(*peripherals.spiBus, SpiBus::CS_RIGHT_ADC),
    leftFootImuSpiChannel(*peripherals.spiBus, SpiBus::CS_LEFT_FOOT_MPU),
    rightFootImuSpiChannel(*peripherals.spiBus, SpiBus::CS_RIGHT_FOOT_MPU),
    leftLoadCells(leftLoadCellsSpiChannel, "left_foot_load_cells_v6.conf"),
    rightLoadCells(rightLoadCellsSpiChannel, "right_foot_load_cells_v6.conf"),
    rgbLed(*peripherals.statusLed),
    triggerWButton(0.05f),
    quitWButton(0.05f),
    modeUpWButton(0.05f),
    modeDownWButton(0.05f),
    increaseSlopeWButton(0.05f),
    decreaseSlopeWButton(0.05f),
    riseLeftLegWButton(0.05f),
    riseRightLegWButton(0.05f),
    actionTriggerButton(0.05f),
    exitModeButton(0.05f),
    modeUpButton(0.05f),
    modeDownButton(0.05f)
{
    // If a motorboard is not detected, abort the program.
    if((leftMotorBoard.getState() != ACTIVE) ||
       (rightMotorBoard.getState() != ACTIVE))
    {
        throw runtime_error("Controller: motorboard not detected!");
    }

    // Add the SyncVars.
    syncVars.add("left_motorboard/", leftMotorBoard.getVars());
    syncVars.add("right_motorboard/", rightMotorBoard.getVars());

#if TWIICE_VERSION == 1
    for(SyncVar* sv : syncVars)
    {
        // Suffix the motorboard vars with hip/knee, which is more intuitive
        // than A/B.
        if(*(sv->getName().end()-2) == '_' && *(sv->getName().end()-1) == 'a')
            sv->setName(sv->getName() + "_knee");

        if(*(sv->getName().end()-2) == '_' && *(sv->getName().end()-1) == 'b')
            sv->setName(sv->getName() + "_hip");
    }
#endif

    syncVars.add("left_load_cells/", leftLoadCells.getVars());
    syncVars.add("right_load_cells/", rightLoadCells.getVars());

    syncVars.push_back(makeSyncVar<bool>("calibrate_load_cells", "", nullptr,
                                         [=](bool calibrate)
                                         {
                                            if(calibrate)
                                            {
                                                leftLoadCells.setZero();
                                                rightLoadCells.setZero();
                                            }
                                         }, false));

    // Check if the feet load cells are functional.
    debug << leftLoadCells.printInitResult("Left load cells") << endl;
    debug << rightLoadCells.printInitResult("Right load cells") << endl;

    // Start the sensors threads.
    loadCellsThread = new thread(&Echoing::updateLoadCells, this);

    struct sched_param sp;
    sp.sched_priority = 2;
    pthread_setschedparam(loadCellsThread->native_handle(), SCHED_RR, &sp);

    // Setup the GPIO of the crutch remote.
    triggerGpio.setupAsInput();
    exitModeGpio.setupAsInput();
    modeUpGpio.setupAsInput();
    modeDownGpio.setupAsInput();

    triggerGpio.setPinState(true);  // Give a correct (neutral) default value if
    exitModeGpio.setPinState(true); // the pin is not readable (simulation or
    modeUpGpio.setPinState(true);   // GPIO driver error). The neutral state is
    modeDownGpio.setPinState(true); // high because of the pullups.

    // Slow down the fan.
    peripherals.caseFan->setDuty(0.1f);
    leftMotorBoard.setVar(VAR_FAN_C_SPEED, 60);

    // Setup the shank IMU.
/*#if USE_FOOT_IMU
    footImu.startAutoUpdate(0.001f);

    imuVars = footImu.getVars();
    imuVars.getFromName("ax")->setLogToFile(true);
    imuVars.getFromName("ay")->setLogToFile(true);
    imuVars.getFromName("az")->setLogToFile(true);
    imuVars.getFromName("gx")->setLogToFile(true);
    imuVars.getFromName("gy")->setLogToFile(true);
    imuVars.getFromName("gz")->setLogToFile(true);
    syncVars.add("foot_imu/", imuVars);

    syncVars.add("foot_pitch", "deg", footOrientation.getOrientation().y,
                 VarAccess::READ, true);
#endif*/

    // Setup the LED.
    peripherals.statusLed->enableControl();
    peripherals.statusLed->setNormalColor(false, true, false);

#if USE_BLUETOOTH_CRUTCHES
    // Setup the Bluetooth crutches.
    // (The first time, these addresses can be obtained with "hcitool scan".)
    execBashCommand("rfcomm bind /dev/rfcomm1 98:D3:21:F4:7D:9D 1");
    execBashCommand("rfcomm bind /dev/rfcomm2 98:D3:A1:F9:6B:D5 1");

    leftCrutch = new CrutchBoard("left", UART_BT_A,
                                 "left_crutch_load_cell.conf",
                                 "left_crutch_imu_conf");
    rightCrutch = new CrutchBoard("right", UART_BT_B,
                                  "right_crutch_load_cell.conf",
                                  "right_crutch_imu_conf");
    leftCrutch->setFailSafeGpioStates({true, true, true, true, true, true, true, true, true});
    rightCrutch->setFailSafeGpioStates({true, true, true, true, true, true, true, true, true});
    syncVars.add("left_crutch/", leftCrutch->getVars());
    syncVars.add("right_crutch/", rightCrutch->getVars());
#endif

    // SyncVars specific to the echoing controller
    echoingSyncVars.push_back(makeSyncVar("pareticRight?", "[0/1]", isRightSideParetic, VarAccess::READWRITE, false));
    echoingSyncVars.push_back(makeSyncVar("clearToArm?", "[0/1]", clearToArm, VarAccess::READWRITE, false));
    echoingSyncVars.push_back(makeSyncVar("footLoadStanceThresh", "[N]", footLoadStanceThreshold, VarAccess::READWRITE, false));
    echoingSyncVars.push_back(makeSyncVar("transitionToPlaySpeed", "[deg/s]", transitionToPlaySpeed, VarAccess::READWRITE, false));
    echoingSyncVars.push_back(makeSyncVar("overRideLoadCells?", "[0/1]", loadCellsOverride, VarAccess::READWRITE, false));

    echoingSyncVars.push_back(makeSyncVar("State", "", currentState, VarAccess::READWRITE, false));

    echoingSyncVars.push_back(makeSyncVar("motorsArmed?", "[0/1]", areMotorsArmed, VarAccess::READ, false));
    echoingSyncVars.push_back(makeSyncVar("rightFootLoad", "[N]", rightFootLoad, VarAccess::READWRITE, true));
    echoingSyncVars.push_back(makeSyncVar("leftFootLoad", "[N]", leftFootLoad, VarAccess::READWRITE, true));
    echoingSyncVars.push_back(makeSyncVar("beepStrength", "[0-1]", beepStrength, VarAccess::READWRITE, true));


    syncVars.add("Controller/", echoingSyncVars);

    // Initialize required echoing variables
    currentState = INACTIVE;
    prevState = UNDEFINED;
    areMotorsArmed = false;
    clearToArm = false;
    buttonPressTime = 0.0f;
    loadCellsOverride = false;
    beepStrength = 0.4f;
    // Important parameters
    footLoadStanceThreshold = 10.0f;        //[N]
    transitionToPlaySpeed = 10.0f;          //[deg/s]

    // Controller ready, play a beep.
    beeper.playSequence({ {880.0f, 0.4f, 0.05f},
                          {880.0f, 0.0f, 0.05f},
                          {880.0f, 0.4f, 0.1f} });


}

Echoing::~Echoing()
{
#if USE_BLUETOOTH_CRUTCHES
    // Delete the Bluetooth crutches interface.
    delete leftCrutch;
    delete rightCrutch;
#endif

    // Stop the sensors threads.
    stopLoadCellsThread = true;
    loadCellsThread->join();
    delete loadCellsThread;
    loadCellsThread = nullptr;
}

void Echoing::update(float dt)
{
    // Update the motorboards.
    leftMotorBoard.update(dt);
    rightMotorBoard.update(dt);

    // If the values received from the motorboard are bogus, emergency stop.
    if(leftMotorBoard.getPosition(JOINT_A) < -180.0f ||
       leftMotorBoard.getPosition(JOINT_A) > 180.0f ||
       leftMotorBoard.getPosition(JOINT_B) < -180.0f ||
       leftMotorBoard.getPosition(JOINT_B) > 180.0f ||
       rightMotorBoard.getPosition(JOINT_A) < -180.0f ||
       rightMotorBoard.getPosition(JOINT_A) > 180.0f ||
       rightMotorBoard.getPosition(JOINT_B) < -180.0f ||
       rightMotorBoard.getPosition(JOINT_B) > 180.0f)
    {
        if(communicationErrorsCounter < MAX_COMMUNICATION_ERRORS)
        {
            communicationErrorsCounter++;
            return; // Ignore the error.
        }
        else
        {
            leftMotorBoard.setVoltage(JOINT_A, 0.0f);
            leftMotorBoard.setVoltage(JOINT_B, 0.0f);
            rightMotorBoard.setVoltage(JOINT_A, 0.0f);
            rightMotorBoard.setVoltage(JOINT_B, 0.0f);

            throw runtime_error("Bogus angles received from the motorboard.");
        }
    }

    // In case of emergency stop of one of the motorboards, go into brake mode.
    if(leftMotorBoard.fault() || rightMotorBoard.fault())
    {
        debug << "Motorboard fault: brake mode enabled." << endl;
        //driveMode = TwiiceMotorDriveMode::BRAKE; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        rgbLed.setNormalColor(true, false, false);
    }

    // In case of high temperature of the motorboard H-bridges, play an alert
    // sound.
    if(leftMotorBoard.overTemperatureWarning() ||
       rightMotorBoard.overTemperatureWarning())
    {
        beeper.beep(1320.0f, 0.02f, 0.5f);
    }

    /*
    //Read and display temperatures
    float temperatureLeftA, temperatureLeftB, temperatureLeftC,
          temperatureRightA, temperatureRightB, temperatureRightC; // [celsius].
    leftMotorBoard.getTemperatures(&temperatureLeftA, &temperatureLeftB,
                                   &temperatureLeftC);
    rightMotorBoard.getTemperatures(&temperatureRightA, &temperatureRightB,
                                    &temperatureRightC);

    static int temperatureDispDecimCounter = 0; // TEMPORARY.
    temperatureDispDecimCounter++;

    if(temperatureDispDecimCounter % 10000 == 0)
    {
        debug << "Temperatures: " << endl
              << "  Windings: " << temperatureLeftA << "°C, "
                                << temperatureLeftB << "°C, "
                                << temperatureRightA << "°C, "
                                << temperatureRightB << "°C." << endl
              << "  Heatsinks: " << temperatureLeftC << "°C, "
                                 << temperatureRightC << "°C." << endl
              << endl;
        temperatureDispDecimCounter = 0;
    }*/

#if USE_BLUETOOTH_CRUTCHES
    // Get the buttons status from the Bluetooth crutches.
    leftCrutch->update(dt);
    rightCrutch->update(dt);

    triggerWButton.update(!leftCrutch->getGpioChannels()[0] | !rightCrutch->getGpioChannels()[0], dt);
    quitWButton.update(!leftCrutch->getGpioChannels()[1] | !rightCrutch->getGpioChannels()[1], dt);
    modeUpWButton.update(!leftCrutch->getGpioChannels()[2] | !rightCrutch->getGpioChannels()[2], dt);
    modeDownWButton.update(!leftCrutch->getGpioChannels()[3] | !rightCrutch->getGpioChannels()[3], dt);
    riseLeftLegWButton.update(!leftCrutch->getGpioChannels()[4] | !rightCrutch->getGpioChannels()[4], dt);
    riseRightLegWButton.update(!leftCrutch->getGpioChannels()[5] | !rightCrutch->getGpioChannels()[5], dt);
    increaseSlopeWButton.update(!leftCrutch->getGpioChannels()[6] | !rightCrutch->getGpioChannels()[6], dt);
    decreaseSlopeWButton.update(!leftCrutch->getGpioChannels()[7] | !rightCrutch->getGpioChannels()[7], dt);
#endif

    // Compute the wired buttons states, with debouncing.
    actionTriggerButton.update(!triggerGpio.getPinState(), dt);
    exitModeButton.update(!exitModeGpio.getPinState(), dt);

    modeUpButton.update(!modeUpGpio.getPinState(), dt);
    modeDownButton.update(!modeDownGpio.getPinState(), dt);

    // Read the loading on each foot from the load cells
    if (!loadCellsOverride) //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    {
        rightFootLoad = rightLoadCells.getFilteredChannels().sum();
        leftFootLoad = leftLoadCells.getFilteredChannels().sum();
    }

    /************************** The main state machine *******************************/

    switch(currentState)
    {
    case INACTIVE:
        if(prevState != currentState)
        {
            debug<<"Entered INACTIVE."<<endl;
            rgbLed.setNormalColor(false, true, false);
            prevState = currentState;
        }

        // Arm the motorboards if not done already:
        if(!areMotorsArmed && clearToArm)
        {
            debug<<"Activating motors..."<<endl;
            armMotorBoards();
        }

        // Free the joints
        freeRightLeg();
        freeLeftLeg();

        // Wait until trigger is pressed for 0.5s, then go to DOUBLE_STANCE
        if(actionTriggerButton.isHigh() || triggerWButton.isPressed())
        {
            buttonPressTime += dt;
            if (buttonPressTime < 0.5f)
            {
                buttonPressTime += dt;
                beeper.beep(440.0f,beepStrength,0.1f);
            }
            else
                beeper.beep(880.0f,beepStrength,0.1f);
        }
        else if(buttonPressTime >= 0.5f)
        {
            buttonPressTime = 0.0f;
            currentState = DOUBLE_STANCE;
        }

        break;

    case DOUBLE_STANCE:
        if(prevState != currentState)
        {
            prevState = currentState;
            debug<<"Entered DOUBLE_STANCE."<<endl;
            rgbLed.setNormalColor(true,true,true);

            //Save the initial paretic knee angle to lock the knee
            if(isRightSideParetic)
                pareticKneeStanceAngle = rightKneeJoint.getPosition();
            else
                pareticKneeStanceAngle = leftKneeJoint.getPosition();
        }

        //Free all joints except for the paretic knee
        freeAllButPareticKnee();

        //If the trigger is pressed and held for 0.5s, return to INACTIVE
        if(actionTriggerButton.isHigh() || triggerWButton.isPressed())
        {
            buttonPressTime += dt;
            if (buttonPressTime < 0.5f)
            {
                buttonPressTime += dt;
                beeper.beep(880.0f,beepStrength,0.1f);
            }
            else
                beeper.beep(440.0f,beepStrength,0.1f);
        }
        else if(buttonPressTime >= 0.5f)
        {
            buttonPressTime = 0.0f;
            currentState = INACTIVE;
        }

        //If one of the feet is unloaded below a threshold, switch to RECORDING/PLAYING state
        else
        {
            if(isRightSideParetic)
            {
                if(leftFootLoad <= footLoadStanceThreshold && rightFootLoad > footLoadStanceThreshold)
                {
                    beeper.beep(220.0f,beepStrength,0.1f);
                    currentState = RECORDING;
                }
                else if(rightFootLoad <= footLoadStanceThreshold && leftFootLoad > footLoadStanceThreshold)
                {
                    beeper.beep(294.0f,beepStrength,0.1f);
                    currentState = PLAYING;
                }
            }
            else
            {
                if(rightFootLoad <= footLoadStanceThreshold && leftFootLoad > footLoadStanceThreshold)
                {
                    beeper.beep(220.0f,beepStrength,0.1f);
                    currentState = RECORDING;
                }
                else if(leftFootLoad <= footLoadStanceThreshold && rightFootLoad > footLoadStanceThreshold)
                {
                    beeper.beep(294.0f,beepStrength,0.1f);
                    currentState = PLAYING;
                }
            }
        }
        break;

    case RECORDING:
        if(prevState != currentState)
        {
            prevState = currentState;
            debug<<"Entered RECORDING."<<endl;
            rgbLed.setNormalColor(true,true,false);
            recordedHipTraj.clear();
            recordedKneeTraj.clear();
        }

        freeAllButPareticKnee();

        if(isRightSideParetic)
        {
            //If the sound leg is loaded, return to double stance mode
            if(leftFootLoad >= footLoadStanceThreshold)
            {
                currentState = DOUBLE_STANCE;
                beeper.beep(220.0f,beepStrength,0.1f);
            }
            //Otherwise, keep recording joint angles
            else
            {
                recordLeftLegTraj();
            }
        }
        else
        {
            //If the sound leg is loaded, return to double stance mode
            if(rightFootLoad >= footLoadStanceThreshold)
            {
                currentState = DOUBLE_STANCE;
                beeper.beep(220.0f,beepStrength,0.1f);
            }
            //Otherwise, keep recording joint angles
            else
            {
                recordRightLegTraj();
            }
        }
        break;

    case PLAYING:
        if(prevState != currentState)
        {
            prevState = currentState;
            debug<<"Entered PLAYING."<<endl;
            rgbLed.setNormalColor(true, false, true);

            beginTrajPlayback();
        }

        //Wait for the transition to finish
        if(transitionToPlayTimer <= transitionToPlayDuration)
        {
            transitionToPlayTimer += dt;
        }
        //While there are still points left in the recorded trajectory
        else if(!recordedHipTraj.empty())
        {
            playTrajectory();
        }
        //Finished playing the trajectory, go back to DOUBLE_STANCE
        else
        {
            currentState = DOUBLE_STANCE;
            beeper.beep(294.0f,beepStrength,0.1f);
        }

        // Free the sound joints
        if(isRightSideParetic)
        {
            freeLeftLeg();
        }
        else
        {
            freeRightLeg();
        }
        break;

    case EMERGENCY_STOP:
        if(prevState != currentState)
        {
            disarmMotorBoards();
            clearToArm = false;
            rgbLed.setNormalColor(true,false,false);
            prevState = currentState;
        }

        break;

    case UNDEFINED:
        //This state will never be entered actually, it's only used for initializing the prevState
        // variable!
        break;
    }

    // Update the beeper.
    beeper.update(dt);
}

void Echoing::armMotorBoards()
{
    rightMotorBoard.armBridges();
    leftMotorBoard.armBridges();
    areMotorsArmed = true;
}


void Echoing::disarmMotorBoards()
{
    rightMotorBoard.disarmBridges();
    leftMotorBoard.disarmBridges();
    areMotorsArmed = false;
}

void Echoing::freeRightLeg()
{
    rightHipJoint.coast();
    rightKneeJoint.coast();
}

void Echoing::freeLeftLeg()
{
    leftHipJoint.coast();
    leftKneeJoint.coast();
}

void Echoing::freeAllButPareticKnee()
{
    if(isRightSideParetic)
    {
        freeLeftLeg();
        rightHipJoint.coast();
        rightKneeJoint.setPosition(pareticKneeStanceAngle);
    }
    else
    {
        freeRightLeg();
        leftHipJoint.coast();
        leftKneeJoint.setPosition(pareticKneeStanceAngle);
    }
}

void Echoing::recordRightLegTraj()
{
    recordedHipTraj.push_back(rightHipJoint.getPosition());
    recordedKneeTraj.push_back(rightKneeJoint.getPosition());
}

void Echoing::recordLeftLegTraj()
{
    recordedHipTraj.push_back(leftHipJoint.getPosition());
    recordedKneeTraj.push_back(leftKneeJoint.getPosition());
}

void Echoing::beginTrajPlayback()
{
    if(isRightSideParetic)
    {
        transitionToPlayDuration = max( abs(rightHipJoint.getPosition() - recordedHipTraj[0])/transitionToPlaySpeed,
                                    abs(rightKneeJoint.getPosition() - recordedKneeTraj[0])/transitionToPlaySpeed);
        rightHipJoint.setPosition(recordedHipTraj[0], transitionToPlayDuration);
        rightKneeJoint.setPosition(recordedKneeTraj[0], transitionToPlayDuration);
    }
    else
    {
        transitionToPlayDuration = max( abs(leftHipJoint.getPosition() - recordedHipTraj[0])/transitionToPlaySpeed,
                                    abs(leftKneeJoint.getPosition() - recordedKneeTraj[0])/transitionToPlaySpeed);
        leftHipJoint.setPosition(recordedHipTraj[0], transitionToPlayDuration);
        leftKneeJoint.setPosition(recordedKneeTraj[0], transitionToPlayDuration);
    }
    recordedHipTraj.erase(recordedHipTraj.begin());
    recordedKneeTraj.erase(recordedKneeTraj.begin());
    transitionToPlayTimer = 0.0f;
}

void Echoing::playTrajectory()
{
    if(isRightSideParetic)
    {
        rightHipJoint.setPosition(recordedHipTraj[0]);
        rightKneeJoint.setPosition(recordedKneeTraj[0]);
    }
    else
    {
        leftHipJoint.setPosition(recordedHipTraj[0]);
        leftKneeJoint.setPosition(recordedKneeTraj[0]);
    }
    recordedHipTraj.erase(recordedHipTraj.begin());
    recordedKneeTraj.erase(recordedKneeTraj.begin());
}

void Echoing::updateLoadCells()
{
    float dt = USEC_TO_SEC(LOAD_CELLS_UPDATE_PERIOD);
    system_clock::time_point nextExecTime = high_resolution_clock::now();

    stopLoadCellsThread = false;

    while(!stopLoadCellsThread)
    {
        //
        auto now = high_resolution_clock::now();

        // Acquire the load cells, and compute the haptic feedback.
        leftLoadCells.update(dt);
        rightLoadCells.update(dt);

        /*leftVibration.update(-leftLoadCells.getRawChannels().sum() / MAX_FOOT_LOAD
                             * vibrationIntensity, dt);
        rightVibration.update(-rightLoadCells.getRawChannels().sum() / MAX_FOOT_LOAD
                              * vibrationIntensity, dt);

        if(testVibLeft == 0.0f && testVibRight == 0.0f)
        {
            leftVibrator.setSpeed(leftVibration.get());
            rightVibrator.setSpeed(rightVibration.get());
        }
        else
        {
            leftVibrator.setSpeed(testVibLeft);
            rightVibrator.setSpeed(testVibRight);
        }

        leftVibrator.update(dt);
        rightVibrator.update(dt);*/

        //
        nextExecTime = now + microseconds(LOAD_CELLS_UPDATE_PERIOD);
        std::this_thread::sleep_until(nextExecTime);
    }
}
