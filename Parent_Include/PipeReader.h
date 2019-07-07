#pragma once
/**
 * @file  PipeReader.h
 *
 * @brief  Opens the named pipe the KeyDaemon will use to send in key codes.
 */

#include "InputReader.h"
#include "KeyEventType.h"

class PipeReader : public InputReader
{
public:
    /**
     * @brief  Handles key events read by the PipeReader.
     */
    class Listener
    {
    private:
        friend PipeReader;

        /**
         * @brief  Processes a new keyboard input event.
         *
         * @param keyCode  The key code associated with the event.
         *
         * @param type     The type of key event reported.
         */
        virtual void keyEvent(const int keyCode, const KeyEventType type) = 0;
    };

    /**
     * @brief  Opens a new named pipe at the given path on construction, and 
     *         starts listening for input in a new thread.
     *
     * @param listener  The object that will handle keyboard input.
     */
    PipeReader(Listener* listener);

    virtual ~PipeReader() { }

private:
    /**
     * @brief  Opens the pipe file, handling errors and using appropriate file
     *         reading options.
     *
     * @return  A file descriptor for the pipe file, or 0 if opening the pipe
     *          failed.
     */
    virtual int openFile() override;

    /**
     * @brief  Processes new input from the pipe file.
     *
     * Input data will be available in the same buffer returned by getBuffer().
     *
     * @param inputBytes  The number of input bytes read from the file. If this
     *                    value is less than or equal to zero, there was an
     *                    error in reading file input.
     */
    virtual void processInput(const int inputBytes) override;

    /**
     * @brief  Gets the maximum size in bytes available within the object's 
     *         pipe input buffer.
     *
     * @return  Number of bytes available for each input read.
     */
    virtual int getBufferSize() const override;

    /**
     * @brief  Gets the buffer where the InputReader should read in new pipe
     *         input. This buffer must have room for at least getBufferSize()
     *         bytes of data.
     *
     * @return  The object's input buffer pointer.
     */
    virtual void* getBuffer() override;

    // The object that will handle key input events:
    Listener* listener = nullptr;
    // The maximum number of bytes to read from the pipe at one time:
    static const constexpr int bufSize = 6;
    // The buffer where key input codes will be stored:
    char buffer[bufSize];
};
