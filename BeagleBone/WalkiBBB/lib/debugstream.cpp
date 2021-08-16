#include "debugstream.h"
#include "utils.h"

using namespace std;
using namespace Utils;

DebugStream debug;

/**
 * @brief Constructor.
 */
DebugStream::DebugStream()
{
    communication = nullptr;
    timestamp = nullptr;
    flushThread = new thread(&DebugStream::asyncFlush, this);
}

/**
 * @brief Destructor.
 */
DebugStream::~DebugStream()
{
    // Stop the flushing thread.
    if(flushThread != nullptr)
    {
        runFlushThread = false;
        flushThread->join();
        delete flushThread;
    }
}

/**
 * @brief Set the communication object, and start logging to the remote client.
 * @param communication the communication object. If nullptr is given, the debug
 * output will not be sent to the remote client.
 * @note Before the call to this function, all the text sent with the operator<<
 * will never be sent to the remote computer. So, this function should be called
 * as early as possible.
 */
void DebugStream::setCommunication(Communication *communication)
{
    this->communication = communication;
}

/**
 * @brief Set the prefix of the logfile name, and start logging to a file.
 * @param logfilePrefix the prefix of the logfile name.
 * @note Before the call to this function, all the text sent with the operator<<
 * will never be written to the logfile. So, this function should be called as
 * early as possible.
 */
void DebugStream::setLogfilePrefix(std::string logfileBaseName)
{
    // Create the logfile.
    logfileName = logfileBaseName + "_info.txt";
    logfile.open(logfileName, ios::trunc);

    if(!logfile.is_open())
        debug << "DebugStream: could not create the logfile." << endl;
}

/**
 * @brief DebugStream::setTimestamp
 * @param timestamp the main timestamp counter [us].
 * @note Before the call to this function, the text logged will have an invalid
 * timestamp (0), so this method should be called as early as possible.
 */
void DebugStream::setTimestamp(const uint64_t *timestamp)
{
    this->timestamp = timestamp;
}

/**
 * @brief Gets the filename of the current logfile being written.
 * @return the filename of the current logfile.
 */
string DebugStream::getLogfileName() const
{
    return logfileName;
}

/**
 * @brief Sends the buffered debug text to the waiting queue.
 * Gets the text collected in the stream, and send it to the waiting queue, in
 * order for it to be processed soon. The stream is then cleared.
 */
void DebugStream::flush()
{
    // Queue the message along with its timestamp header.
    QueuedLogMessage queuedMessage;

    queuedMessage.message = stringStream.str();

    queuedMessage.timestamp = dateToStr(chrono::system_clock::now(),
                                        "[%Y-%m-%d_%H_%M_%S__");

    if(timestamp)
        queuedMessage.timestamp += to_string(*timestamp) + "] ";
    else
        queuedMessage.timestamp += "0] ";

    messagesQueue.push_back(queuedMessage);

    // Clear the stream.
    stringStream.clear();
    stringStream.str("");
}

/**
 * @brief Sends continuously the content of the waiting queue.
 * This function is meant to be running continuously, in a dedicated thread.
 * It gets the text from the waiting queue, then prints it in the terminal,
 * writes it into the logfile, and sends it to the remote supervisor.
 */
void DebugStream::asyncFlush()
{
    runFlushThread = true;

    while(runFlushThread)
    {
        this_thread::sleep_for(chrono::milliseconds(10));

        while(!messagesQueue.empty())
        {
            // Get the message from the queue.
            string timestampString = messagesQueue.front().timestamp;
            string message = messagesQueue.front().message;
            messagesQueue.pop_front();

            // Print on standard output.
            cout << message;

            // Prefix the message with the date, and write it to the logfile, if
            // available.
            if(logfile.is_open())
            {
                // Split the message into individual lines, if it contains
                // "end of line" characters.
                auto messages = Utils::split(message, '\n');

                // Write the lines to the logfile.
                for(auto m : messages)
                    logfile << timestampString << m << "\n";

                logfile << std::flush;
            }

            // Send to the remote client.
            if(communication != nullptr)
                communication->sendDebugMessage(message);
        }
    }
}

/**
 * @brief Overload of operator<< to handle endl/flush/ends properly.
 * @param fp the stream manipulator.
 * @return A reference to the DebugStream object.
 */
DebugStream& DebugStream::operator<<(DebugStreamManipulator fp)
{
    if(fp == (DebugStreamManipulator)std::endl)
    {
        *this << '\n';
        flush();
    }

    if(fp == (DebugStreamManipulator)std::flush)
        flush();

    if(fp == (DebugStreamManipulator)std::ends)
        *this << '\0';

    return *this;
}
