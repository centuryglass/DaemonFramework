#include "KeyReader.h"
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <algorithm>

static const constexpr char* messagePrefix = "KeyReader: ";
static const constexpr int eventBufSize = 16;

// Initializes the KeyReader and starts listening for relevant keyboard events.
KeyReader::KeyReader(const char* eventFilePath, std::vector<int> keyCodes,
        Listener* listener) :
    trackedCodes(keyCodes),
    listener(listener),
    threadID(pthread_self())
{
    std::lock_guard<std::mutex> lock(readerMutex);
    keyFile = open(eventFilePath, O_RDONLY);
    bool errorFound = true;
    switch (keyFile)
    {
        case EACCES:
            std::cerr << messagePrefix 
                    << "Unable to read the keyboard event file.\n";
            break;
        case EINTR:
            std::cerr << messagePrefix
                << "Opening keyboard event file was interrupted.\n";
            break;
        case ENFILE:
            std::cerr << messagePrefix
                << "Too many files open for the system to open another.\n";
            break;
        case ENOENT:
            std::cerr << messagePrefix << eventFilePath 
                << " does not exist.\n";
            break;
        default:
            errorFound = false;
    };
    if (errorFound)
    {
        keyFile = 0;
        return;
    }
    // Make sure key codes are sorted for fast code lookup:
    std::sort(trackedCodes.begin(), trackedCodes.end());

    int threadError = pthread_create(&threadID, nullptr, threadAction,
            (void *) this);
    if (threadError != 0)
    {
        std::cerr << messagePrefix << "Couldn't create new thread.\n";
        close(keyFile);
        keyFile = 0;
        threadID = 0;
    }
}


// Stops event reading and closes the keyboard event file.
KeyReader::~KeyReader()
{
    stopReading();
}


// Checks if the KeyReader is currently reading keyboard input events.
bool KeyReader::isReading()
{
    return keyFile != 0;
}


// Ensures that the KeyReader is not reading keyboard input.
void KeyReader::stopReading()
{
    // If on the reader thread, just make sure the event file is closed, and the
    // loop will terminate before it would try the next read call.
    if (pthread_equal(pthread_self(), threadID))
    {
        if (keyFile != 0)
        {
            close(keyFile);
            keyFile = 0;
        }
        return;
    }
    while (! readerMutex.try_lock())
    {
        // thread is currently reading, interrupt it and try again.
        pthread_kill(threadID, SIGINT);
    }
    // mutex locked, close the file and unlock.
    if (keyFile != 0)
    {
        close(keyFile);
        keyFile = 0;
    }
    readerMutex.unlock();
}


// Continually waits for and processes keyboard input events.
void KeyReader::readLoop()
{
    if (listener == nullptr)
    {
        // There's no point in listening for events if there's no listener to
        // hear them.
        std::lock_guard<std::mutex> lock(readerMutex);
        if (keyFile != 0)
        {
            close(keyFile);
            keyFile = 0;
        }
        return;
    }
    unsigned char eventBits = 0;
    unsigned long lastKeyBits = 0;
    struct input_event events[eventBufSize];

    while (keyFile != 0)
    {
        int eventsRead = 0;
        {
            std::lock_guard<std::mutex> lock(readerMutex);
            if (keyFile != 0)
            {
                ssize_t readSize = read(keyFile, events,
                        sizeof(struct input_event) * eventBufSize);
                if (readSize < 0)
                {
                    std::cerr << messagePrefix << "Event reading failed.\n";
                    close(keyFile);
                    keyFile = 0;
                }
                else
                {
                    eventsRead = readSize / sizeof(struct input_event);
                }
            }
        }
        for (int i = 0; i < eventsRead; i++)
        {
            if (events[i].type != EV_KEY)
            {
                continue;
            }
            //if (std::binary_search(trackedCodes.begin(), trackedCodes.end(),
            //            events[i].code))
            //{
                if (events[i].value == 1)
                {
                    listener->keyPressed(events[i].code);
                }
                else if (events[i].value == 0)
                {
                    listener->keyReleased(events[i].code);
                }
            //}
        }
    }
}


// Used to start readLoop within a new thread.
void* KeyReader::threadAction(void* keyReader)
{
    KeyReader* reader = static_cast<KeyReader*>(keyReader);
    reader->readLoop();
    return nullptr;
}
