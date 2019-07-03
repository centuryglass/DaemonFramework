#include "InputReader.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>


static const constexpr char* messagePrefix = "KeyDaemon: InputReader: ";


// Saves the file path and prepares to read the input file.
InputReader::InputReader(const char* path) : path(path) { }


// Stops reading and closes the input file.
InputReader::~InputReader()
{
    stopReading();
}


// Checks if the InputReader is currently reading keyboard input events.
bool InputReader::isReading()
{
    return inputFile != 0;
}


// Opens the input file and starts the input read loop if not already reading.
bool InputReader::startReading()
{
    std::lock_guard<std::mutex> lock(readerMutex);
    if (isReading())
    {
        return true;
    }
    inputFile = openFile();
    if (inputFile == 0)
    {
        std::cerr << messagePrefix << "Failed to open input file at \""
                << path << "\"\n";
        return false;
    }
    int threadError = pthread_create(&threadID, nullptr, threadAction,
            (void *) this);
    if (threadError != 0)
    {
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
        closeInputFile();
        return;
    }
    while (! readerMutex.try_lock())
    {
        // thread is currently reading, interrupt it and try again.
        pthread_kill(threadID, SIGINT);
    }
    // mutex locked, close the file and unlock.
    closeInputFile();
    readerMutex.unlock();
}


// Gets the path used to open the input file.
const char* InputReader::getPath() const
{
    return path;
}


// Continually waits for and processes input events.
void InputReader::readLoop()
{
    while (inputFile != 0) 
    {
        int eventsRead = 0;
        {
            std::lock_guard<std::mutex> lock(readerMutex);
            if (inputFile != 0)
            {
                errno = 0;
                ssize_t readSize = read(inputFile, getBuffer(),
                        getBufferSize());
                if (readSize <= 0)
                {
                    if (errno != EINTR && errno != 0)
                    {
                        std::cerr << messagePrefix << "Input reading failed:\n";
                        perror(messagePrefix);
                    }
                    else
                    {
                        std::cout << messagePrefix 
                                << "Stopping interrupted thread.\n";
                    }
                    closeInputFile();
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
