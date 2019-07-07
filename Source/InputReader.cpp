#include "InputReader.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <assert.h>

// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: InputReader: ";

// Milliseconds to wait on file input before pausing to see if the thread should
// stop:
static const constexpr int readTimeoutMS = 300;


// Saves the file path and prepares to read the input file.
InputReader::InputReader(const char* path) : path(path) { }


// Stops reading and closes the input file.
InputReader::~InputReader()
{
    stopReading();
}


// Opens the input file and starts the input read loop if not already reading.
bool InputReader::startReading()
{
    {
        std::lock_guard<std::mutex> lock(readerMutex);
        if (currentState == State::closed || currentState == State::failed)
        {
            return false;
        }
        if (currentState != State::initializing)
        {
            return true;
        }
        currentState = State::opening;
    }
    inputFile = openFile();

    {
        std::lock_guard<std::mutex> lock(readerMutex);
        if (inputFile == 0)
        {
            currentState = State::failed;
            std::cerr << messagePrefix << "Failed to open input file at \""
                    << path << "\"\n";
            return false;
        }
        currentState = State::opened;
    }

    int threadError = pthread_create(&threadID, nullptr, threadAction,
            (void *) this);
    if (threadError != 0)
    {
        std::lock_guard<std::mutex> lock(readerMutex);
        std::cerr << messagePrefix << "Couldn't create new thread.\n";
        threadID = 0;
        closeInputFile();
        return false;
    }
    return true;
}


// Ensures that the InputReader is not reading input.
void InputReader::stopReading()
{
    // If on the reader thread, just make sure the event file is closed, and the
    // loop will terminate before it would try the next read call.
    if (pthread_equal(pthread_self(), threadID))
    {
        assert (currentState == State::reading 
                || currentState == State::processing);
        closeInputFile();
        currentState = State::closed; // readerMutex should already be locked
        return;
    }
    std::lock_guard<std::mutex> lock(readerMutex);
    if (currentState != State::closed && currentState != State::failed)
    {
        closeInputFile();
        currentState = State::closed;
    }
}


// Gets the path used to open the input file.
const char* InputReader::getPath() const
{
    return path;
}


// Gets the current state of the input reader.
InputReader::State InputReader::getState()
{
    std::lock_guard<std::mutex> lock(readerMutex);
    return currentState;
}

// Continually waits for and processes input events.
void InputReader::readLoop()
{
    while (inputFile != 0) 
    {
        std::lock_guard<std::mutex> lock(readerMutex);
        currentState = State::reading;
        if (inputFile != 0)
        {
            // Use select to wait for file input until the timeout period ends:
            fd_set readSet;
            fd_set emptyWriteSet, emptyExceptSet;
            FD_ZERO(&readSet);
            FD_ZERO(&emptyWriteSet);
            FD_ZERO(&emptyExceptSet);
            FD_SET(inputFile, &readSet);
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = readTimeoutMS;
            if (select(inputFile + 1, &readSet, &emptyWriteSet, &emptyExceptSet,
                        &timeout) == 1)
            {
                currentState = State::processing;
                errno = 0;
                ssize_t readSize = read(inputFile, getBuffer(),
                        getBufferSize());
                if (errno != 0 || readSize == 0)
                {
                    std::cerr << messagePrefix << "Input reading failed, "
                            << readSize << " bytes apparently read.\n";
                    perror(messagePrefix);
                    closeInputFile();
                    std::cout << messagePrefix << "Closed file "
                            << getPath() << "\n";
                    currentState = State::closed;
                }
                else
                {
                    processInput(readSize);
                }
            }
        }
    }
}


// Closes the input file.
void InputReader::closeInputFile()
{
    if (inputFile != 0)
    {
        close(inputFile);
        inputFile = 0;
    }
}


// Used to start readLoop within a new thread.
void* InputReader::threadAction(void* inputReader)
{
    InputReader* reader = static_cast<InputReader*>(inputReader);
    reader->readLoop();
    return nullptr;
}
