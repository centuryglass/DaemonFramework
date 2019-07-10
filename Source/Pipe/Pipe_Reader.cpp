#include "Pipe_Reader.h"
#include "Pipe_Listener.h"
#include "Debug.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>


#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix 
        = "DaemonFramework::Pipe::Reader::";
#endif


// Configures how pipe data will be found and processed.
DaemonFramework::Pipe::Reader::Reader
(const char* path, Listener* listener, const size_t bufferSize) :
    InputReader(path), listener(listener), bufSize(bufferSize)
{
    buffer = new unsigned char[bufferSize];
}


// Frees the buffer data on destruction.
DaemonFramework::Pipe::Reader::~Reader()
{
    if (buffer != nullptr)
    {
        delete[] buffer;
        buffer = nullptr;
    }
}


// Opens the pipe file, handling errors and using appropriate file access modes.
int DaemonFramework::Pipe::Reader::openFile()
{
    errno = 0;
    int pipeFileDescriptor = open(getPath(), O_RDONLY);
    if (errno != 0)
    {
        DBG(messagePrefix << __func__ 
                << ": Failed to open pipe at path \""
                << getPath() << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        return 0;
    }
    else
    {
        DBG_V(messagePrefix << __func__ << ": Opened pipe at path \""
                << getPath() << "\"");
    }
    return pipeFileDescriptor;
}


// Processes new data from the pipe file.
void DaemonFramework::Pipe::Reader::processInput(const int inputBytes)
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
    DBG_V(messagePrefix << __func__ << ": Sending " << inputBytes 
            << " bytes of data " << " to Listener.");
    listener->processData(buffer, inputBytes);
}


// Gets the maximum size in bytes available within the object's pipe input
// buffer.
int DaemonFramework::Pipe::Reader::getBufferSize() const
{
    return bufSize;
}


// Gets the buffer where the InputReader should read in new pipe data.
void* DaemonFramework::Pipe::Reader::getBuffer()
{
    return (void*) buffer;
}
