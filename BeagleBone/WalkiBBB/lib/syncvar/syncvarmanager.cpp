#include "syncvarmanager.h"

#include <algorithm>

#include "../debugstream.h"

using namespace std;

#define WRITE_BUFFERS_SIZE 6000000 // Can hold 50 float values at 1kHz during 30s.

/**
 * @brief Constructor.
 * @param logfileBaseName the filename prefix of the logfile. This includes the
 * directory and the beginning of the basename.
 */
SyncVarManager::SyncVarManager(string logfileBaseName)
{
    logBuffers = new uint8_t*[2];
    logBuffers[0] = new uint8_t[WRITE_BUFFERS_SIZE];
    logBuffers[1] = new uint8_t[WRITE_BUFFERS_SIZE];

    varsListLocked = false;
    currentBuffer = 0;
    logBuffer = &logBuffers[currentBuffer][0];
    writingLogBuffer = nullptr;
    writerThread = nullptr;
    nBytesToWriteToFile = 0;

    // Create the logfile.
    logfile.open(logfileBaseName + "_vars.wkv", ios::trunc | ios::binary);

    if(!logfile.is_open())
        debug << "SyncVarManager: could not create the logfile." << endl;
}

/**
 * @brief Destructor.
 */
SyncVarManager::~SyncVarManager()
{
    // Stop the writer thread.
    if(writerThread != nullptr)
    {
        runWriterThread = false;
        writerThread->join();
        delete writerThread;
    }

    // Delete the write buffers.
    delete[] logBuffers[0];
    delete[] logBuffers[1];
    delete[] logBuffers;
}

/**
 * @brief Adds an existing SyncVar to the list.
 * @param sv the SyncVar to add to the list, without copy.
 * @note If the SyncVars list has been locked, then this function does nothing.
 */
void SyncVarManager::add(SyncVar *sv)
{
    if(!varsListLocked)
        syncVars.push_back(sv);
    else
    {
        debug << "Can't add " << sv->getName()
              << " SyncVar, because the list has been locked." << endl;
    }
}

/**
 * @brief Adds several SyncVars to the list.
 * @param prefix string to be prepended to the name of each SyncVar of the given
 * list.
 * @param syncVars SyncVars to add to the list.
 * @note The SyncVars full name (existing name + prefix) cannot be longer than
 * SYNCVAR_NAME_COMM_LENGTH. If the given string is longer, the last characters
 * will be removed in order to fit.
 * @note If the SyncVars list has been locked, then this function does nothing.
 */
void SyncVarManager::add(std::string prefix, SyncVarList syncVars)
{
    if(!varsListLocked)
        this->syncVars.add(prefix, syncVars);
    else
    {
        debug << "Can't add " << prefix << " SyncVars, because the list has "
             << "been locked." << endl;
    }
}

/**
 * @brief Adds all the peripheral SyncVars to the list.
 * @param prefix string to be prepended to the name of each SyncVar of the
 * peripheral.
 * @param peripheral Peripheral object to add the SyncVars from.
 * @note The SyncVars full name (existing name + prefix) cannot be longer than
 * SYNCVAR_NAME_COMM_LENGTH. If the given string is longer, the last characters
 * will be removed in order to fit.
 * @note If the SyncVars list has been locked, then this function does nothing.
 */
void SyncVarManager::add(std::string prefix, Peripheral &peripheral)
{
    if(!varsListLocked)
        this->syncVars.add(prefix, peripheral.getVars());
    else
    {
        debug << "Can't add " << prefix << " SyncVars, because the list has "
             << "been locked." << endl;
    }
}

/**
 * @brief Locks the SyncVars list.
 * Locks the SyncVars list, so it will not be possible anymore to add more.
 * All the following calls to add*() will do nothing.
 */
void SyncVarManager::lockSyncVarsList()
{
    varsListLocked = true;

    // Write the logfile header line, with the name of the variables.
    if(!logfile.is_open())
        return;

    uint16_t nLoggedVars = count_if(syncVars.begin(), syncVars.end(),
                                [&](SyncVar* sv) {return sv->getLogToFile();});

    ByteBuffer fileHeader;

    fileHeader << (uint16_t) (1 + nLoggedVars);

    fileHeader << string("timestamp") + '\0'
               << string("us") + '\0'
               << (uint8_t) VarType::UINT64
               << (uint32_t)sizeof(uint64_t);

    totalLogVarsSize = sizeof(uint64_t); // Timestamp.

    for(SyncVar* sv : syncVars)
    {
        if(sv->getLogToFile())
        {
            fileHeader << sv->getName() + '\0'
                       << sv->getUnit() + '\0'
                       << (uint8_t) sv->getType()
                       << sv->getLength();

            totalLogVarsSize += sv->getLength();
        }
    }

    logfile.write((const char*)fileHeader.data(), fileHeader.size());

    // Start the writer thread.
    writerThread = new thread(&SyncVarManager::asyncWrite, this);
}

