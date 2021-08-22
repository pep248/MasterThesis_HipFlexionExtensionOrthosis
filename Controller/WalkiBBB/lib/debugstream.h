#ifndef DEF_LIB_DEBUGSTREAM_H
#define DEF_LIB_DEBUGSTREAM_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <atomic>

#include "../communication.h"

/**
 * @defgroup DebugStream Debug stream
 * @brief Sends debug messages to multiples locations at once.
 * @ingroup Lib
 * @{
 */

typedef std::ostream& (*DebugStreamManipulator)(std::ostream&); ///< Convenience function pointer typedef.

struct QueuedLogMessage
{
    std::string timestamp; // Date/time header in the format "[YYYY-mm-dd_HH_MM_SS__timestamp] ".
    std::string message; // Actual text message.
};

/**
 * @brief Sends debug messages to multiples locations at once.
 * This class provides a global object that is a replacement to cout. All the
 * text sent to this object will be printed on the terminal, logged into a file,
 * and sent to the remote monitoring computer.
 * @ingroup Lib
 */
class DebugStream : public std::ostream
{
    template<typename T>
    friend DebugStream& operator<<(DebugStream& ds, T o);

public:
    DebugStream();
    ~DebugStream();
    void setCommunication(Communication *communication);
    void setLogfilePrefix(std::string logfileBaseName);
    void setTimestamp(uint64_t const *timestamp);
    std::string getLogfileName() const;

    DebugStream& operator<<(DebugStreamManipulator fp);
    void flush(); 

private:
    void asyncFlush();

    Communication *communication; ///< Pointer to the communication object.
    std::string logfileName; ///< Filename of the logfile.
    std::ofstream logfile; ///< Logging file.
    std::ostringstream stringStream; ///< Temporarily converts and holds the text, before it is sent to the three streams.
    uint64_t const *timestamp;

    std::thread *flushThread; ///< Thread to perform the IO operations.
    std::atomic_bool runFlushThread; ///< Set to false to request flushThread to stop.
    std::list<QueuedLogMessage> messagesQueue; ///< Waiting queue of the messages to be processed by flushThread.
};

/**
 * @brief Converts any object to text, and send it to the logging streams.
 * @param ds DebugStream object.
 * @param o the object to convert to text and send.
 * @return A reference to the DebugStream object.
 * @note The debug text is actually buffered and not printed/written/sent, until
 * flush() is called, or flush or endl is given.
 */
template<typename T> DebugStream& operator<<(DebugStream& ds, T o)
{
    ds.stringStream << o;

    return ds;
}

extern DebugStream debug; ///< Global logging object used in the whole program.

/**
 * @}
 */

#endif
