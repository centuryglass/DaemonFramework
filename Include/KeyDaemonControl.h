/**
 * @file  KeyDaemonControl.h
 *
 * @brief  Starts the KeyDaemon, sets some of its initial properties, and 
 *         creates a PipeReader to receive piped key codes.
 */

#pragma once
#include "PipeReader.h"
#include <pthread.h>
#include <vector>

class KeyDaemonControl
{
public:
    /**
     * @brief  Saves the paths used to initialize the key pipe and launch the
     *         KeyDaemon.
     *
     * @param keyListener  The object that will handle incoming key codes.
     */
    KeyDaemonControl(PipeReader::Listener* keyListener); 

    ~KeyDaemonControl() { }

    /**
     * @brief  If the KeyDaemon isn't already running, this launches the daemon
     *         and starts listening for key codes.
     *
     * @param trackedKeyCodeArgs  A null-terminated array of numeric c-strings
     *                            used to set the list of tracked key codes.
     */
    void startDaemon(const char** trackedKeyCodeArgs);

    /**
     * @brief  If the KeyDaemon isn't already running, this launches the daemon
     *         and starts listening for key codes.
     *
     * @param trackedKeyCodeString  The list of key codes the KeyDaemon will
     *                              listen for, as numeric strings separated by
     *                              spaces.
     */
    void startDaemon(const char* trackedKeyCodeString);

    /**
     * @brief  If the KeyDaemon isn't already running, this launches the daemon
     *         and starts listening for key codes.
     *
     * @param trackedKeyCodes  The list of key codes the KeyDaemon will listen
     *                         for.
     */
    void startDaemon(const std::vector<int>& trackedKeyCodes);

    /**
     * @brief  If the KeyDaemon is running, this stops the process and closes
     *         the key input pipe.
     */
    void stopDaemon();

    /**
     * @brief  Checks if the KeyDaemon is running.
     *
     * @return  Whether the KeyDaemon thread is still active.
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
    // ID of the thread that opens the key pipe:
    pthread_t pipeInitThread = 0;
    // Reads key codes sent by the daemon:
    PipeReader pipeReader;
    // Exit code returned by the completed process:
    int exitCode = 0;
};
