#ifndef DEF_LIB_QUADLOADCELLS_H
#define DEF_LIB_QUADLOADCELLS_H

#include "../drivers/adc124s01.h"
#include "peripheral.h"
#include "vec2.h"
#include "vec4.h"
#include "filters/lowpassfilter.h"
#include "configfile.h"

/**
 * @defgroup QuadLoadCells Quad load cells
 * @brief Acquires four load cells, to compute the center of pressure.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Acquires four load cells, to compute the center of pressure.
 * After initialization, a number of calls (specified in N_OFFSET_SAMPLES)
 * to the update method is used for calibration, during this, the QuadLoadCell
 * will not return any measurements
 * @ingroup Lib
 */
class QuadLoadCells : public Peripheral
{
public:
    QuadLoadCells(SpiChannel &spi, std::string configFileName = "");
    void setZero();
    void update(float dt) override;
    Vec4f getRawChannels();
    Vec4f getFilteredChannels();
    
private:
    Adc124S01 adc; ///< ADC on the foot PCB, to acquire the load cells voltages.
    Vec4f offset; ///< The voltage offset of the four load cells. [V].
    Vec4f lastSamples; ///< Last load cells unfiltered force samples [N].
    std::array<LowPassFilter,4> filteredLoads; ///< Current filtered load values of the cells [N].
    int calibrationCounter; ///< Number of samples acquired, during the calibration process.
    ConfigFile configFile; ///< Configuration file, to load and store the calibration parameters.
};

/**
 * @}
 */

#endif
