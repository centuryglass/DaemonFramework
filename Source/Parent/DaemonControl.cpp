#include "DaemonControl.h"
#include "Pipe.h"
#include "ExitCode.h"
#include "Debug.h"
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <string>
#include <sstream>

#ifdef DF_DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix
    = "DaemonFramework::DaemonControl::";
#endif

// Seconds to wait before assuming the daemon process is not going to handle
// a SIGTERM signal and needs to be killed:
static const constexpr int daemonTermTimeout = 2;


// If relevant, prepares daemon IO pipe objects on construction.
DaemonFramework::DaemonControl::DaemonControl
#ifdef DF_OUTPUT_PIPE_PATH
(Pipe::Listener* listener, const size_t bufferSize) : 
        pipeReader(DF_OUTPUT_PIPE_PATH, listener, bufferSize),
#else
() :
#endif
#       ifdef DF_INPUT_PIPE_PATH
        pipeWriter(DF_INPUT_PIPE_PATH),
#       endif
        exitCode(0)
{
#   ifdef DF_OUTPUT_PIPE_PATH
    // Ensure the daemon output pipe exists:
    Pipe::createPipe(DF_OUTPUT_PIPE_PATH, S_IRUSR);
    DF_DBG_V(messagePrefix << __func__ << ": Parent input reader: opened "
            << DF_OUTPUT_PIPE_PATH);
#   endif
#   ifdef DF_INPUT_PIPE_PATH
    // Ensure the daemon input pipe exists:
    Pipe::createPipe(DF_INPUT_PIPE_PATH, S_IWUSR);
    DF_DBG_V(messagePrefix << __func__ << ": Parent output writer: opened "
            << DF_INPUT_PIPE_PATH);
#   endif
}


/**
 * @brief  Closes all open file descriptors except for stdin/stout/stderr.
 *
 *  This should only be called within the daemon process, before executing the
 * daemon. This ensures that the parent application's open files aren't 
 * unnecessarily shared with the daemon. If any errors occur, the process will
 * exit, returning ErrorCode::fdCleanupFailed.
 */
static void cleanupFileTable()
{
    using DaemonFramework::ExitCode;
    pid_t processID = getpid();
    const std::string fdPath = std::string("/proc/")
            + std::to_string((int) processID) + "/fd";
    DF_DBG_V(messagePrefix << __func__ 
            << ": Finding and closing leaked file descriptors listed in \""
            << fdPath << "\"");
    errno = 0;
    int fdDirFD = open(fdPath.c_str(), O_RDONLY | O_DIRECTORY);
    if (fdDirFD == -1)
    {
        DF_DBG(messagePrefix << __func__
                << ": Failed to open process file descriptor directory.");
        DF_PERROR(messagePrefix);
        exit((int) ExitCode::fdCleanupFailed);
    }
    DIR* fdDir = fdopendir(fdDirFD);
    if (fdDir == nullptr)
    {
        DF_DBG(messagePrefix << __func__ << ": Failed to open process file"
                << " descriptor directory stream.");
#       ifdef DF_DEBUG
        DF_PERROR(messagePrefix);
#       endif
        exit((int) ExitCode::fdCleanupFailed);
    }
    struct dirent* fdFileInfo;
#   ifdef DF_DEBUG
    int filesClosed = 0;
#   endif
    while ((fdFileInfo = readdir(fdDir)) != nullptr)
    {
        int fd = strtol(fdFileInfo->d_name, nullptr, 10);
        if (fd > 2 && fd != fdDirFD)
        {
            errno = 0;
            int result = 0;
            do
            {
                result = close(fd);
            }
            while (errno == EINTR);
            if (result == -1 && errno != EBADF)
            {
                DF_DBG(messagePrefix << __func__ 
                        << ": Failed to close open file descriptor " << fd);
#               ifdef DF_DEBUG
                DF_PERROR(messagePrefix);
#               endif
                exit((int) ExitCode::fdCleanupFailed);
            }
#           ifdef DF_DEBUG
            filesClosed++;
#           endif
        }
    }
    closedir(fdDir);
    DF_DBG(messagePrefix << __func__ << ": Closed " << filesClosed
            << " unnecessary open file descriptors.");
}


// If the daemon isn't already running, this launches the daemon and opens
// daemon communication pipes if needed.
void DaemonFramework::DaemonControl::startDaemon(std::vector<std::string> args)
{
    if (daemonProcess != 0)
    {
        DF_DBG(messagePrefix << __func__
                << ": Aborting, daemon process is already running.");
        return;
    }
#   ifdef DF_INPUT_PIPE_PATH
    pipeWriter.openPipe();
#   endif
#   ifdef DF_OUTPUT_PIPE_PATH
    pipeReader.openPipe();
#   endif

    daemonProcess = fork();
    if (daemonProcess == 0) // If runnning the new process:
    {
        DF_DBG_V(messagePrefix << __func__ << ": Daemon process started.");
        cleanupFileTable();
        DF_DBG_V(messagePrefix << __func__ << ": Launching \"" << DF_DAEMON_PATH
                << "\"");
        std::vector<const char*> cStrings(args.size() + 1);
        for (std::string& arg : args)
        {
            cStrings.push_back(arg.c_str());
        }
        cStrings.push_back(nullptr);
        errno = 0;
        int result = execv(DF_DAEMON_PATH, (char* const*) cStrings.data());
        if (result == -1)
        {
            DF_DBG(messagePrefix << __func__ << ": Failed to launch daemon.");
            DF_PERROR(messagePrefix);
            exit(static_cast<int>(ExitCode::daemonExecFailed));
        }
        else
        {
            DF_DBG(messagePrefix << __func__ << ": Daemon exited with code "
                    << result << ", ending daemon process.");
            exit(result);
        }
    }
}