/**
 * @brief Checks if the SyncVars list has been locked.
 * @return true if the SyncVars list has been locked, false otherwise.
 */
bool SyncVarManager::isLocked()
{
    return varsListLocked;
}

/**
 * @brief Updates the values of the SyncVars in the list.
 * This updates the value of the SyncVars that are being changed smoothly.
 * @param dt time elapsed since the last call to this method [s].
 */
void SyncVarManager::update(float dt)
{
    for(SyncVar* sv : syncVars)
        sv->update(dt);
}

/**
 * @brief Logs the current SyncVars state to the logfile.
 * Adds a new line to the logfile, with the timestamp and the value of each
 * SyncVar.
 * @param timestamp current timestamp [us].
 */
void SyncVarManager::log(uint64_t timestamp)
{
    if(!logfile.is_open())
        return;

    // Check that there is enough space in the current write buffer to write the
    // new state of all the variables logged to file.
    if(logBuffer + totalLogVarsSize >=
       &logBuffers[currentBuffer][WRITE_BUFFERS_SIZE])
    {
        debug << "The SyncVars logfile buffer is full." << endl;
        logBuffer = &logBuffers[currentBuffer][0];
        return;
    }

    // Write the current state of the logged variables to the write buffer.
    memcpy(logBuffer, &timestamp, sizeof(timestamp));
    logBuffer += sizeof(timestamp);

    for(SyncVar* sv : syncVars)
    {
        if(sv->getLogToFile())
        {
            memcpy(logBuffer, sv->getData(), sv->getLength());
            logBuffer += sv->getLength();
        }
    }

    // If the writer thread is idle, request writing the current buffer to the
    // logfile, then switch to the other one.
    if(nBytesToWriteToFile == 0)
    {
        writingLogBuffer = &logBuffers[currentBuffer][0];
        nBytesToWriteToFile = logBuffer - writingLogBuffer;

        currentBuffer = !currentBuffer; // Switch to the other buffer.
        logBuffer = &logBuffers[currentBuffer][0];
    }
}

/**
 * @brief Gets a SyncVar from its index.
 * @param index the index of the SyncVar in the list.
 * @return A constant reference to the Syncvar.
 * @warning This function currently does not check if the index is valid.
 */
const SyncVar &SyncVarManager::getVar(int index) const
{
    if(varsListLocked)
        return *syncVars[index];
    else
        throw runtime_error("SyncVarManager::getVars(): list is not locked.");
}

/**
 * @brief Gets a SyncVar from its index.
 * @param index the index of the SyncVar in the list.
 * @return A reference to the Syncvar.
 * @warning This function currently does not check if the index is valid.
 */
SyncVar &SyncVarManager::getVar(int index)
{
    if(varsListLocked)
        return *syncVars[index];
    else
        throw runtime_error("SyncVarManager::getVars(): list is not locked.");
}

/**
 * @brief Gets the size of a log "line".
 * @return The amount of data logged at every timestep [B].
 * @throw A runtime_error is thrown if this method is called before the list
 * is locked.
 */
int SyncVarManager::getLogLineSize() const
{
    if(varsListLocked)
        return totalLogVarsSize;
    else
    {
        throw runtime_error("SyncVarManager::getLogLineSize(): list is not "
                            "locked.");
    }
}

/**
 * @brief Continuously writes the designated data to a file.
 * First set writingLogBuffer to select the buffer to be written to the file,
 * then set nBytesToWriteToFile to set the number of bytes of this buffer to
 * write (data size). nBytesToWriteToFile will be set to zero by this function,
 * when the write operation is finished.
 * This function runs until nBytesToWriteToFile is set to false.
 */
void SyncVarManager::asyncWrite()
{
    runWriterThread = true;

    while(runWriterThread)
    {
        // If a buffer is ready for writing, write it.
        if(writingLogBuffer != nullptr && nBytesToWriteToFile > 0)
        {
            logfile.write((const char*)writingLogBuffer, nBytesToWriteToFile);
            nBytesToWriteToFile = 0;
        }

        //
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

/**
 * @brief Gets the full SyncVars list.
 * @return the SyncVars list.
 */
const SyncVarList &SyncVarManager::getVars()
{
    if(varsListLocked)
        return syncVars;
    else
        throw runtime_error("SyncVarManager::getVars(): list is not locked.");
}
