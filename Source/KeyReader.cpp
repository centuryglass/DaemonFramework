#include "KeyReader.h"
#include "Debug.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <algorithm>

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: KeyReader::";
#endif

// Initializes the KeyReader and starts listening for relevant keyboard events.
KeyReader::KeyReader(const char* eventFilePath,
        const std::vector<int>& keyCodes, Listener* listener) :
    InputReader(eventFilePath),
    trackedCodes(keyCodes),
    listener(listener)
{
    if (! startReading())
    {
        DBG(messagePrefix << __func__ 
                << ": Failed to start listening for key events from \""
                << eventFilePath << "\"");
    }
    else
    {
        DBG_V(messagePrefix << __func__ 
                << ": Started listening for key events from \"" << eventFilePath
                << "\"");
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
        DBG(messagePrefix << __func__ 
                << ": Failed to open keyboard event file \"" << getPath() 
                << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        return 0;
    }
    DBG_V(messagePrefix << __func__ 
            << ": Opened keyboard event file \"" << getPath() << "\"");
    return keyEventFileDescriptor;
}


// Processes new input from the input file.
void KeyReader::processInput(const int inputBytes)
{
    if (listener == nullptr)
    {
        // There's no point in listening for events if there's no listener to
        // hear them.
        DBG(messagePrefix << __func__ 
                << ": No listener found, ignoring input and closing file \""
                << getPath() << "\"");
        stopReading();
        return;
    }
    const int eventsRead = inputBytes / sizeof(struct input_event);
    DBG_V(messagePrefix << __func__ << ": Read " << eventsRead 
            << " input events from \"" << getPath() << "\":");
    if (eventsRead > 0)
    {
        for (int i = 0; i < eventsRead; i++)
        {
            if (eventBuffer[i].type != EV_KEY || eventBuffer[i].value < 0
                    || eventBuffer[i].value 
                    >= (int) KeyEventType::trackedTypeCount)
            {
                DBG_V(messagePrefix << __func__ << ": Event " << i
                        << ": Ignoring irrelevant event with type "
                        << eventBuffer[i].type << ", value " 
                        << eventBuffer[i].value);
                continue;
            }
            if (std::binary_search(trackedCodes.begin(), trackedCodes.end(),
                        eventBuffer[i].code))
            {
                DBG_V(messagePrefix << __func__ << ": Event " << i
                        << ": Sending tracked event of type "
                        << eventBuffer[i].type << ", value " 
                        << eventBuffer[i].value << ", code "
                        << eventBuffer[i].code << " to Listener.");
                listener->keyEvent(eventBuffer[i].code,
                        (KeyEventType) eventBuffer[i].value);
            }
            else
            {
                DBG_V(messagePrefix << __func__ << ": Event " << i
                        << ": Ignoring event of type " << eventBuffer[i].type
                        << ", value " << eventBuffer[i].value 
                        << " with untracked code " << eventBuffer[i].code);
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
