#include "quadloadcells.h"
#include "debugstream.h"

using namespace std;

#define LOAD_CELLS_GAIN 1444.9f // [N/V].
#define LPF_TAU 0.010f // [s].
#define N_OFFSET_SAMPLES 100.0f

/**
 * @brief Constructor.
 * @param spi SPI bus used to communicate with the foot ADC.
 * @param chipselect SPI chipselect corresponding to the foot ADC.
 * @param configFileName filename of the config file. It will not be loaded if
 * "" is given.
 */
QuadLoadCells::QuadLoadCells(SpiChannel &spi, string configFileName) :
    adc(spi), filteredLoads({{{LPF_TAU}, {LPF_TAU}, {LPF_TAU}, {LPF_TAU}}}),
    configFile(configFileName)
{
    // Create the SyncVars.
    for(unsigned int i=0; i<filteredLoads.size(); i++)
    {
        syncVars.push_back(makeSyncVar(string(1, 'a'+(char)i), "N",
                                       filteredLoads[i].get(), VarAccess::READ,
                                       true));
    }

    for(unsigned int i=0; i<filteredLoads.size(); i++)
    {
        syncVars.push_back(makeSyncVar("raw_" + string(1, 'a'+(char)i), "N",
                                       lastSamples[i], VarAccess::READ, false));
    }
    
    //
    if(adc.getState() == PeripheralState::ACTIVE)
    {
        //setZero();
        state = PeripheralState::ACTIVE;
    }
    else
        state = adc.getState();

    // Get the calibration values from the config file, if available.
    offset = configFile.get("offsets", VecN<float,4>({0.0f, 0.0f, 0.0f, 0.0f}));
}

/**
 * @brief Starts the calibration.
 * Starts the calibration procedure. For the next N_OFFSET_SAMPLES calls to
 * update(), samples will be recorded, and averaged to compute the load cells
 * offsets. During the calibration, the actual load is assumed to be zero.
 */
void QuadLoadCells::setZero()
{
    state = PeripheralState::CALIBRATING;

    debug << "QuadLoadCells: starting calibration" << endl;

    offset.nullify();
    calibrationCounter = 0;

    for(auto filt : filteredLoads)
        filt.reset(0.0f);
}

/**
 * @brief Acquires the four load cells.
 * For the first N_OFFSET_SAMPLES calls to this method, the LoadCell is
 * calibrated, and does not write any values to lastSamples or xFiltered
 * @param dt the time elapsed since the last call to this function [s].
 * @note Until the calibration is complete, the foot should not touch the
 * ground.
 */
void QuadLoadCells::update(float dt)
{
    adc.update(dt);

    if(state == PeripheralState::CALIBRATING)
    {
        offset += adc.getLastSamples() / N_OFFSET_SAMPLES;
        calibrationCounter++;

        if(calibrationCounter >= N_OFFSET_SAMPLES)
        {
            state = PeripheralState::ACTIVE;
            debug << "QuadLoadCells calibration done." << endl;

            // Save the calibration parameters to the config file.
            configFile.set("offsets", offset);
        }
    }
    else
    {
        lastSamples = (adc.getLastSamples() - offset) * LOAD_CELLS_GAIN;

        for(unsigned int i=0; i<filteredLoads.size(); i++)
            filteredLoads[i].update(lastSamples[i], dt);
    }
}

/**
 * @brief Gets the last measured loads.
 * @return the last measured loads [N].
 * @note This function does not acquire the load cells, so acquire() should be
 * called before.
 */
Vec4f QuadLoadCells::getRawChannels()
{
    return lastSamples;
}

/**
 * @brief Gets the last measured loads, after filtering.
 * @return the last values of the loads filters [N].
 */
Vec4f QuadLoadCells::getFilteredChannels()
{
    return Vec4f(filteredLoads[0].get(),
                 filteredLoads[1].get(),
                 filteredLoads[2].get(),
                 filteredLoads[3].get());
}
