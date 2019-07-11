#include "Pipe.h"
#include "Debug.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cstdio>

#ifdef DEBUG
#include <string>
// Print the application and namespace name before all info/error messages:
static const constexpr char* messagePrefix = "DaemonFramework::Pipe::";
#endif

// Attempts to create a new pipe file at a specific path.
bool DaemonFramework::Pipe::createPipe(const char* path, const mode_t mode)
{
    // Check if pipe already exists:
    errno = 0;
    if (access(path, 0) != 0)
    {
        if (errno != ENOENT)
        {
            #ifdef DEBUG
                std::string error(messagePrefix);
                error = error +  __func__ + ": Error when checking pipe path";
                perror(error.c_str());
            #endif
            return false;
        }
    }
    else // File exists, make sure the mode is correct.
    {
        int pipeFD = 0;
        errno = 0;
        pipeFD = open(path, O_RDONLY);
        if (pipeFD == -1)
        {
            #ifdef DEBUG
                std::string error(messagePrefix);
                error = error +  __func__ 
                        + ": Failed to open existing pipe file";
                perror(error.c_str());
            #endif
            return false;
        }
        struct stat pipeInfo = {0};
        errno = 0;
        if (fstat(pipeFD, &pipeInfo) != 0)
        {
            #ifdef DEBUG
                std::string error(messagePrefix);
                error = error +  __func__ 
                        + ": Error when checking existing pipe mode";
                perror(error.c_str());
            #endif
            close(pipeFD);
            return false;
        }
        close(pipeFD);
        if (pipeInfo.st_mode == (S_IFIFO | mode))
        {
            DBG_V(messagePrefix << __func__ 
                    << ": Pipe file already exists with correct mode.");
            return true;
        }
        else
        {
            DBG(messagePrefix << __func__
                    << ": Pipe file exists with incorrect mode!");
            DBG(messagePrefix << __func__ << ": Expected mode: " 
                    << (int) (S_IFIFO | mode) << "(S_IFIFO | mode / "
                    << (int) S_IFIFO << " | " << (int) mode << ")");
            DBG(messagePrefix << __func__ << ": Actual mode: " 
                    << (int) pipeInfo.st_mode);
            return false;
        }
    }
    // Attempt to create pipe:
    errno = 0;
    if (mkfifo(path, mode) != 0)
    {
        DBG(messagePrefix << __func__ 
                << ": Failed to create pipe for Daemon at path \""
                << path << "\"");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        return false;
    }
    DBG_V(messagePrefix << __func__ 
            << ": Created named FIFO pipe for Daemon at path \""
            << path << "\"");
    return true;
}
