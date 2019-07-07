#pragma once
/**
 * @file  InputReader.h
 *
 * @brief  An abstract basis for classes that read new data from an input file
 *         within a new thread.
 */

#include <pthread.h>
#include <vector>
#include <mutex>

class InputReader
{
public:
    /**
     * @brief  Saves the file path and prepares to read the input file.
     *
     * @param path   The path to the input file.
     */
    InputReader(const char* path);

    /**
     * @brief  Stops reading and closes the input file.
     */
    ~InputReader();

    /**
     * @brief  Opens the input file and starts the input read loop if not
     *         already reading.
     *
     * @return  Whether the InputReader successfully started reading input, or
     *          was already reading input.
     */
    bool startReading();

    /**
     * @brief  Ensures that the InputReader is not reading input.
     */
    void stopReading();

    /**
     * @brief  Gets the path used to open the input file.
     *
     * @return  The input file path.
     */
    const char* getPath() const;

    /**
     * @brief  Values used to store the current state of the reader.
     */
    enum class State
    {
        initializing, // Not yet started accessing the input file.
        opening,      // Opening the input file.
        opened,       // Input file open, but not trying to read input yet.
        reading,      // Waiting to read data from the input file.
        processing,   // Handling data read from the input file.
        closed,       // Input file has been closed.
        failed        // Opening the file failed.
    };

    /**
     * @brief  Gets the current state of the input reader.
     *
     * @return  The stored reader state.
     */
    State getState();

private:
    /**
     * @brief  Continually waits for and processes input events.
     */
    void readLoop();

    /**
     * @brief  Opens the input file, handling errors and using appropriate 
     *         file reading options.
     *
     * @return  A file descriptor for the input file, or <= 0 if opening the
     *          file failed.
     */
    virtual int openFile() = 0;

    /**
     * @brief  Processes new input from the input file.
     *
     * Input data will be available in the same buffer returned by getBuffer().
     *
     * @param inputBytes  The number of input bytes read from the file. If this
     *                    value is less than or equal to zero, there was an
     *                    error in reading file input.
     */
    virtual void processInput(const int inputBytes) = 0;

    /**
     * @brief  Closes the input file.
     *
     *  If the input file is already closed, this will do nothing.
     */
    void closeInputFile();

    /**
     * @brief  Gets the maximum size in bytes available within the object's 
     *         file input buffer.
     *
     * @return  Number of bytes available for each input read.
     */
    virtual int getBufferSize() const = 0;

    /**
     * @brief  Gets the buffer where the InputReader should read in new file
     *         input. This buffer must have room for at least getBufferSize()
     *         bytes of data.
     *
     * @return  The object's input buffer pointer.
     */
    virtual void* getBuffer() = 0;

    /**
     * @brief  Used to start readLoop within a new thread.
     *
     * @param keyReader  A pointer to the KeyReader that will run the loop.
     *
     * @return           An ignored null value.
     */
    static void* threadAction(void* keyReader);

    // The file path:
    const char* path;
    // The ID of the thread that is/was running the read loop:
    pthread_t threadID = 0;
    // File descriptor for the input file:
    int inputFile = 0;
    // Current reader state:
    State currentState = State::initializing;
    // Prevents simultaneous access to the input event file:
    std::mutex readerMutex;
};
