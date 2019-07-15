#include "Pipe_Writer.h"
#include "Debug.h"
#include <linux/input-event-codes.h>
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

// Saves the pipe's path on construction.
DaemonFramework::Pipe::Writer::Writer(const char* path) : pipePath(path)
{
    /*
    errno = 0;
    pipeFile = open(path, O_WRONLY); 
    if (errno != 0)
    {
        DF_DBG(messagePrefix << __func__ << ": Failed to open pipe \"" << path
                << "\"");
#       ifdef DF_DEBUG
        perror(messagePrefix);
#       endif
        pipeFile = 0;
    }
    DF_DBG_V(messagePrefix << __func__ << ": Opened pipe \"" << path << "\"");
    */
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
#           ifdef DF_DEBUG
            perror(messagePrefix);
#           endif
            pipeFile = 0;
            return false;
        }
        DF_DBG_V(messagePrefix << __func__ << ": Opened pipe \"" << pipePath
                << "\"");
    }
    errno = 0;
    if (write(pipeFile, data, size) == -1)
    {
        DF_DBG(messagePrefix << __func__
            << ": Failed to write data to pipe file.");
#       ifdef DF_DEBUG
        perror(messagePrefix);
#       endif
        return false;
    }
    return true;
}


// Closes the pipe file.
void DaemonFramework::Pipe::Writer::closePipe()
{
    std::lock_guard<std::mutex> pipeLock(lock);
    DF_DBG_V(messagePrefix << __func__ << ": Closing pipe \"" << pipePath
            << "\"");
    if (pipeFile != 0)
    {
        close(pipeFile);
        pipeFile = 0;
    }
}
