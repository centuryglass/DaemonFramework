/**
 * @file  KeyReader.h
 *
 * @brief  Reads and transmits specific keyboard key events.
 */
#pragma once
#include <pthread.h>
#include <vector>
#include <mutex>

class KeyReader
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
         * @brief  Called whenever the KeyReader detects a key press event.
         *
         * @param keyCode  The code value of a tracked key that was pressed.
         */
        virtual void keyPressed(const int keyCode) = 0;

        /**
         * @brief  Called whenever the KeyReader detects a key release event.
         *
         * @param keyCode  The code value of a tracked key that was released.
         */
        virtual void keyReleased(const int keyCode) = 0;
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
    KeyReader(const char* eventFilePath, std::vector<int> keyCodes,
            Listener* listener);

    /**
     * @brief  Stops event reading and closes the keyboard event file.
     */
    ~KeyReader();

    /**
     * @brief  Checks if the KeyReader is currently reading keyboard input
     *         events.
     *
     * @return  Whether the keyboard event file is open and the loop is running.
     */
    bool isReading();

    /**
     * @brief  Ensures that the KeyReader is not reading keyboard input.
     */
    void stopReading();

private:

    /**
     * @brief  Continually waits for and processes keyboard input events.
     */
    void readLoop();

    /**
     * @brief  Used to start readLoop within a new thread.
     *
     * @param keyReader  A pointer to the KeyReader that will run the loop.
     *
     * @return           An ignored null value.
     */
    static void* threadAction(void* keyReader);

    // The ID of the thread that is/was running the read loop:
    pthread_t threadID = 0;
    // File descriptor for the keyboard input event file:
    int keyFile = 0;
    // List of relevant key codes to report:
    std::vector<int> trackedCodes;
    // Prevents simultaneous access to the keyboard input event file:
    std::mutex readerMutex;
    // Handles reported keyboard events:
    Listener* listener = nullptr;
};
