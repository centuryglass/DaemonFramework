#include "Pipe_Writer.h"
#include "Debug.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DF_DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "DaemonFramework::Pipe::Writer::";
#endif

// Maximum number of seconds to wait for initialization when trying to write to
// the pipe:
static const constexpr int writeInitTimeout = 1;

// Saves the named pipe's path, optionally opening it immediately.
DaemonFramework::Pipe::Writer::Writer(const char* path, const bool openNow) :
    pipePath(path)
{
    if (openNow)
    {
        startInitThread();
    }
}


// Closes the pipe on destruction.
DaemonFramework::Pipe::Writer::~Writer()
{
    closePipe();
}


// Sends arbitrary data through the pipe.
bool DaemonFramework::Pipe::Writer::sendData
(const unsigned char* data, const size_t size)
{
    DF_DBG_V(messagePrefix << __func__ << ": Sending " << size
            << " bytes of data.");
    if (! finishedInit())
    {
        const bool success = waitForInit(writeInitTimeout);
        if (! success && ! finishedInit())
        {
            DF_DBG(messagePrefix << __func__ << ": Writing failed, pipe \""
                    << pipePath 
                    << "\" failed to open within timeout period.");
            return false;
        }
    }
    if (! successfulInit())
    {
        DF_DBG(messagePrefix << __func__ << ": Writing failed, pipe \""
                << pipePath << "\" did not open successfully.");
        return false;
    }
    std::lock_guard<std::mutex> pipeLock(lock);
    errno = 0;
    if (write(pipeFile, data, size) == -1)
    {
        DF_DBG(messagePrefix << __func__
            << ": Failed to write data to pipe file.");
        DF_PERROR("Write error type");
        return false;
    }
    return true;
}


// Asynchronously opens the pipe file for writing.
void DaemonFramework::Pipe::Writer::openPipe()
{
    startInitThread();
}


// Closes the pipe file.
void DaemonFramework::Pipe::Writer::closePipe()
{
    cancelInit();
    std::lock_guard<std::mutex> pipeLock(lock);
    DF_DBG_V(messagePrefix << __func__ << ": Closing pipe \"" << pipePath
            << "\"");
    if (pipeFile != 0)
    {
        errno = 0;
        while(close(pipeFile) == -1)
        {
            DF_DBG(messagePrefix << __func__ << ": Error closing pipe:");
            DF_PERROR(pipePath);
            if (errno != EINTR)
            {
                break;
            }
        }
        pipeFile = 0;
    }
    DF_DBG_V(messagePrefix << __func__ << ": Closed pipe \"" << pipePath
            << "\"");
}


// Opens the pipe in preparation for writing data.
bool DaemonFramework::Pipe::Writer::threadedInitAction()
{
    std::lock_guard<std::mutex> pipeLock(lock);
    if (pipeFile == 0)
    {
        DF_DBG_V(messagePrefix << __func__ << ": Opening pipe \"" << pipePath 
                << "\" for initial writing.");
        errno = 0;
        pipeFile = open(pipePath, O_WRONLY); 
        if (errno != 0)
        {
            DF_DBG(messagePrefix << __func__ << ": Failed to open pipe \""
                    << pipePath << "\"");
            DF_PERROR("Pipe opening error");
            pipeFile = 0;
            return false;
        }
        DF_DBG_V(messagePrefix << __func__ << ": Opened pipe \"" << pipePath
                << "\"");
    }
    return true;
}
