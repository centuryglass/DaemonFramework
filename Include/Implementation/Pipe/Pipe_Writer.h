/**
 * @file  Pipe_Writer.h
 *
 * @brief  Writes data to a named Unix pipe.
 */

#pragma once
#include "ThreadedInit.h"
#include <mutex>

namespace DaemonFramework { namespace Pipe { class Writer; } }

class DaemonFramework::Pipe::Writer : public ThreadedInit
{
public:
    /**
     * @brief  Saves the named pipe's path, optionally opening it immediately.
     *
     * @param path  The path to a named pipe.
     *
     * @param openNow  Whether the writer should start opening the pipe on
     *                 construction.
     */
    Writer(const char* path, const bool openNow = false);

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
     * @brief  Asynchronously opens the pipe file for writing.
     *
     * This does nothing if the pipe has already been opened.
     */
    void openPipe();

    /**
     * @brief  Closes the pipe file.
     *
     *  Any sendData() calls after the pipe file is closed will be ignored.
     */
    void closePipe();

private:
    /**
     * @brief  Opens the pipe in preparation for writing data.
     *
     * @return  Whether the pipe is now open.
     */
    virtual bool threadedInitAction() override;

    // Named pipe file descriptor:
    int pipeFile = 0;
    // Pipe file path:
    const char* pipePath = nullptr; 
    // Protects the pipe from concurrent access:
    std::mutex lock;
};
