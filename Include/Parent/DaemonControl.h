/**
 * @file  DaemonControl.h
 *
 * @brief  Starts the Daemon, sets some of its initial properties, and creates a
 *         PipeReader to receive messages from the daemon.
 */

#pragma once

#include "Pipe_Reader.h"
#include "Pipe_Listener.h"
#include "Pipe_Writer.h"
#include <pthread.h>
#include <vector>
#include <string>

namespace DaemonFramework { class DaemonControl; }

class DaemonFramework::DaemonControl
{
public:
    /**
     * @brief  Configures the controller for its specific daemon on
     *         construction.
     *
     * @param daemonPath      The path to this controller's daemon executable.
     *
     * @param pipeToDaemon    An optional path to a named pipe that the daemon
     *                        will scan for messages from this controller.
     *
     * @param pipeFromDaemon  An optional path to a named pipe that the daemon
     *                        will use to pass messages to this controller.
     *
     * @param bufferSize      The amount of memory in bytes to reserve for any 
     *                        messages sent by the daemon.
     */
    DaemonControl(const std::string daemonPath,
            const std::string pipeToDaemon = "",
            const std::string pipeFromDaemon = "",
            const size_t bufferSize = 0); 

    virtual ~DaemonControl() { }

    /**
     * @brief  If the Daemon isn't already running, this launches the daemon
     *         and opens daemon communication pipes if needed.
     *
     * @param args      An array of strings that will be passed to the daemon as
     *                  launch arguments.
     *
     * @param listener  The object that will handle incoming data if the
     *                  daemon's output pipe is enabled.
     */
    void startDaemon
    (std::vector<std::string> args, Pipe::Listener* keyListener = nullptr);


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
     * @brief  Sends arbitrary data to the daemon using the daemon's named
     *         input pipe, if one exists.
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

protected:
    /**
     * @brief  Creates the pipe file used to send messages to the daemon if it
     *         doesn't already exist.
     *
     *  By default, this creates the pipe file using Pipe::createPipe, setting
     * user permissions to read-only.
     *
     *  Different setups may require pipes with different permissions, or even
     * require the pipe file to be created before the parent application runs.
     * If this is the case, use a DaemonControl subclass that overrides this
     * function.
     *
     * @param pipePath  The path where the daemon's input pipe should be
     *                  created.
     */
    virtual void createDaemonInputPipe(const std::string& pipePath);

    /**
     * @brief  Creates the pipe file used to read messages from the daemon if it
     *         doesn't already exist.
     *
     *  By default, this creates the pipe file using Pipe::createPipe, setting
     * user permissions to write-only.
     *
     *  Different setups may require pipes with different permissions, or even
     * require the pipe file to be created before the parent application runs.
     * If this is the case, use a DaemonControl subclass that overrides this
     * function.
     *
     * @param pipePath  The path where the daemon's output pipe should be
     *                  created.
     */
    virtual void createDaemonOutputPipe(const std::string& pipePath);

private:
    // Daemon executable path:
    const std::string daemonPath;

    // ID of the daemon's process:
    pid_t daemonProcess = 0;

    // Reads data sent by the daemon:
    const std::string outPipePath;
    Pipe::Reader pipeReader;
    const bool readerEnabled;

    // Sends data to the daemon:
    const std::string inPipePath;
    Pipe::Writer pipeWriter;
    const bool writerEnabled;

    // Exit code returned by the completed process:
    int exitCode = 0;
};
