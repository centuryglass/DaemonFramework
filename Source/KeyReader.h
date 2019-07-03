#pragma once
/**
 * @file  KeyReader.h
 *
 * @brief  Reads and transmits specific keyboard key events.
 */

#include "KeyEventType.h"
#include "InputReader.h"
#include <vector>
#include <linux/input.h>

class KeyReader : public InputReader
{
public:
    /**
     * @brief  Handles key events registered by the KeyReader.
     */
    class Listener
    {
    public:
        virtual ~Listener() { }

        /**
         * @brief  Called whenever the KeyReader detects a key input event.
         *
         * @param keyCode  The code value of a tracked key that was pressed.
         *
         * @param type     The type of key event that was detected.
         */
        virtual void keyEvent(const int keyCode, const KeyEventType type) = 0;
    };

    /**
     * @brief  Initializes the KeyReader and starts listening for relevant
     *         keyboard events.
     *
     * @param eventFilePath  The path to the keyboard's input event file.
     *
     * @param keyCodes       A list of all key event codes that the KeyReader
     *                       should report.
     *
     * @param listener       The object that will handle relevant keyboard
     *                       events.
     */
    KeyReader(const char* eventFilePath, const std::vector<int>& keyCodes,
            Listener* listener);

    virtual ~KeyReader() { }

private:
    /**
     * @brief  Opens the input file, handling errors and using appropriate 
     *         file reading options.
     *
     * @return  A file descriptor for the input file, or <= 0 if opening the
     *          file failed.
     */
    virtual int openFile() override;

    /**
     * @brief  Processes new input from the input file.
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
     *         file input buffer.
     *
     * @return  Number of bytes available for each input read.
     */
    virtual int getBufferSize() const override;

    /**
     * @brief  Gets the buffer where the InputReader should read in new file
     *         input. This buffer must have room for at least getBufferSize()
     *         bytes of data.
     *
     * @return  The object's input buffer pointer.
     */
    virtual void* getBuffer() override;

    // List of relevant key codes to report:
    const std::vector<int>& trackedCodes;
    // Handles reported keyboard events:
    Listener* listener = nullptr;
    // Maximum number of events that can be buffered at once:
    static const constexpr int eventBufSize = 16;
    // Keyboard event input buffer:
    struct input_event eventBuffer[eventBufSize];
};
