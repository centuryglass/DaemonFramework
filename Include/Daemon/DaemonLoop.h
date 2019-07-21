/**
 * @file  DaemonLoop.h
 *
 * @brief  An abstract class defining the daemon's main action loop.
 */

#pragma once
#include <cstddef>
#include <atomic>

#ifdef DF_OUTPUT_PIPE_PATH
#include "Pipe_Writer.h"
#endif

#ifdef DF_INPUT_PIPE_PATH
#include "Pipe_Reader.h"
#include "Pipe_Listener.h"
#include "Process_Security.h"
#endif

namespace DaemonFramework { class DaemonLoop; }

/**
 * @brief  Handles the daemon's main behavior loop.
 *
 *  When starting the loop, the daemon subscribes to termination signals, 
 * performs initial security checks, starts the input pipe reader thread
 * if used, and runs the initLoop() fumction.
 *
 *  During each loop iteration, the daemon runs all repeated security checks,
 * calls the loopAction() function, and checks if it received a termination
 * signal. This continues until the termination signal is received, a security
 * check fails, or loopAction returns a nonzero value.
 *
 *  Meanwhile, if enabled, the input pipe will continually read and process data
 * sent by the daemon's parent process, using the handleParentMessage()
 * function. This will occur within a separate thread, so DaemonLoop
 * implementations should take care to properly control access to any data
 * members in both
 * 
 *  When the loop finishes, the daemon closes all pipes, and returns either the
 * value last returned by loopAction(), or an appropriate error code as defined
 * in the ExitCode enum class.
 *
 *  Only one DaemonLoop object should be created for any application. Creating
 * more than one will trigger an immediate runtime error on construction in
 * Debug builds, and will cause undefined behavior in Release builds.
 */
class DaemonFramework::DaemonLoop
#ifdef DF_INPUT_PIPE_PATH
: public Pipe::Listener
#endif
{
public:
    /**
     * @brief  Initializes the DaemonLoop.
     *
     * @param inputBufferSize  The size of the buffer the DaemonLoop should use
     *                         when reading data sent from the parent
     *                         application. If the input pipe path is not
     *                         defined, this value will be ignored.
     */
    DaemonLoop(const int inputBufferSize = 0);

    /**
     * @brief  Ensures all pipes are closed on destruction.
     */
    virtual ~DaemonLoop();

    /**
     * @brief  Starts the daemon's main action loop as long as the loop isn't
     *         already running.
     *
     *  This loop repeatedly checks security conditions, then runs the
     * loopAction() function. The loop runs continually until a security check
     * fails, loopAction() returns a nonzero value, or the daemon reaches the
     * end of its timeout period(if defined).
     *
     * @return  The code the daemon process should return when exiting. This
     *          will either be a value defined in the ExitCode enum class, or a
     *          custom value returned by the loopAction() function.
     */
    int runLoop();

protected:
    /**
     * @brief  Checks if runLoop() has been called already, and the loop is
     *         currently running.
     *
     * @return  Whether the loop is running.
     */
    bool isLoopRunning() const;

#   ifdef DF_OUTPUT_PIPE_PATH
    /**
     * @brief  Sends arbitrary data to the parent process through the daemon's
     *         named output pipe.
     *
     * @param messageData  A generic pointer to a block of memory that holds no
     *                     less than messageSize bytes. The caller is
     *                     responsible for ensuring that this data block is
     *                     valid.
     *
     * @param messageSize  The number of bytes to send from the messageData
     *                     pointer.
     */
    void messageParent(const unsigned char* messageData,
            const size_t messageSize);
#   endif

private:
    /**
     * @brief  Performs any extra initialization required just before the main
     *         daemon loop starts.
     *
     *  This function will only be called once, by the runLoop() function, after
     * opening pipes and rumning standard security checks. If initLoop returns a
     * nonzero value, the runLoop() function will cancel the daemon loop and
     * return the same value.
     *
     * The default implementation immediately returns zero.
     *
     * @return  An exit code to return after cancelling the loop, or zero if the
     *          loop may start.
     */
    virtual int initLoop();

    /**
     * @brief  An abstract function that should be implemented to handle the
     *         daemon's primary action.
     *
     *  This function will be repeatedly called once the runLoop() function is
     * called. Implementations of this function should avoid running for too
     * long, to ensure that the daemon loop gets a chance to regularly follow up
     * on security checks.
     *
     * @return  Zero if the loop should keep running, any other value to stop
     *          the loop. A returned nonzero value will also be used as the
     *          return value of the runLoop() function, so returned code 
     *          definitions should avoid conflicts with the ExitCode enum
     *          class.
     */
    virtual int loopAction() = 0; 

#   ifdef DF_INPUT_PIPE_PATH
    /**
     * @brief  Handles data sent from the daemon's parent process. This function
     *         will be called within the input pipe process.  Implementations
     *         of this function are responsible for validating input, and 
     *         synchronizing any communications between the pipe thread and
     *         the main daemon loop's thread.
     *
     * @param messageData  A pointer to the message data array sent by the
     *                     parent.
     *
     * @param messageSize  The number of bytes available at the messageData
     *                     pointer.
     */
    virtual void handleParentMessage(const unsigned char* messageData,
            const size_t messageSize) = 0;

    /**
     * @brief  Passes data sent from the parent application to the 
     *         handleParentMessage() function.
     *
     * @param data  A raw message data pointer.
     *
     * @param size  The number of bytes available at that data pointer.
     */
    virtual void processData(const unsigned char* data, const size_t size)
            final override;
#   endif

    /**
     * @brief  Sets termSignalReceived when a termination signal is caught.
     *
     * @param signum  The received signal, which should only ever be SIGTERM.
     */
    static void flagTermSignal(int signum);

    // Stores whether the loop is currently running:
    std::atomic_bool loopRunning;
    // Performs daemon process security checks:
    Process::Security securityMonitor;

#   ifdef DF_INPUT_PIPE_PATH
    // Manages the named pipe used to receive data from the parent application:
    Pipe::Reader inputPipe;
#   endif

#   ifdef DF_OUTPUT_PIPE_PATH
    // Manages the named pipe used to send data to the parent application:
    Pipe::Writer outputPipe;
#   endif

#   ifdef DF_LOCK_FILE_PATH
    // File descriptor for the lock file used to ensure only one instance runs:
    int lockFD = 0;
#   endif
};
