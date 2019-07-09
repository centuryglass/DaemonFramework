/**
 * @file  PipeWriter.h
 *
 * @brief  Sends relevant keyboard codes to the supported application.
 */

#pragma once
#include "KeyReader.h"
#include <mutex>

class PipeWriter;

/**
 * @brief  Sends keyboard input codes to the application supported by the
 *         KeyDaemon.
 *
 *  On construction, PipeWriter opens a named pipe at the given path string. All
 * key codes will be transmitted through this pipe to the supported target
 * application.
 *
 *  If the PipeWriter object cannot open the named pipe, it will print an error
 * message and force close the KeyDaemon.
 */
class PipeWriter : public KeyReader::Listener
{
public:
    /**
     * @brief  Opens the pipe file on construction.
     */
    PipeWriter();

    /**
     * @brief  Closes the pipe on destruction.
     */
    virtual ~PipeWriter();

    /**
     * @brief  Sends a key code to the supported application through the named
     *         pipe. Invalid codes will be ignored.
     *
     * @param code        A valid key event code to transmit. 
     *
     * @param keyPressed  True if the key was pressed, false if the key was
     *                    released.
     */
    virtual void keyEvent(const int code, const KeyEventType type) override;

    /**
     * @brief  Closes the pipe file.
     *
     *  Any keyEvent() calls after the pipe file is closed will be ignored.
     */
    void closePipe();

private:
    // Named pipe file descriptor:
    int pipeFile = 0;
    // Protects the pipe from concurrent access:
    std::mutex lock;
};
