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


// Configures how pipe data will be found and processed.
DaemonFramework::Pipe::Reader::Reader
(const char* path, const size_t bufferSize) :
        InputReader(path), listener(nullptr), bufSize(bufferSize)
{
    if (path != nullptr)
    {
        buffer = new unsigned char[bufferSize];
    }
    else
    {
        DF_DBG_V(messagePrefix << __func__
                << ": no path provided, this reader will remain inactive.");
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
void DaemonFramework::Pipe::Reader::openPipe(Listener* listener)
{
    if (! getPath().empty())
    {
        this->listener = listener;
        startInitThread();
    }
}


// Stops the pipe reading thread and closes the pipe.
void DaemonFramework::Pipe::Reader::closePipe()
{
    if (! getPath().empty())
    {
        cancelInit();
        stopReading();
    }
}


// Called by the asynchronous init thread to open the pipe file for reading.
bool DaemonFramework::Pipe::Reader::threadedInitAction()
{
    return startReading();
}


// Opens the pipe file, handling errors and using appropriate file access modes.
int DaemonFramework::Pipe::Reader::openFile()
{
    if (getPath().empty())
    {
        return 0;
    }
    errno = 0;
    int pipeFileDescriptor = open(getPath().c_str(), O_RDONLY);
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
        DF_ASSERT(inputBytes <= bufSize);
    }
    DF_DBG_V(messagePrefix << __func__ << ": Passing " << inputBytes 
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
