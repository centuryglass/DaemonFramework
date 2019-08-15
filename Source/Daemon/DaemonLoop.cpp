#include "DaemonLoop.h"
#include "ExitCode.h"
#include "File_Utils.h"
#include "Debug.h"
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include <fcntl.h>

#ifdef DF_TIMEOUT
#include <chrono>
#endif

#ifdef DF_DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "DaemonFramework::DaemonLoop::";
#endif


// Stores whether the daemon process should be terminated:
// -1: sigaction not yet called.
//  0: sigaction called, SIGTERM not received.
//  1: sigterm received.
volatile static std::atomic_int termSignalReceived(-1);


// Sets termSignalReceived when a termination signal is caught:
void DaemonFramework::DaemonLoop::flagTermSignal(int signum)
{
    DF_ASSERT(signum == SIGTERM);
    DF_DBG(messagePrefix << __func__ << ": Received SIGTERM");
    termSignalReceived = true;
}


// Initializes the DaemonLoop.
DaemonFramework::DaemonLoop::DaemonLoop(const int inputBufferSize) :
#ifdef DF_INPUT_PIPE_PATH
inputPipe(DF_INPUT_PIPE_PATH, inputBufferSize),
#endif
#ifdef DF_OUTPUT_PIPE_PATH
outputPipe(DF_OUTPUT_PIPE_PATH, true),
#endif
loopRunning(false)
{
#   ifdef DF_INPUT_PIPE_PATH
    inputPipe.openPipe(this);
    DF_DBG_V(messagePrefix << __func__ << ": Daemon input reader: opened "
            << DF_INPUT_PIPE_PATH);
#   endif
#   ifdef DF_OUTPUT_PIPE_PATH
    DF_DBG_V(messagePrefix << __func__ << ": Daemon output writer: using "
            << DF_OUTPUT_PIPE_PATH);
#   endif
    // Verify that only one DaemonLoop is created:
    static std::atomic_bool constructFlag(false);
    if (constructFlag.exchange(true))
    {
        DF_DBG(messagePrefix << __func__ 
                << ": Created more than one DaemonLoop!");
        DF_ASSERT(false);
    }

    // Set up handler for SIGTERM signals:
    if (termSignalReceived.exchange(0) == -1)
    {
        struct sigaction action = {};
        sigaction(SIGTERM, &action, NULL);
        action.sa_handler = flagTermSignal;
    }
    else
    {
        DF_DBG(messagePrefix << __func__ 
                << ": Attempted to set SIGTERM handler more than once!");
        DF_ASSERT(false);
    }
}


// Ensures all pipes are closed on destruction.
DaemonFramework::DaemonLoop::~DaemonLoop()
{
#   ifdef DF_INPUT_PIPE_PATH
    DF_DBG_V(messagePrefix << __func__ << ": Closing input pipe:");
    inputPipe.closePipe();
#   endif
#   ifdef DF_OUTPUT_PIPE_PATH
    DF_DBG_V(messagePrefix << __func__ << ": Closing output pipe:");
    outputPipe.closePipe();
#   endif
#   ifdef DF_LOCK_FILE_PATH
    if (lockFD != 0)
    {
        DF_DBG_V(messagePrefix << __func__ << ": Unlocking lock file \""
                << DF_LOCK_FILE_PATH << "\"");
        errno = 0;
        struct 
        {
            short l_type = F_UNLCK;
            short l_whence = SEEK_SET;
            off_t l_start = 0;
            off_t l_len = 0;
            pid_t l_pid = getpid();
        } lockInfo;
        if (fcntl(lockFD, F_GETLK, &lockInfo) == -1)
        {
            DF_DBG(messagePrefix << __func__ << ": Error unlocking lock file:");
            DF_PERROR(messagePrefix);
        }
        DF_DBG_V(messagePrefix << __func__ << ": Closing lock file:");
        while (close(lockFD) == -1)
        {
            DF_DBG(messagePrefix << __func__ << ": Error closing lock file:");
            DF_PERROR(messagePrefix);
            if (errno != EINTR)
            {
                break;
            }
        }
        lockFD = 0;
    }
    DF_DBG_V(messagePrefix << __func__ << ": DaemonLoop destroyed.");
#   endif
}

