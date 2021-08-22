#include "spi.h"
#include "../config/config.h"
#include "../lib/utils.h"

#include <iostream>

using namespace std;

#define BUFFERS_SIZE 128
#define CHECK_REG_WRITE false

#define SPI_OVERLAY "BB-SPI1-01"

#define SPI_PATH "/dev/spidev1.0"

#define SPI_MISO_PIN "P9.29"
#define SPI_MOSI_PIN "P9.30"
#define SPI_CLK_PIN "P9.31"

const Gpio_Id SpiBus::CS_LEFT_ADC = GPIO3_17;
const Gpio_Id SpiBus::CS_RIGHT_ADC = GPIO0_7;
const Gpio_Id SpiBus::CS_LEFT_FOOT_MPU = GPIO1_17;
const Gpio_Id SpiBus::CS_RIGHT_FOOT_MPU = GPIO3_19;
const Gpio_Id SpiBus::CS_EXT_1 = GPIO2_22;
const Gpio_Id SpiBus::CS_EXT_2 = GPIO2_24;
const Gpio_Id SpiBus::CS_EXT_3 = GPIO0_10;
const Gpio_Id SpiBus::CS_EXT_4 = GPIO0_11;

/**
 * @brief Constructor.
 */
SpiBus::SpiBus()
{
    file = -1;

#ifdef __arm__
    // Set the pins muxing.
    try
    {
        Utils::loadOverlay(SPI_OVERLAY, SPI_PATH);
    }
    catch(runtime_error)
    {
        state = PeripheralState::FAULT;
        return;
    }

    // Initialize the SPI driver.
    file = open(SPI_PATH, O_RDWR);
    if(file < 0)
    {
        state = PeripheralState::FAULT;
        return;
    }

    if(!setSpiMode(SPI_MODE_0))
    {
        state = PeripheralState::FAULT;
        return;
    }

    //
    setSpiMode(SPI_MODE_0);

    //
    state = PeripheralState::ACTIVE;
#else
    state = PeripheralState::DISABLED;
#endif
}

/**
 * @brief Destructor.
 */
SpiBus::~SpiBus()
{
    if(file >= 0)
        close(file);
}

/**
 * @brief Sets the SPI mode.
 * @param mode SPI mode (SPI_MODE_0, SPI_MODE_1, SPI_MODE_2 or SPI_MODE_3).
 * @return true if the operation was successful, false otherwise.
 */
bool SpiBus::setSpiMode(uint8_t mode)
{
    int status;

    // Set the SPI mode.
    status = ioctl(file, SPI_IOC_WR_MODE, &mode);
    if(status < 0)
        return false;

    status = ioctl(file, SPI_IOC_RD_MODE, &mode);
    if(status < 0)
        return false;

    currentSpiMode = mode;

    // Perform a dummy transaction to apply the settings. This is needed because
    // the chipselect line is GPIO and hence not synchronized with the other
    // signals (toggling the chipselect always occurs before).
    uint8_t txData = 0;
    uint8_t rxData;

    spi_ioc_transfer transferConfig;
    memset(&transferConfig, 0, sizeof(transferConfig));

    transferConfig.tx_buf = (__u64)&txData;
    transferConfig.rx_buf = (__u64)&rxData;
    transferConfig.len = 1;
    transferConfig.delay_usecs = 0;
    transferConfig.speed_hz = 1000000;
    transferConfig.bits_per_word = 8;
    transferConfig.cs_change = false;

    status = ioctl(file, SPI_IOC_MESSAGE(1), &transferConfig) ;
    if(status < 0)
        return false;

    return true;
}

/**
 * @brief Performs a SPI full duplex transfer (reads and write simultaneously).
 * @param chipSelect GPIO pin to be used as SPI chipselect.
 * @param txData vector with the bytes to send.
 * @param rxData vector to receive the bytes.
 * @param speed SPI bus speed.
 * @param mode SPI mode (SPI_MODE_0, SPI_MODE_1, SPI_MODE_2 or SPI_MODE_3).
 */
void SpiBus::rawTransfer(const Gpio &chipSelect,
                         const std::vector<uint8_t> txData,
                         std::vector<uint8_t> &rxData, int speed, uint8_t mode)
{
    //
    if(state != PeripheralState::ACTIVE)
        return;

    // Wait until the SPI device is available.
    lock_guard<mutex> deviceLocker(deviceMutex);

    //
    rxData.resize(txData.size());

    // Change the SPI mode if necessary.
    if(mode != currentSpiMode)
        setSpiMode(mode);

    // Set chipselect low.
    ((Gpio &)chipSelect).setPinState(false); // TODO: find a cleaner way.

    // Setup the SPI master peripheral for each byte.
    spi_ioc_transfer transferConfig;
    memset(&transferConfig, 0, sizeof(transferConfig));

    transferConfig.tx_buf = (__u64)txData.data();
    transferConfig.rx_buf = (__u64)rxData.data();
    transferConfig.len = txData.size();
    transferConfig.delay_usecs = 0;
    transferConfig.speed_hz = speed;
    transferConfig.bits_per_word = 8;
    transferConfig.cs_change = false;

    // Transfer (write and read at the same time).
    int status = ioctl(file, SPI_IOC_MESSAGE(1), &transferConfig) ;

    if(status < 0)
    {
        cout << strerror(errno) << endl;
        throw runtime_error("Error during SPI transfer.\n");
    }

    // Set chipselect high.
    ((Gpio &)chipSelect).setPinState(true); // TODO: find a cleaner way.
}

