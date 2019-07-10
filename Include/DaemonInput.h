/**
 * @file  DaemonInput.h
 *
 * @brief  Sends data to the daemon.
 */

#pragma once
#include <mutex>

class DaemonInput;

/**
 * @brief  Sends arbitrary data from the parent application to the daemon.
 *
 *  On construction, DaemonInput opens a named pipe at a path given on
 * compilation. The parent application may then use the object to send raw data
 * to the daemon process.
 */
class DaemonInput
{
public:
    /**
     * @brief  Opens the pipe file on construction.
     */
    DaemonInput();

    /**
     * @brief  Closes the pipe on destruction.
     */
    virtual ~DaemonInput();

    /**
     * @brief  Sends arbitrary data through the pipe to the daemon.
     *
     * @param data  A raw data array to send.
     *
     * @param size  The number of bytes to send through the pipe.
     */
    virtual void sendData(const unsigned char* data, const size_t size);

    /**
     * @brief  Closes the pipe file.
     *
     *  Any sendData() calls after the pipe file is closed will be ignored.
     */
    void closePipe();

private:
    // Named pipe file descriptor:
    int pipeFile = 0;
    // Protects the pipe from concurrent access:
    std::mutex lock;
};
