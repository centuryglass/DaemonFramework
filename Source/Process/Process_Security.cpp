#include "Process_Security.h"
#include "Process_State.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

static const constexpr char* errorPrefix = "KeyDaemon: Process::Security::";


/**
 * @brief  Given a file path, return the path to that file's directory.
 *
 * @param filePath  An absolute path to any file.
 *
 * @return          The path of the directory containing that file, or the empty
 *                  string if filePath was formatted incorrectly.
 */
static std::string getDirectoryPath(const std::string& filePath)
{
    const size_t lastDirChar = filePath.find_last_of('/');
    if (lastDirChar == std::string::npos)
    {
        std::cerr << errorPrefix << __func__
                << ": Failed to find executable directory from path.\n";
        return std::string();
    }
    return filePath.substr(0, lastDirChar);
}


// Loads process data on construction.
Process::Security::Security()
{
    daemonProcess = Data(getpid());
    if (daemonProcess.isValid())
    {
        parentProcess = Data(daemonProcess.getParentId());
    }
}


// Checks if the KeyDaemon executable is running from the expected path.
bool Process::Security::validDaemonPath()
{
    const std::string installPath(INSTALL_PATH);
    return processSecured(daemonProcess, installPath);
}


// Checks if the KeyDaemon was launched by an executable at the expected path.
bool Process::Security::validParentPath()
{
    const std::string parentPath(PARENT_PATH);
    return processSecured(parentProcess, parentPath);
}


// Checks if the KeyDaemon's directory is secure.
bool Process::Security::daemonPathSecured()
{
    const std::string installPath(INSTALL_PATH);
    const std::string installDir(getDirectoryPath(installPath));
    return directorySecured(installDir);
}


// Checks if the parent application's directory is secure.
bool Process::Security::parentPathSecured()
{
    const std::string parentPath(PARENT_PATH);
    const std::string parentDir(getDirectoryPath(parentPath));
    return directorySecured(parentDir);
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
