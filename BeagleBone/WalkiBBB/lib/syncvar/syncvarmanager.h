#ifndef DEF_LIB_SYNCVAR_SYNCVARMANAGER_H
#define DEF_LIB_SYNCVAR_SYNCVARMANAGER_H

#include "syncvar.h"
#include "../bytebuffer.h"

#include <thread>

/**
 * @brief Manages the SyncVars list and log their values to a file.
 * @ingroup SyncVar
 */
class SyncVarManager
{
public:
    SyncVarManager(std::string logfileBaseName);
    ~SyncVarManager();

    void add(SyncVar *sv);
    template<typename T>
    void add(std::string name, std::string unit, T &var,
                    VarAccess access, bool logToFile = false);
    void add(std::string prefix, SyncVarList syncVars);
    void add(std::string prefix, Peripheral &peripheral);
    void lockSyncVarsList();
    bool isLocked();

    void update(float dt);
    void log(uint64_t timestamp);

    const SyncVar &getVar(int index) const;
    SyncVar &getVar(int index);
    const SyncVarList &getVars();

    int getLogLineSize() const;

private:
    void asyncWrite();

    SyncVarList syncVars; ///< List of all the Syncvars.
    bool varsListLocked; ///< true if the list is complete and ready for transmission to remote client, false otherwise.
    std::ofstream logfile; ///< Logfile to store the variables values over time.
    uint8_t* logBuffer; ///< Current position in the logfile buffer.
    uint8_t** logBuffers; ///< Dual logfile buffer.
    int currentBuffer; ///< Index of the buffer that is currently being filled.
    int totalLogVarsSize; ///< Size of a log "line", to check quickly if there is enough room in the log buffer [B].

    std::thread *writerThread; ///< Thread dedicated to actually write the data to the logfile, to avoid blocking the main thread.
    volatile bool runWriterThread; ///< True if the writer thread should keep running, false if it should stop.
    volatile uint8_t* writingLogBuffer; ///< Pointer to the logfile to write to the file.
    volatile int nBytesToWriteToFile; ///< Length of the writingLogBuffer data to write on the logfile [B].
};

#endif
