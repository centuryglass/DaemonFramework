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


#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "DaemonFramework::Pipe::Writer::";
#endif

// Opens the pipe file on construction.
DaemonFramework::Pipe::Writer::Writer(const char* path)
{
    errno = 0;
    pipeFile = open(path, O_WRONLY); 
    if (errno != 0)
    {
        DBG(messagePrefix << __func__ << ": Failed to open pipe \"" << path
                << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        pipeFile = 0;
    }
    DBG_V(messagePrefix << __func__ << ": Opened pipe \"" << path << "\"");
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
        DBG(messagePrefix << __func__
                << ": Failed to write data, pipe file isn't open.");
        return false;
    }
    errno = 0;
    if (write(pipeFile, data, size) == -1)
    {
        DBG(messagePrefix << __func__
            << ": Failed to write data to pipe file.");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        return false;
    }
    return true;
}


// Closes the pipe file.
void DaemonFramework::Pipe::Writer::closePipe()
{
    std::lock_guard<std::mutex> pipeLock(lock);
    DBG_V(messagePrefix << __func__ << ": Closing pipe \"" << getPath()
            << "\"");
    if (pipeFile != 0)
    {
        close(pipeFile);
        pipeFile = 0;
    }
}
