#include "PipeWriter.h"
#include "Debug.h"
#include <linux/input-event-codes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef KEY_PIPE_PATH
  #error "KeyDaemon: PipeWriter: KEY_PIPE_PATH not defined!"
#endif

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: PipeWriter::";
#endif

// Opens the pipe file on construction.
PipeWriter::PipeWriter()
{
    errno = 0;
    pipeFile = open(KEY_PIPE_PATH, O_WRONLY); 
    if (errno != 0)
    {
        DBG(messagePrefix << __func__ << ": Failed to open key code pipe \""
                << KEY_PIPE_PATH << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        pipeFile = 0;
    }
    DBG_V(messagePrefix << __func__ << ": Opened key code pipe \""
            << KEY_PIPE_PATH << "\"");
}


// Closes the pipe on destruction.
PipeWriter::~PipeWriter()
{
    DBG_V(messagePrefix << __func__ << ": Closing key code pipe \""
            << KEY_PIPE_PATH << "\"");
    if (pipeFile != 0)
    {
        close(pipeFile);
        pipeFile = 0;
    }
}


// Sends a key code to the supported application through the named pipe.
// Invalid codes will be ignored.
void PipeWriter::keyEvent(const int code, const KeyEventType type)
{
    std::lock_guard<std::mutex> pipeLock(lock);
    if (pipeFile == 0)
    {
        DBG(messagePrefix << __func__
                << ": Failed to write key code, pipe file isn't open.");
        return;
    }
    if (code <= KEY_RESERVED || code >= KEY_UNKNOWN)
    {
        DBG(messagePrefix << __func__ << ": Code \"" << code
                << "\" is not within the range of valid keyboard codes.");
        return;
    }
    char codeBuffer[5];
    const int charCount = sprintf(codeBuffer, "%d", code);
    if (charCount <= 0)
    {
        DBG(messagePrefix << __func__ << ": Code \"" << code
                << "\" couldn't be converted to string.");
        return;
    }
    if (charCount > 3)
    {
        DBG(messagePrefix << __func__ << ": Code \"" << code
                << "\" was converted to " << charCount 
                << " characters, but should have used no more than three.");
        return;
    }
    codeBuffer[charCount + 1] = '\0';
    codeBuffer[charCount] = (char) type;
    errno = 0;
    if (write(pipeFile, codeBuffer, charCount + 1) == -1)
    {
        DBG(messagePrefix << __func__
            << ": Failed to write key code to pipe file.");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
    }
}
