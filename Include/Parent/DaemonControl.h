/**
 * @file  DaemonControl.h
 *
 * @brief  Starts the Daemon, sets some of its initial properties, and creates a
 *         PipeReader to receive messages from the daemon.
 */

#pragma once

#ifdef DF_OUTPUT_PIPE_PATH
#include "Pipe_Reader.h"
#include "Pipe_Listener.h"
#endif

#ifdef DF_INPUT_PIPE_PATH
#include "Pipe_Writer.h"
#endif

#include <pthread.h>
#include <vector>
#include <string>

namespace DaemonFramework { class DaemonControl; }

class DaemonFramework::DaemonControl
{
public:

#   ifdef DF_OUTPUT_PIPE_PATH
    /**
     * @brief  If relevant, prepares daemon IO pipe objects on construction.
     *
     * @param listener    The object that will handle incoming data if the
     *                    daemon's output pipe is enabled.
     *
     * @param bufferSize  The amount of memory in bytes to reserve for the
     *                    output pipe's message buffer.
     */
    DaemonControl(Pipe::Listener* keyListener, const size_t bufferSize); 
#   else
    DaemonControl();
#   endif

    virtual ~DaemonControl() { }

    /**
     * @brief  If the Daemon isn't already running, this launches the daemon
     *         and starts listening for data if relevant.
     *
     * @param args  An array of strings that will be passed to the daemon as
     *              launch arguments.
     */
    void startDaemon(std::vector<std::string> args);

    /**
     * @brief  If the Daemon is running, this stops the process and closes the
     *         input pipe.
     */
    void stopDaemon();

    /**
     * @brief  Checks if the daemon is running.
     *
     * @return  Whether the daemon thread is still active.
     */
    bool isDaemonRunning();

#   ifdef DF_INPUT_PIPE_PATH
    /**
     * @brief  Sends arbitrary data to the daemon using the daemon's named
     *         input pipe.
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

    /**
     * @brief  Gets the ID of the daemon process if running.
     *
     * @return  The process ID if the daemon is running, zero otherwise.
     */
    pid_t getDaemonProcessID();

    /**
     * @brief  Gets the exit code returned by the daemon process after it
     *         finished.
     *
     * @return  The process exit code, or 0 if the process is still running.
     */
    int getExitCode();

    /**
     * @brief  Waits until the daemon process terminates and gets the process
     *         exit code.
     *
     * @return  The exit code returned by the daemon on termination.
     */
    int waitToExit();

private:
    // ID of the daemon's process:
    pid_t daemonProcess = 0;

#   ifdef DF_OUTPUT_PIPE_PATH
    // ID of the thread that opens the input pipe:
    pthread_t pipeInitThread = 0;
    // Reads data sent by the daemon:
    Pipe::Reader pipeReader;
#   endif

#   ifdef DF_INPUT_PIPE_PATH
    // Sends data to the daemon:
    Pipe::Writer pipeWriter;
#   endif

    // Exit code returned by the completed process:
    int exitCode;
};
