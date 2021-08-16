#include "capturcontroller.h"

#include "../../lib/debugstream.h"

using namespace std;

/**
 * @brief Constructor.
 * @param peripherals peripherals initialized by main().
 */
CapturController::CapturController(PeripheralsSet peripherals) :
    Controller("Captur", peripherals),
    leftHipEncoderSpiChannel(*peripherals.spiBus, SpiBus::CS_EXT_2),
    leftKneeEncoderSpiChannel(*peripherals.spiBus, SpiBus::CS_EXT_1),
    rightHipEncoderSpiChannel(*peripherals.spiBus, SpiBus::CS_EXT_3),
    rightKneeEncoderSpiChannel(*peripherals.spiBus, SpiBus::CS_EXT_4),
    leftFootLoadCellsSpiChannel(*peripherals.spiBus, SpiBus::CS_LEFT_ADC),
    rightFootLoadCellsSpiChannel(*peripherals.spiBus, SpiBus::CS_RIGHT_ADC),
    leftFootImuSpiChannel(*peripherals.spiBus, SpiBus::CS_LEFT_FOOT_MPU),
    rightFootImuSpiChannel(*peripherals.spiBus, SpiBus::CS_RIGHT_FOOT_MPU),
    leftHipEncoder(leftHipEncoderSpiChannel),
    leftKneeEncoder(leftKneeEncoderSpiChannel),
    rightHipEncoder(rightHipEncoderSpiChannel),
    rightKneeEncoder(rightKneeEncoderSpiChannel),
    leftFootLoadCells(leftFootLoadCellsSpiChannel,
                      "left_foot_load_cells.conf"),
    rightFootLoadCells(rightFootLoadCellsSpiChannel,
                       "right_foot_load_cells.conf"),
    backImu(*peripherals.backImu),
    leftFootImu(leftFootImuSpiChannel, MPU_ACCEL_RANGE_4G,
                MPU_GYRO_RANGE_500DPS, MPU_DLPF_BW_98HZ),
    rightFootImu(rightFootImuSpiChannel, MPU_ACCEL_RANGE_4G,
                 MPU_GYRO_RANGE_500DPS, MPU_DLPF_BW_98HZ),
    trunkOrientationEstimator(ImuOrientation::BACKPACK_VERTICAL_REVERSED,
                              &backImu),
    leftFootEstimator(ImuOrientation::LEFT_FOOT, &leftFootImu),
    rightFootEstimator(ImuOrientation::RIGHT_FOOT, &rightFootImu),
    leftCrutch("left", UART_PORT_A,
               "left_crutch_load_cell.conf", "crutchboard-v2.0-1_imu.conf"),
    rightCrutch("right", UART_PORT_B,
                "right_crutch_load_cell.conf", "crutchboard-v2.0-2_imu.conf")
{
    // Print the sensors status.
    debug << leftHipEncoder.printInitResult("Left hip encoder") << endl;
    debug << rightHipEncoder.printInitResult("Right hip encoder") << endl;
    debug << leftKneeEncoder.printInitResult("Left knee encoder") << endl;
    debug << rightKneeEncoder.printInitResult("Right knee encoder") << endl;

    debug << leftFootLoadCells.printInitResult("Left foot load cells") << endl;
    debug << rightFootLoadCells.printInitResult("Right foot load cells") << endl;

    // Create the SyncVars.
    syncVars.push_back(makeSyncVar<bool>("set_encoders_zero", "", nullptr,
                                         [=](bool)
                                         {
                                             setEncodersZero();
                                         }, false));

    syncVars.push_back(makeSyncVar<bool>("set_load_cells_zero", "", nullptr,
                                         [=](bool)
                                         {
                                             leftFootLoadCells.setZero();
                                             rightFootLoadCells.setZero();
                                         }, false));

    syncVars.push_back(makeSyncVar("left_hip_angle", "deg", leftHipAngle,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("right_hip_angle", "deg", rightHipAngle,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("left_knee_angle", "deg", leftKneeAngle,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("right_knee_angle", "deg", rightKneeAngle,
                                   VarAccess::READ, true));
    syncVars.add("left_foot_load_cells/", leftFootLoadCells.getVars());
    syncVars.add("right_foot_load_cells/", rightFootLoadCells.getVars());

    syncVars.push_back(makeSyncVar("trunk_pitch", "deg", trunkPitch,
                                   VarAccess::READ, true));
    syncVars.push_back(makeSyncVar("trunk_roll", "deg", trunkRoll,
                                   VarAccess::READ, true));

    syncVars.add("left_foot_est/", leftFootEstimator.getSyncVars());
    syncVars.add("right_foot_est/", rightFootEstimator.getSyncVars());

    auto imuVars = backImu.getVars();
    imuVars.getFromName("ax")->setLogToFile(true);
    imuVars.getFromName("ay")->setLogToFile(true);
    imuVars.getFromName("az")->setLogToFile(true);
    imuVars.getFromName("gx")->setLogToFile(true);
    imuVars.getFromName("gy")->setLogToFile(true);
    imuVars.getFromName("gz")->setLogToFile(true);
    syncVars.add("trunk_imu/", imuVars);

    imuVars = leftFootImu.getVars();
    imuVars.getFromName("ax")->setLogToFile(true);
    imuVars.getFromName("ay")->setLogToFile(true);
    imuVars.getFromName("az")->setLogToFile(true);
    imuVars.getFromName("gx")->setLogToFile(true);
    imuVars.getFromName("gy")->setLogToFile(true);
    imuVars.getFromName("gz")->setLogToFile(true);
    syncVars.add("left_foot_imu/", imuVars);

    imuVars = rightFootImu.getVars();
    imuVars.getFromName("ax")->setLogToFile(true);
    imuVars.getFromName("ay")->setLogToFile(true);
    imuVars.getFromName("az")->setLogToFile(true);
    imuVars.getFromName("gx")->setLogToFile(true);
    imuVars.getFromName("gy")->setLogToFile(true);
    imuVars.getFromName("gz")->setLogToFile(true);
    syncVars.add("right_foot_imu/", imuVars);

    syncVars.add("left_crutch/", leftCrutch.getVars());
    syncVars.add("right_crutch/", rightCrutch.getVars());
}

/**
 * @brief Destructor.
 */
CapturController::~CapturController()
{
    // Stop the IMU acquisition thread.
    backImu.stopAutoUpdate();
}

/**
 * @brief Updates the controller state.
 * @param dt time elapsed since the last call to this method [s].
 */
void CapturController::update(float dt)
{
    // Acquire all the sensors.
    leftHipEncoder.update(dt);
    leftKneeEncoder.update(dt);
    rightHipEncoder.update(dt);
    rightKneeEncoder.update(dt);

    leftHipAngle = leftHipEncoder.getAngle();
    leftKneeAngle = leftKneeEncoder.getAngle();
    rightHipAngle = -rightHipEncoder.getAngle();
    rightKneeAngle = -rightKneeEncoder.getAngle();

    backImu.update(dt);
    leftFootImu.update(dt);
    rightFootImu.update(dt);

    leftFootLoadCells.update(dt);
    rightFootLoadCells.update(dt);

    trunkOrientationEstimator.update(dt);
    trunkPitch = trunkOrientationEstimator.getOrientation().y;
    trunkRoll = trunkOrientationEstimator.getOrientation().x;

    leftCrutch.update(dt);
    rightCrutch.update(dt);
}

/**
 * @brief Permanently sets the encoders zero position.
 */
void CapturController::setEncodersZero()
{
    if(leftHipEncoder.setZero() && leftKneeEncoder.setZero() &&
       rightHipEncoder.setZero() && rightKneeEncoder.setZero())
    {
        debug << "All encoders were zeroed correctly." << endl;
    }
    else
        debug << "Zeroing failed for a least one encoder." << endl;
}
