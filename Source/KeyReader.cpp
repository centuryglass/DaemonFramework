#include "KeyReader.h"
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <algorithm>

static const constexpr char* messagePrefix = "KeyDaemon: KeyReader: ";
static const constexpr int eventBufSize = 16;

// Initializes the KeyReader and starts listening for relevant keyboard events.
KeyReader::KeyReader(const char* eventFilePath, std::vector<int> keyCodes,
        Listener* listener) :
    trackedCodes(keyCodes),
    listener(listener),
    threadID(pthread_self())
{
    std::lock_guard<std::mutex> lock(readerMutex);
    errno = 0;
    keyFile = open(eventFilePath, O_RDONLY);
    if (errno != 0)
    {
        std::cerr << messagePrefix << "Failed to open keyboard event file!";
        perror(messagePrefix);
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

    while (keyFile != 0) {
        int eventsRead = 0;
        {
            std::lock_guard<std::mutex> lock(readerMutex);
            if (keyFile != 0)
            {
                errno = 0;
                ssize_t readSize = read(keyFile, events,
                        sizeof(struct input_event) * eventBufSize);
                if (readSize < 0)
                {
                    if (errno != EINTR)
                    {
                        std::cerr << messagePrefix << "Event reading failed:\n";
                        perror(messagePrefix);
                    }
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
            if (events[i].type != EV_KEY || events[i].value < 0
                    || events[i].value >= (int) KeyEventType::trackedTypeCount)
            {
                continue;
            }
            if (std::binary_search(trackedCodes.begin(), trackedCodes.end(),
                        events[i].code))
            {
                listener->keyEvent(events[i].code,
                        (KeyEventType) events[i].value);
            }
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
