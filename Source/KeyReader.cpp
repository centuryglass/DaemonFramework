#include "KeyReader.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <algorithm>

static const constexpr char* messagePrefix = "KeyDaemon: KeyReader: ";
static const constexpr int eventBufSize = 16;

// Initializes the KeyReader and starts listening for relevant keyboard events.
KeyReader::KeyReader(const char* eventFilePath,
        const std::vector<int>& keyCodes, Listener* listener) :
    InputReader(eventFilePath),
    trackedCodes(keyCodes),
    listener(listener)
{
    if (! startReading())
    {
        std::cerr << messagePrefix 
                << "Failed to start listening to key events.";
    }
}


// Opens the input file, handling errors and using appropriate file reading
// options.
int KeyReader::openFile()
{
    errno = 0;
    int keyEventFileDescriptor = open(getPath(), O_RDONLY);
    if (errno != 0)
    {
        std::cerr << messagePrefix << "Failed to open keyboard event file!";
        perror(messagePrefix);
        return 0;
    }
    return keyEventFileDescriptor;
}


// Processes new input from the input file.
void KeyReader::processInput(const int inputBytes)
{
    if (listener == nullptr)
    {
        // There's no point in listening for events if there's no listener to
        // hear them.
        stopReading();
        return;
    }
    const int eventsRead = inputBytes / sizeof(struct input_event);
    if (eventsRead > 0)
    {
        for (int i = 0; i < eventsRead; i++)
        {
            if (eventBuffer[i].type != EV_KEY || eventBuffer[i].value < 0
                    || eventBuffer[i].value 
                    >= (int) KeyEventType::trackedTypeCount)
            {
                continue;
            }
            if (std::binary_search(trackedCodes.begin(), trackedCodes.end(),
                        eventBuffer[i].code))
            {
                listener->keyEvent(eventBuffer[i].code,
                        (KeyEventType) eventBuffer[i].value);
            }
        }
    }
}


// Gets the maximum size in bytes available within the object's file input
// buffer.
int KeyReader::getBufferSize() const
{
    return sizeof(struct input_event) * eventBufSize;
}


// Gets the buffer where the InputReader should read in new file input.
void* KeyReader::getBuffer()
{
    return (void*) eventBuffer;
}
