#include "Pipe.h"
#include "Debug.h"
#include "File_Utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <cstdio>

#ifdef DF_DEBUG
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
#           ifdef DF_DEBUG
            std::string error(messagePrefix);
            error = error +  __func__ + ": Error when checking pipe path";
            DF_PERROR(error.c_str());
#           endif
            return false;
        }
    }
    else // File exists, make sure the mode is correct.
    {
        struct stat pipeInfo = {0};
        errno = 0;
        if (stat(path, &pipeInfo) != 0)
        {
            DF_DBG(messagePrefix << __func__ 
                    << ": Error when checking existing pipe mode");
            DF_PERROR("stat error");
            return false;
        }
        if (pipeInfo.st_mode == (S_IFIFO | mode))
        {
            DF_DBG_V(messagePrefix << __func__ 
                    << ": Pipe file already exists with correct mode.");
            return true;
        }
        else
        {
            DF_DBG(messagePrefix << __func__
                    << ": Pipe file exists with incorrect mode!");
            DF_DBG(messagePrefix << __func__ << ": Expected mode: " 
                    << (int) (S_IFIFO | mode) << "(S_IFIFO | mode / "
                    << (int) S_IFIFO << " | " << (int) mode << ")");
            DF_DBG(messagePrefix << __func__ << ": Actual mode: " 
                    << (int) pipeInfo.st_mode);
            return false;
        }
    }
    // Make sure pipe file directory exists:
    std::string pipeDir(File::Utils::parentDir(path));
    if (! pipeDir.empty() && ! File::Utils::createDir(pipeDir))
    {
        DF_DBG(messagePrefix << __func__
                << ": Failed to create pipe directory \"" << pipeDir << "\"");
        return false;
    }
    // Attempt to create pipe:
    errno = 0;
    if (mkfifo(path, mode) != 0)
    {
        DF_DBG(messagePrefix << __func__ 
                << ": Failed to create pipe for Daemon at path \""
                << path << "\"");
        DF_PERROR(messagePrefix);
        return false;
    }
    DF_DBG_V(messagePrefix << __func__ 
            << ": Created named FIFO pipe for Daemon at path \""
            << path << "\"");
    return true;
}
