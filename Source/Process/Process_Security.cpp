#include "Process_Security.h"
#include "Process_State.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

static const constexpr char* errorPrefix = "KeyDaemon: Process::Security::";

// Loads process data on construction.
Process::Security::Security()
{
    appProcess = Data(getpid());
    if (appProcess.isValid())
    {
        parentProcess = Data(appProcess.getParentId());
    }
}


// Checks if this application's process is secure.
bool Process::Security::appProcessSecured()
{
#ifdef INSTALL_PATH
    std::string installPath(INSTALL_PATH);
    appProcess.update();
    if (! processSecured(appProcess, installPath))
    {
        std::cerr << errorPrefix << __func__
                << ": This application's process is insecure.\n";
        return false;
    }
    return true;
#else
    std::cerr << errorPrefix << __func__
        << ": No install path defined, security cannot be verified.\n";
    return false;
#endif
}


// Checks if this application's parent process is secure.
bool Process::Security::parentProcessSecured()
{
#ifdef PARENT_PATH
    std::string parentPath(PARENT_PATH);
    if (! parentProcessRunning())
    {
        std::cerr << errorPrefix << __func__
                << ": The application's parent process is no longer alive.\n";
        return false;
    }
    if(! processSecured(parentProcess, parentPath))
    {
        std::cerr << errorPrefix << __func__
                << ": The application's parent process is insecure.\n";
        return false;
    }
    return parentProcessRunning();
#else
    std::cerr << errorPrefix << __func__
        << ": No parent path defined, security cannot be verified.\n";
    return false;
#endif
}


// Checks if this application's parent process is still running.
bool Process::Security::parentProcessRunning()
{
    parentProcess.update();
    const State processState = parentProcess.getLastState();
    return processState != State::stopped 
            && processState != State::zombie
            && processState != State::dead
            && processState != State::invalid;
}


// Checks if a specific process is running from a specific expected path within
// a secure directory.
bool Process::Security::processSecured
(const Process::Data& process, const std::string& path) const
{
    if (! process.isValid())
    {
        std::cerr << errorPrefix << __func__
                << ": Process is not valid.\n";
        return false;
    }

    if (path != process.getExecutablePath())
    {
        std::cerr << errorPrefix << __func__
                << ": Process running from invalid executable path \""
                << process.getExecutablePath() << "\".\n";
        return false;
    }

    const size_t lastDirChar = path.find_last_of('/');
    if (lastDirChar == std::string::npos)
    {
        std::cerr << errorPrefix << __func__
                << ": Failed to find executable directory from path.\n";
        return false;
    }

    const std::string dirPath = path.substr(0, lastDirChar);
    if (! directorySecured(dirPath))
    {
        std::cerr << errorPrefix << __func__
                << ": Process is running from an unsecured directory.\n";
        return false;
    }
    return true;
}


// Checks if a given directory is secure.
bool Process::Security::directorySecured(const std::string& dirPath) const
{
    struct stat dirStats;
    errno = 0;
    if (lstat(dirPath.c_str(), &dirStats) == -1)
    {
        switch (errno)
        {
            case EACCES:
                std::cerr << errorPrefix << __func__
                        << ": Failed to search path, security is uncertain.\n";
                return false;
            case EIO:
                std::cerr << errorPrefix << __func__
                        << ": Failed to read from file system.\n";
                return false;
            case ELOOP:
                std::cerr << errorPrefix << __func__
                        << ": Encountered a symbolic link loop in the path.\n";
                return false;
            case ENAMETOOLONG:
            case ENOENT:
            case ENOTDIR:
            case EOVERFLOW:
                std::cerr << errorPrefix << __func__
                        << ": Invalid directory path.\n";
                return false;
            default:
                std::cerr << errorPrefix << __func__
                        << ": Unexpected error type " << errno << "\n";
                return false;
        }
    }
    if (! S_ISDIR(dirStats.st_mode))
    {
        std::cerr << errorPrefix << __func__ << ": Path \"" << dirPath
                << "\" was not a directory.\n";
        return false;
    }
    if (dirStats.st_uid != 0 || dirStats.st_gid != 0)
    {
        std::cerr << errorPrefix << __func__ << ": Directory \"" << dirPath 
                << "\" is not exclusively owned by root.\n";
        return false;
    }
    if ((dirStats.st_mode & S_IWOTH) != 0)
    {
        std::cerr << errorPrefix << __func__
                << ": Write permissions for \"" << dirPath 
                << "\" are not restricted to root.\n";
        return false;
    }
    return true;
}
