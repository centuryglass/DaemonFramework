#include "Pipe_Reader.h"
#include "Pipe_Listener.h"
#include "Debug.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>


#ifdef DF_DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix 
        = "DaemonFramework::Pipe::Reader::";
#endif


// Configures how pipe data will be found and processed, and optionally
// asynchronously opens the pipe file.
DaemonFramework::Pipe::Reader::Reader(const char* path, Listener* listener,
        const size_t bufferSize, const bool openNow) :
        InputReader(path), listener(listener), bufSize(bufferSize)
{
    buffer = new unsigned char[bufferSize];
    if (openNow)
    {
        startInitThread();
    }
}


// Frees the buffer data on destruction.
DaemonFramework::Pipe::Reader::~Reader()
{
    closePipe();
    if (buffer != nullptr)
    {
        delete[] buffer;
        buffer = nullptr;
    }
}


// Asynchronously opens the pipe for reading.
void DaemonFramework::Pipe::Reader::openPipe()
{
    startInitThread();
}


// Stops the pipe reading thread and closes the pipe.
void DaemonFramework::Pipe::Reader::closePipe()
{
    cancelInit();
    stopReading();
}


// Called by the asynchronous init thread to open the pipe file for reading.
bool DaemonFramework::Pipe::Reader::threadedInitAction()
{
    return startReading();
}


// Opens the pipe file, handling errors and using appropriate file access modes.
int DaemonFramework::Pipe::Reader::openFile()
{
    errno = 0;
    int pipeFileDescriptor = open(getPath(), O_RDONLY);
    if (errno != 0)
    {
        DF_DBG(messagePrefix << __func__ 
                << ": Failed to open pipe at path \""
                << getPath() << "\"");
        DF_PERROR("Pipe opening error");
        return 0;
    }
    else
    {
        DF_DBG_V(messagePrefix << __func__ << ": Opened pipe at path \""
                << getPath() << "\"");
    }
    return pipeFileDescriptor;
}


// Processes new data from the pipe file.
void DaemonFramework::Pipe::Reader::processInput(const int inputBytes)
{
    if (listener == nullptr)
    {
        DF_DBG(messagePrefix << __func__ << ": No Listener, closing pipe.");
        stopReading();
        return;
    }
    int code = 0;
    if (inputBytes > bufSize)
    {
        DF_DBG(messagePrefix << __func__ << ": Invalid read size "
                << inputBytes << ", expected < " << bufSize);
        DF_ASSERT(inputBytes < bufSize);
    }
    DF_DBG_V(messagePrefix << __func__ << ": Sending " << inputBytes 
            << " bytes of data to Listener.");
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
