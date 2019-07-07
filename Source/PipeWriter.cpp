#include "PipeWriter.h"
#include <linux/input-event-codes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifndef KEY_PIPE_PATH
  #error "KeyDaemon: PipeWriter: KEY_PIPE_PATH not defined!"
#endif

static const constexpr char* messagePrefix = "KeyDaemon: PipeWriter: ";

// Opens the pipe file on construction.
PipeWriter::PipeWriter()
{
    errno = 0;
    pipeFile = open(KEY_PIPE_PATH, O_WRONLY); 
    if (errno != 0)
    {
        std::cerr << messagePrefix << "Failed to open key code pipe!\n";
        perror(messagePrefix);
        pipeFile = 0;
    }
}


// Closes the pipe on destruction.
PipeWriter::~PipeWriter()
{
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
        std::cerr << messagePrefix
            << "Failed to write key code, pipe file isn't open.\n";
        return;
    }
    if (code <= KEY_RESERVED || code >= KEY_UNKNOWN)
    {
        std::cerr << messagePrefix << "Error: Code \"" << code
                << "\" is not within the range of valid keyboard codes.\n";
        return;
    }
    char codeBuffer[5];
    const int charCount = sprintf(codeBuffer, "%d", code);
    if (charCount <= 0)
    {
        std::cerr << messagePrefix << "Error: Code \"" << code
                << "\" couldn't be converted to string.\n";
        return;
    }
    if (charCount > 3)
    {
        std::cerr << messagePrefix << "Error: Code \"" << code
                << "\" was converted to " << charCount 
                << " characters, but should have used no more than three.\n";
        return;
    }
    codeBuffer[charCount + 1] = '\0';
    codeBuffer[charCount] = (char) type;
    errno = 0;
    if (write(pipeFile, codeBuffer, charCount + 1) == -1)
    {
        std::cerr << messagePrefix
            << "Failed to write key code to pipe file.\n";
        perror(messagePrefix);
    }
}