/**
 * @brief Constructor.
 * @param spiBus SPI bus.
 * @param chipselect GPIO pin to be used as the chipselect line.
 * @param speed SPI communication frequency [Hz].
 * @param spiMode SPI mode (SPI_MODE_0, SPI_MODE_1, SPI_MODE_2 or SPI_MODE_3).
 */
SpiChannel::SpiChannel(SpiBus &spiBus, Gpio_Id chipselect, int speed,
                       uint8_t spiMode) :
    spiBus(spiBus), chipselect(chipselect), speed(speed), spiMode(spiMode)
{
#ifdef __arm__
    // Check the status of the chipselect GPIO.
    if(this->chipselect.getState() != PeripheralState::ACTIVE)
        throw runtime_error("SpiChannel: chipselect GPIO not active.");

    // Setup the chipselect pin as output GPIO.
    this->chipselect.setupAsOutput();
    this->chipselect.setPinState(true);

    // Pre-allocate the buffers.
    txBuffer.resize(BUFFERS_SIZE);
    rxBuffer.resize(BUFFERS_SIZE);
#endif
}

/**
 * @brief Gets the SPI bus of this channel.
 * @return a reference to the SPI bus.
 */
SpiBus &SpiChannel::getBus()
{
    return spiBus;
}

/**
 * @brief Set the SPI communication frequency.
 * @param speed SPI clock frequency [Hz].
 */
void SpiChannel::setSpeed(int speed)
{
    this->speed = speed;
}

/**
 * @brief Sets the SPI mode.
 * @param spiMode SPI mode (SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3).
 */
void SpiChannel::setMode(uint8_t spiMode)
{
    this->spiMode = spiMode;
}

/**
 * @brief Transmits and reads data at the same time.
 * @param txData vector of data bytes to send.
 * @param rxData vector to receive the bytes. It will be resized to the size of
 * txData.
 */
void SpiChannel::rawTransfer(const std::vector<uint8_t> txData,
                             std::vector<uint8_t> &rxData)
{
    spiBus.rawTransfer(chipselect, txData, rxData, speed, spiMode);
}

/**
 * @brief Writes a byte to a specified 8-byte register.
 * @param registerAddress address of the register to write.
 * @param data data byte to write (intended value of the register).
 */
void SpiChannel::writeRegister(uint8_t registerAddress, uint8_t data)
{
    txBuffer.resize(2);
    txBuffer[0] = registerAddress & ~(1<<7); // Ensure the register address is 7 bits, set the read/write bit to zero.
    txBuffer[1] = data;

    rawTransfer(txBuffer, rxBuffer);

#if CHECK_REG_WRITE == true
    // Read the register, to check.
    uint8_t readValue = readRegister(chipSelect, registerAddress, speed);
    if(readValue == data)
    {
        debug << hex << "SPI register write success in register "
             << (int)registerAddress << "." << endl;
    }
    else
    {
        debug << hex << "SPI register write failed: read " << (int)readValue
             << ", instead of " << (int)data << "." << endl;
    }
#endif
}

/**
 * @brief Writes several bytes to a register.
 * @param registerAddress address of the register to write.
 * @param data data bytes to write (intended value of the register).
 */
void SpiChannel::writeRegister(uint8_t registerAddress,
                               const std::vector<uint8_t> &data)
{
    txBuffer.resize(1 + data.size());
    txBuffer[0] = registerAddress & ~(1<<7); // Ensure the register address is 7 bits, set the read/write bit to zero.
    std::copy(data.begin(), data.end(), &txBuffer[1]);

    rawTransfer(txBuffer, rxBuffer);

#if CHECK_REG_WRITE == true
    // Read the register, to check.
    vector<uint8_t> readValue;
    readRegister(readValue, chipSelect, registerAddress, speed);
    if(readValue == data)
    {
        debug << hex << "SPI register write success in register "
             << (int)registerAddress << "." << endl;
    }
    else
        debug << hex << "SPI register write failed." << endl;
#endif
}

/**
 * @brief Reads a byte from a 8-bit register.
 * @param registerAddress address of the register to read.
 * @return the register value.
 */
uint8_t SpiChannel::readRegister(uint8_t registerAddress)
{
    txBuffer.resize(2);
    txBuffer[0] = registerAddress | (1<<7); // Ensure the register address is 7 bits, set the read/write bit to one.
    txBuffer[1] = 0;

    rawTransfer(txBuffer, rxBuffer);

    return rxBuffer[1];
}

/**
 * @brief Reads several bytes from a register.
 * @param registerAddress address of the register to read.
 * @param length number of bytes to read.
 * @return a reference to the array containing the bytes read. Its content will
 * be invalid the next time readRegister is called.
 */
const std::vector<uint8_t> &SpiChannel::readRegister(uint8_t registerAddress,
                                                     int length)
{
    // Add one element to both arrays, to send the register address during the
    // first byte exchange.
    rxBuffer.resize(length + 1);
    txBuffer.resize(length + 1);

    // Ensure the register address is 7 bits, set the read/write bit to one.
    txBuffer[0] = registerAddress | (1<<7);

    // Perform the actual SPI transfer.
    rawTransfer(txBuffer, rxBuffer);
    rxBuffer.erase(rxBuffer.begin()); // Remove the first case (register address transmission, no reception).

    return rxBuffer;
}
