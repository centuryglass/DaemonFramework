#include "PipeReader.h"
#include "Debug.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#ifndef KEY_PIPE_PATH
  #error "Parent_Include::PipeReader: KEY_PIPE_PATH must be defined."
#endif

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix 
        = "KeyDaemon Parent: PipeReader::";
#endif


// Opens a new named pipe at the given path on construction, and starts
// listening for input in a new thread.
PipeReader::PipeReader(Listener* listener) :
    InputReader(KEY_PIPE_PATH), listener(listener)
{
    errno = 0;
    if (mkfifo(getPath(), S_IRUSR) != 0)
    {
        DBG(messagePrefix << __func__ 
                << ": Failed to create pipe for KeyDaemon at path \""
                << KEY_PIPE_PATH << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
    }
    else
    {
        DBG_V(messagePrefix << __func__ 
                << ": Created named FIFO pipe for KeyDaemon at path \""
                << KEY_PIPE_PATH << "\"");
    }
}


// Opens the pipe file, handling errors and reading options.
int PipeReader::openFile()
{
    errno = 0;
    int pipeFileDescriptor = open(getPath(), O_RDONLY);
    if (errno != 0)
    {
        DBG(messagePrefix << __func__ 
                << ": Failed to open pipe for KeyDaemon at path \""
                << KEY_PIPE_PATH << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        return 0;
    }
    else
    {
        DBG_V(messagePrefix << __func__ 
                << ": Opened pipe for KeyDaemon at path \"" << KEY_PIPE_PATH
                << "\"");
    }
    return pipeFileDescriptor;
}


// Processes new input from the pipe file.
void PipeReader::processInput(const int inputBytes)
{
    if (listener == nullptr)
    {
        DBG(messagePrefix << __func__ << ": No Listener, closing pipe.");
        stopReading();
        return;
    }
    int code = 0;
    if (inputBytes >= bufSize)
    {
        DBG(messagePrefix << __func__ << ": Invalid read size "
                << inputBytes << ", expected <" << bufSize);
        ASSERT(inputBytes < bufSize);
    }
    for (int i = 0; i < (inputBytes - 1); i++)
    {
        int charValue = buffer[i] - '0'; 
        ASSERT(charValue >= 0 && charValue <= 9);
        code *= 10;
        code += charValue;
    }
    KeyEventType type = (KeyEventType) buffer[inputBytes - 1];
    DBG_V(messagePrefix << __func__ << ": Sending code " << code << ", type "
            << (int) type << " to Listener.");
    listener->keyEvent(code, type);
}


// Gets the maximum size in bytes available within the object's pipe input
// buffer.
int PipeReader::getBufferSize() const
{
    return bufSize;
}


// Gets the buffer where the InputReader should read in new pipe input. This
// buffer must have room for at least getBufferSize() bytes of data.
void* PipeReader::getBuffer()
{
    return (void*) buffer;
}
