#include "Process_Security.h"
#include "Process_State.h"
#include "../Debug.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#ifdef DF_DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix 
        = "DaemonFramework::Process::Security::";
#endif

/**
 * @brief  Gets the list of all process IDs.
 *
 * @return  A vector containing every process ID listed in the /proc directory.
 */
static std::vector<int> getAllPIDs()
{
    std::vector<int> pIDs;
    DIR* procDir = nullptr;
    struct dirent* dirEntry = nullptr;
    errno = 0;
    if ((procDir = opendir("/proc")) != nullptr)
    {
        while ((dirEntry = readdir(procDir)) != nullptr)
        {
            if (dirEntry->d_type != DT_DIR)
            {
                continue;
            }
            std::string dirName(dirEntry->d_name);
            if (dirName.find_first_not_of("0123456789") == std::string::npos
                    && dirName.size() > 0)
            {
                pIDs.push_back(std::stoi(dirName));
            }
        }
#       ifdef DF_DEBUG
        if (errno != 0)
        {
            DF_DBG(messagePrefix << __func__ << ": Error scanning /proc:");
            perror(messagePrefix);
        }
#       endif
        closedir(procDir);
    }
    else
    {
        DF_DBG(messagePrefix << __func__ << ": Failed to scan /proc.");
#       ifdef DF_DEBUG
        perror(messagePrefix);
#       endif
    }
    DF_DBG_V(messagePrefix << __func__ << ": Found " << pIDs.size()
            << " process IDs.");
    return pIDs;
}

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
        DF_DBG(messagePrefix << __func__
                << ": Failed to find executable directory from path.");
        return std::string();
    }
    return filePath.substr(0, lastDirChar);
}




// Loads process data on construction.
DaemonFramework::Process::Security::Security()
{
    daemonProcess = Data(getpid());
    if (daemonProcess.isValid())
    {
        parentProcess = Data(daemonProcess.getParentId());
    }
}


#ifdef DF_VERIFY_PATH
// Checks if the daemon executable is running from the expected path.
bool DaemonFramework::Process::Security::validDaemonPath()
{
    const std::string installPath(DF_DAEMON_PATH);
    return processSecured(daemonProcess, installPath);
}
#endif


#ifdef DF_REQUIRED_PARENT_PATH
// Checks if the daemon was launched by an executable at the expected path.
bool DaemonFramework::Process::Security::validParentPath()
{
    const std::string parentPath(DF_REQUIRED_PARENT_PATH);
    return processSecured(parentProcess, parentPath);
}
#endif


#ifdef DF_VERIFY_PATH_SECURITY
// Checks if the daemon's directory is secure.
bool DaemonFramework::Process::Security::daemonPathSecured()
{
    const std::string installPath(daemonProcess.getExecutablePath());
    const std::string installDir(getDirectoryPath(installPath));
    return directorySecured(installDir);
}
#endif


#ifdef DF_VERIFY_PARENT_PATH_SECURITY
// Checks if the parent application's directory is secure.
bool DaemonFramework::Process::Security::parentPathSecured()
{
    const std::string parentPath(parentProcess.getExecutablePath());
    const std::string parentDir(getDirectoryPath(parentPath));
    return directorySecured(parentDir);
}
#endif


#ifdef DF_REQUIRE_RUNNING_PARENT
// Checks if this application's parent process is still running.
bool DaemonFramework::Process::Security::parentProcessRunning()
{
    parentProcess.update();
    const State processState = parentProcess.getLastState();
    return processState != State::stopped 
            && processState != State::zombie
            && processState != State::dead
            && processState != State::invalid;
}
#endif


#ifdef DF_REQUIRE_SINGULAR
// Checks that only one daemon process is running.
bool DaemonFramework::Process::Security::daemonProcessIsSingular()
{
    daemonProcess.update();
    const std::string daemonPath = daemonProcess.getExecutablePath();
    const std::vector<int> processList = getAllPIDs();
    for (const int& pid : processList)
    {
        if (pid == daemonProcess.getProcessId())
        {
            continue;
        }
        Data processInfo(pid);
        if (! processInfo.isValid())
        {
            continue;
        }
        const State processState = processInfo.getLastState();
        if (processState == State::stopped || processState != State::zombie
                || processState != State::dead
                || processState != State::invalid)
        {
            continue;
        }
        if (processInfo.getExecutablePath()
                == daemonProcess.getExecutablePath())
        {
            return false;
        }
    }
    return true;
}
#endif


// Checks if a specific process is running from a specific expected path within
// a secure directory.
bool DaemonFramework::Process::Security::processSecured
(const Process::Data& process, const std::string& path) const
{
    if (! process.isValid())
    {
        DF_DBG(messagePrefix << __func__ << ": Process is not valid.");
        return false;
    }

    if (path != process.getExecutablePath())
    {
        DF_DBG(messagePrefix << __func__
                << ": Process running from invalid executable path \""
                << process.getExecutablePath() << "\".");
        return false;
    }
    return true;
}


// Checks if a given directory is secure.
bool DaemonFramework::Process::Security::directorySecured
(const std::string& dirPath) const
{
    struct stat dirStats;
    errno = 0;
    if (lstat(dirPath.c_str(), &dirStats) == -1)
    {
        switch (errno)
        {
            case EACCES:
                DF_DBG(messagePrefix << __func__
                        << ": Failed to search path, security is uncertain.");
                return false;
            case EIO:
                DF_DBG(messagePrefix << __func__
                        << ": Failed to read from file system.");
                return false;
            case ELOOP:
                DF_DBG(messagePrefix << __func__
                        << ": Encountered a symbolic link loop in the path.");
                return false;
            case ENAMETOOLONG:
            case ENOENT:
            case ENOTDIR:
            case EOVERFLOW:
                DF_DBG(messagePrefix << __func__ 
                        << ": Invalid directory path.");
                return false;
            default:
                DF_DBG(messagePrefix << __func__ << ": Unexpected error type " 
                        << errno);
                return false;
        }
    }
    if (! S_ISDIR(dirStats.st_mode))
    {
        DF_DBG(messagePrefix << __func__ << ": Path \"" << dirPath
                << "\" was not a directory.");
        return false;
    }
    if (dirStats.st_uid != 0 || dirStats.st_gid != 0)
    {
        DF_DBG(messagePrefix << __func__ << ": Directory \"" << dirPath 
                << "\" is not exclusively owned by root.");
        return false;
    }
    if ((dirStats.st_mode & S_IWOTH) != 0)
    {
        DF_DBG(messagePrefix << __func__ << ": Write permissions for \""
                << dirPath << "\" are not restricted to root.");
        return false;
    }
    return true;
}