// If the daemon is running, this stops the process and closes the key input
// pipe.
void DaemonFramework::DaemonControl::stopDaemon()
{
    if (daemonProcess != 0)
    {
        DF_DBG_V(messagePrefix << __func__ << ": Terminating daemon process "
                << (int) daemonProcess);
        kill(daemonProcess, SIGTERM);
        sleep(daemonTermTimeout);
        if (isDaemonRunning()) // SIGTERM ignored, take more aggressive measures
        {
            DF_DBG_V(messagePrefix << __func__ 
                    << ": Daemon process ignored SIGTERM, sending SIGKILL.");
            kill(daemonProcess, SIGKILL);
            int daemonStatus;
            waitpid(daemonProcess, &daemonStatus, 0);
            exitCode = WEXITSTATUS(daemonStatus);
            daemonProcess = 0;
            DF_DBG(messagePrefix << __func__
                    << ": Daemon process exited with code " << exitCode);
        }
#       ifdef DF_OUTPUT_PIPE_PATH
        DF_DBG_V(messagePrefix << __func__ << ": Closing PipeReader:");
        pipeReader.closePipe();
#       endif
#       ifdef DF_INPUT_PIPE_PATH
        DF_DBG_V(messagePrefix << __func__ << ": Closing PipeWriter:");
        pipeWriter.closePipe();
#       endif
    }
}


// Checks if the daemon is running.
bool DaemonFramework::DaemonControl::isDaemonRunning()
{
    if (daemonProcess == 0)
    {
        return false;
    }
    int daemonStatus;
    errno = 0;
    pid_t waitResult = waitpid(daemonProcess, &daemonStatus, WNOHANG);
    if (waitResult == -1)
    {
        DF_DBG(messagePrefix << __func__ << ": Error checking status:\n");
        DF_PERROR(messagePrefix);
        daemonProcess = 0;
        return false;
    }
    if (waitResult == 0) // Daemon is still running 
    {
        DF_DBG_V(messagePrefix << __func__ << ": Daemon " << daemonProcess
                << " is still running, errno=" << errno);
        return true;
    }
    if (waitResult == daemonProcess) // Process finished
    {
        daemonProcess = 0;
        exitCode = WEXITSTATUS(daemonStatus);
        return false;
    }
    // Result should always be one of the options above
    DF_DBG(messagePrefix << __func__ << ": Invalid wait result "
            << (int) waitResult);
    DF_ASSERT(false);
    return false;
}

#ifdef DF_INPUT_PIPE_PATH
// Sends arbitrary data to the daemon using the daemon's named input pipe.
void DaemonFramework::DaemonControl::messageParent
(const unsigned char* messageData, const size_t messageSize)
{
    pipeWriter.sendData(messageData, messageSize);
}
#endif

// Gets the ID of the daemon process if running.
pid_t DaemonFramework::DaemonControl::getDaemonProcessID()
{
    if (isDaemonRunning())
    {
        return daemonProcess;
    }
    return 0;
}


// Gets the exit code returned by the daemon process after it finished.
int DaemonFramework::DaemonControl::getExitCode()
{
    if (isDaemonRunning())
    {
        DF_DBG(messagePrefix << __func__ << ": Daemon is still running!");
        DF_ASSERT(false);
        return 0;
    }
    return exitCode;
}


// Waits until the daemon process terminates and gets the process exit code.
int DaemonFramework::DaemonControl::waitToExit()
{
    DF_DBG_V(messagePrefix << __func__ << ": Checking if daemon process "
            << (int) daemonProcess << " has already exited:");
    while (isDaemonRunning())
    {
        int daemonStatus;
        errno = 0;
        if(waitpid(daemonProcess, &daemonStatus, 0) == -1)
        {
            DF_DBG(messagePrefix << __func__ << ": waitpid error:");
            DF_PERROR(messagePrefix);
        }
        if (WIFEXITED(daemonStatus))
        {
            daemonProcess = 0;
            exitCode = WEXITSTATUS(daemonStatus);
            break;
        }
        else if (WIFSIGNALED(daemonStatus))
        {
            DF_DBG(messagePrefix << __func__ << ": killed by signal "
                    << WTERMSIG(daemonStatus));
        }
        else if (WIFSTOPPED(daemonStatus))
        {
            DF_DBG(messagePrefix << __func__ << ": stopped by signal "
                    << WSTOPSIG(daemonStatus));
        }
    }
    DF_DBG_V(messagePrefix << __func__ << ": Daemon exited with code "
            << exitCode);
    return exitCode;
}
