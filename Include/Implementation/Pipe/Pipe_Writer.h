/**
 * @file  Pipe_Writer.h
 *
 * @brief  Writes data to a named Unix pipe.
 */

#pragma once
#include <mutex>

namespace DaemonFramework { namespace Pipe { class Writer; } }

class DaemonFramework::Pipe::Writer
{
public:
    /**
     * @brief  Saves the pipe's path on construction.
     *
     * @param path  The path to a named pipe.
     */
    Writer(const char* path);

    /**
     * @brief  Closes the pipe on destruction.
     */
    virtual ~Writer();

    /**
     * @brief  Sends arbitrary data through the pipe.
     *
     * @param data  A raw data array to send.
     *
     * @param size  The number of bytes to send through the pipe.
     *
     * @return      Whether the data was sent correctly.
     */
    bool sendData(const unsigned char* data, const size_t size);

    /**
     * @brief  Closes the pipe file.
     *
     *  Any sendData() calls after the pipe file is closed will be ignored.
     */
    void closePipe();

private:
    // Named pipe file descriptor:
    int pipeFile = 0;
    // Pipe file path:
    const char* pipePath = nullptr; 
    // Protects the pipe from concurrent access:
    std::mutex lock;
};
