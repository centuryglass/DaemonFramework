/**
 * @file  Pipe_Reader.h
 *
 * @brief  Reads and processes data from a named Unix pipe.
 */

#pragma once
#include "InputReader.h"
#include "ThreadedInit.h"
#include <cstddef>

namespace DaemonFramework
{
    namespace Pipe
    {
        class Listener;
        class Reader;
    }
}

class DaemonFramework::Pipe::Reader : public ThreadedInit, private InputReader
{
public:
    /**
     * @brief  Configures how pipe data will be found and processed, and
     *         optionally asynchronously opens the pipe file.
     *
     * @param path        The path to a named pipe.
     *
     * @param listener    The object that will handle data read from the pipe.
     *
     * @param bufferSize  The number of bytes to allocate for the data buffer.
     */
    Reader(const char* path, Listener* listener, const size_t bufferSize,
            const bool openNow = false);

    /**
     * @brief  Frees the buffer data on destruction.
     */
    virtual ~Reader();

    /**
     * @brief  Asynchronously opens the pipe for reading.
     */
    void openPipe();

    /**
     * @brief  Stops the pipe reading thread and closes the pipe.
     */
    void closePipe();

private:
    /**
     * @brief  Called by the asynchronous init thread to open the pipe file for
     *         reading.
     *
     * @return  Whether the file opened successfully.
     */
    virtual bool threadedInitAction() override;

    /**
     * @brief  Opens the pipe file, handling errors and using appropriate file
     *         access modes.
     *
     * @return  A file descriptor for the pipe file, or 0 if opening the pipe
     *          failed.
     */
    virtual int openFile() override;

    /**
     * @brief  Processes new data the pipe file.
     *
     * Data will be available in the same buffer returned by getBuffer().
     *
     * @param inputBytes  The number of bytes read from the file. If this value
     *                    is less than or equal to zero, there was an error in
     *                    reading file data.
     */
    virtual void processInput(const int inputBytes) override;

    /**
     * @brief  Gets the maximum size in bytes available within the object's 
     *         pipe buffer.
     *
     * @return  Number of bytes available for each read.
     */
    virtual int getBufferSize() const override;

    /**
     * @brief  Gets the buffer where the InputReader should read in new pipe
     *         data. This buffer must have room for at least getBufferSize()
     *         bytes of data.
     *
     * @return  The object's buffer pointer.
     */
    virtual void* getBuffer() override;

    // The object that will handle received data:
    Listener* listener = nullptr;
    // The maximum number of bytes to read from the pipe at one time:
    const size_t bufSize = 0;
    // The buffer where pipe data will be stored:
    unsigned char* buffer = nullptr;
};