// Starts the daemon's main action loop as long as the loop isn't already
// running.
int DaemonFramework::DaemonLoop::runLoop()
{
    if (loopRunning.exchange(true))
    {
        return static_cast<int>(ExitCode::daemonAlreadyRunning);
    }

    // Check for SIGTERM between all significant actions:
    if (termSignalReceived)
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, SIGTERM received.");
        loopRunning = false;
        return static_cast<int>(ExitCode::success);
    }

    // Initial security checks:
    DF_DBG_V(messagePrefix << __func__ << ": Starting security checks.");
#   ifdef DF_LOCK_FILE_PATH
    DF_ASSERT(lockFD == 0);
    do
    {
        errno = 0;
        DF_DBG_V(messagePrefix << __func__ << ": Locking lock file \""
                << DF_LOCK_FILE_PATH << "\"");
        lockFD = open(DF_LOCK_FILE_PATH, O_CREAT|O_RDWR|O_NONBLOCK);
        if (lockFD != -1)
        {
            break; // opening file worked, no more steps required.
        }
        if (errno == EINTR)
        {
            if (termSignalReceived)
            {
                DF_DBG(messagePrefix << __func__
                        << ": Lock open interrupted by SIGTERM, exiting.");
                return static_cast<int>(ExitCode::success);
            }
            DF_DBG_V(messagePrefix << __func__
                    << ": opening lock interrupted, trying again:");
            continue;
        }
        if (errno == ENOENT)
        {
            // One or more of the lock's parent directories may need to be
            // created, try and do that.
            const std::string lockPath(DF_LOCK_FILE_PATH);
            const std::string lockDirPath = File::Utils::parentDir(lockPath);
            if (lockDirPath.empty())
            {
                DF_DBG(messagePrefix << __func__
                        << ": Unable to open lock, lock path \""  << lockPath
                        << "\" is probably invalid.");
                DF_ASSERT(false);
                return static_cast<int>(ExitCode::daemonAlreadyRunning);
            }
            if (File::Utils::createDir(lockDirPath))
            {
                DF_DBG_V(messagePrefix << __func__ << ": lock directory \""
                        << lockDirPath << "\" created, trying again:");
                continue;
            }
            DF_DBG(messagePrefix << __func__
                    << ": Failed to create lock directory \"" << lockDirPath
                    << "\", exiting.");
            return static_cast<int>(ExitCode::daemonAlreadyRunning);
        }
        DF_DBG(messagePrefix << __func__
                << ": Exiting, unable to open lock file:")
        DF_PERROR("DaemonLoop: Lock opening error");
        lockFD = 0;
        return static_cast<int>(ExitCode::daemonAlreadyRunning);
    }
    while (lockFD == -1);

    struct 
    {
        short l_type = F_WRLCK;
        short l_whence = SEEK_SET;
        off_t l_start = 0;
        off_t l_len = 0;
        pid_t l_pid = getpid();
    } lockInfo;
    int lockResult = 0;

    // Test if a locking action succeeded, closing a lock file and returning
    // an error code if it failed.
    const auto testLock = [this, &lockInfo, &lockResult]()
    {
        if (lockResult == -1 || (lockInfo.l_pid != getpid()))
        {
            DF_DBG(messagePrefix << __func__
                    << ": Exiting, lock file \"" << DF_LOCK_FILE_PATH
                    << "\" is already locked:")
            if (lockResult == -1)
            {
                DF_PERROR("DaemonLoop: Locking error");
            }
            while(close(lockFD) == -1)
            {
                DF_DBG(messagePrefix << __func__
                        << ": Error closing lock file:");
                DF_PERROR("DaemonLoop: Lock closing error");
                if (errno != EINTR)
                {
                    break;
                }
            }
            lockFD = 0;
            return static_cast<int>(ExitCode::daemonAlreadyRunning);
        }
        return 0;
    };

    // Check if the lock is already held:
    lockResult = fcntl(lockFD, F_GETLK, &lockInfo);
    lockResult = testLock();
    if (lockResult != 0)
    {
        return lockResult;
    }
    // Lock isn't held, try and actually take it:
    // TODO: If another process manages to lock the file at this point, this
    //       process will hang until the other one dies or releases the lock.
    //       Find a way to both check and claim the lock atomically.
    lockInfo.l_type = F_WRLCK;
    lockResult = fcntl(lockFD, F_SETLKW, &lockInfo);
    lockResult = testLock();
    if (lockResult != 0)
    {
        return lockResult;
    }
    DF_DBG_V(messagePrefix << __func__ << ": Successfully locked lock file \""
            << DF_LOCK_FILE_PATH << "\"");

