#include "Process_Security.h"
#include "Process_State.h"
#include "../Debug.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: Process::Security::";
#endif

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
        DBG(messagePrefix << __func__
                << ": Failed to find executable directory from path.");
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
        DBG(messagePrefix << __func__ << ": Process is not valid.");
        return false;
    }

    if (path != process.getExecutablePath())
    {
        DBG(messagePrefix << __func__
                << ": Process running from invalid executable path \""
                << process.getExecutablePath() << "\".");
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
                DBG(messagePrefix << __func__
                        << ": Failed to search path, security is uncertain.");
                return false;
            case EIO:
                DBG(messagePrefix << __func__
                        << ": Failed to read from file system.");
                return false;
            case ELOOP:
                DBG(messagePrefix << __func__
                        << ": Encountered a symbolic link loop in the path.");
                return false;
            case ENAMETOOLONG:
            case ENOENT:
            case ENOTDIR:
            case EOVERFLOW:
                DBG(messagePrefix << __func__ << ": Invalid directory path.");
                return false;
            default:
                DBG(messagePrefix << __func__ << ": Unexpected error type " 
                        << errno);
                return false;
        }
    }
    if (! S_ISDIR(dirStats.st_mode))
    {
        DBG(messagePrefix << __func__ << ": Path \"" << dirPath
                << "\" was not a directory.");
        return false;
    }
    if (dirStats.st_uid != 0 || dirStats.st_gid != 0)
    {
        DBG(messagePrefix << __func__ << ": Directory \"" << dirPath 
                << "\" is not exclusively owned by root.");
        return false;
    }
    if ((dirStats.st_mode & S_IWOTH) != 0)
    {
        DBG(messagePrefix << __func__ << ": Write permissions for \"" << dirPath 
                << "\" are not restricted to root.");
        return false;
    }
    return true;
}
