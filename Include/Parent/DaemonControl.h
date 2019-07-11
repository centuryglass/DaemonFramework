/**
 * @file  DaemonControl.h
 *
 * @brief  Starts the Daemon, sets some of its initial properties, and creates a
 *         PipeReader to receive messages from the daemon.
 */

#pragma once
#include "PipeReader.h"
#include <pthread.h>
#include <vector>

class DaemonControl
{
public:
    /**
     * @brief  Saves the paths used to initialize the key pipe and launch the
     *         KeyDaemon.
     *
     * @param Listener  The object that will handle incoming data.
     */
    DaemonControl(PipeReader::Listener* keyListener); 

    virtual ~DaemonControl() { }

    /**
     * @brief  If the Daemon isn't already running, this launches the daemon
     *         and starts listening for data.
     *
     * @param args  A null-terminated array of c-strings that should be passed
     *              to the daemon as launch arguments.
     */
    void startDaemon(const char** args);

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
     * @return  The exit code returned by KeyDaemon on termination.
     */
    int waitToExit();

private:
    // ID of the daemon's process:
    pid_t daemonProcess = 0;
    // ID of the thread that opens the input pipe:
    pthread_t pipeInitThread = 0;
    // Reads data sent by the daemon:
    PipeReader pipeReader;
    // Exit code returned by the completed process:
    int exitCode = 0;
};