#   endif
#   if defined DF_VERIFY_PATH && DF_VERIFY_PATH
    if (! securityMonitor.validDaemonPath())
    {
        DF_DBG(messagePrefix << __func__
                << ": Exiting, invalid daemon executable path.");
        loopRunning = false;
        return static_cast<int>(ExitCode::badDaemonPath);
    }
#   endif
#   ifdef DF_REQUIRED_PARENT_PATH
    if (! securityMonitor.validParentPath())
    {
        DF_DBG(messagePrefix << __func__
                << ": Exiting, invalid parent executable path.");
        loopRunning = false;
        return static_cast<int>(ExitCode::badParentPath);
    }
#   endif
#   if defined DF_VERIFY_PATH_SECURITY && DF_VERIFY_PATH_SECURITY
    if (! securityMonitor.daemonPathSecured())
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, daemon executable is in"
                << " an unsecured directory.");
        loopRunning = false;
        return static_cast<int>(ExitCode::insecureDaemonDir);
    }
#   endif
#   if defined DF_VERIFY_PARENT_PATH_SECURITY && DF_VERIFY_PARENT_PATH_SECURITY
    if (! securityMonitor.parentPathSecured())
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, parent executable is in"
                << " an unsecured directory.");
        loopRunning = false;
        return static_cast<int>(ExitCode::insecureParentDir);
    }
#   endif


    // Check for SIGTERM again before running initLoop():
    if (termSignalReceived)
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, SIGTERM received.");
        loopRunning = false;
        return static_cast<int>(ExitCode::success);
    }

    DF_DBG_V(messagePrefix << __func__ << ": Calling initLoop():");
    int resultCode = initLoop();

#   if defined DF_TIMEOUT && DF_TIMEOUT > 0
    using namespace std::chrono;
    const time_point<system_clock> loopStartTime = system_clock::now();
#   endif

    DF_DBG_V(messagePrefix << __func__ << ": Starting main loop:");
    while (resultCode == 0)
    {
        if (termSignalReceived)
        {
            DF_DBG(messagePrefix << __func__ << ": Exiting, SIGTERM received.");
            loopRunning = false;
            return static_cast<int>(ExitCode::success);
        }
#       if defined DF_REQUIRE_RUNNING_PARENT && DF_REQUIRE_RUNNING_PARENT
        if(! securityMonitor.parentProcessRunning())
        {
            DF_DBG(messagePrefix << __func__ << ": Exiting, parent stopped.");
            loopRunning = false;
            return static_cast<int>(ExitCode::daemonParentEnded);
        }
#       endif
#       if defined DF_TIMEOUT && DF_TIMEOUT > 0
        const system_clock::duration runtime = system_clock::now()
                - loopStartTime;
        if (duration_cast<seconds>(system_clock::now() - loopStartTime).count()
                >= DF_TIMEOUT)
        {
            DF_DBG(messagePrefix << __func__ 
                    << ": Exiting, reached end of " << DF_TIMEOUT
                    << " second timeout period.");
            return static_cast<int>(ExitCode::success);
        }
#       endif
        resultCode = loopAction();
    }
    DF_DBG(messagePrefix << __func__ << ": Exiting loop with code "
            << resultCode);
    loopRunning = false;
    return resultCode;
}


// Checks if runLoop() has been called already, and the loop is currently
// running.
bool DaemonFramework::DaemonLoop::isLoopRunning() const
{
    return loopRunning;
}


// Performs any extra initialization required just before the main daemon loop
// starts.
int DaemonFramework::DaemonLoop::initLoop()
{
    return 0;
}


#ifdef DF_OUTPUT_PIPE_PATH
// Sends arbitrary data to the parent process through the daemon's named output
// pipe.
void DaemonFramework::DaemonLoop::messageParent(const unsigned char* messageData,
            const size_t messageSize)
{
#   ifdef DF_DEBUG
    if (! outputPipe.sendData(messageData, messageSize))
    {
        DF_DBG(messagePrefix << __func__ << ": Failed to send message of size "
                << messageSize << " to parent process.");
    }
#   else
    outputPipe.sendData(messageData, messageSize);
#   endif
}
#endif


#ifdef DF_INPUT_PIPE_PATH
// Passes data sent from the parent application to the handleParentMessage()
// function.
void DaemonFramework::DaemonLoop::processData
(const unsigned char* data, const size_t size)
{
    handleParentMessage(data, size);
}
#endif
