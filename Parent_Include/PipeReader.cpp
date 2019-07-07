#include "PipeReader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#ifndef KEY_PIPE_PATH
  #error "Parent_Include::PipeReader: KEY_PIPE_PATH must be defined."
#endif


// Opens a new named pipe at the given path on construction, and starts
// listening for input in a new thread.
PipeReader::PipeReader(Listener* listener) :
    InputReader(KEY_PIPE_PATH), listener(listener)
{
    errno = 0;
    if (mkfifo(getPath(), S_IRUSR) != 0)
    {
        perror("Failed to create pipe for KeyDaemon");
    }
}


// Opens the pipe file, handling errors and reading options.
int PipeReader::openFile()
{
    errno = 0;
    int pipeFileDescriptor = open(getPath(), O_RDONLY);
    if (errno != 0)
    {
        perror("Failed to open pipe for KeyDaemon");
        return 0;
    }
    return pipeFileDescriptor;
}


// Processes new input from the pipe file.
void PipeReader::processInput(const int inputBytes)
{
    if (listener == nullptr)
    {
        stopReading();
        return;
    }
    int code = 0;
    if (inputBytes >= bufSize)
    {
        printf("KeyDaemon Parent PipeReader: invalid read size %d\n", inputBytes);
        assert(inputBytes < bufSize);
    }
    for (int i = 0; i < (inputBytes - 1); i++)
    {
        int charValue = buffer[i] - '0'; 
        assert(charValue >= 0 && charValue <= 9);
        code *= 10;
        code += charValue;
    }
    KeyEventType type = (KeyEventType) buffer[inputBytes - 1];
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
