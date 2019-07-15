#include "DaemonLoop.h"
#include "ExitCode.h"
#include "Debug.h"
#include <unistd.h>
#include <signal.h>

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
inputPipe(DF_INPUT_PIPE_PATH, this, inputBufferSize),
#endif
#ifdef DF_OUTPUT_PIPE_PATH
outputPipe(DF_OUTPUT_PIPE_PATH),
#endif
loopRunning(false)
{
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
    inputPipe.stopReading();
#   endif
#   ifdef DF_OUTPUT_PIPE_PATH
    outputPipe.closePipe();
#   endif
}

// Starts the daemon's main action loop as long as the loop isn't already
// running.
int DaemonFramework::DaemonLoop::runLoop()
{
    if (loopRunning.exchange(true))
    {
        return (int) ExitCode::daemonAlreadyRunning;
    }

    // Check for SIGTERM between all significant actions:
    if (termSignalReceived)
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, SIGTERM received.");
        loopRunning = false;
        return (int) ExitCode::success;
    }

    // Initial security checks:
#   ifdef DF_VERIFY_PATH
    if (! securityMonitor.validDaemonPath())
    {
        DF_DBG(messagePrefix << __func__
                << ": Exiting, invalid daemon executable path.");
        loopRunning = false;
        return (int) ExitCode::badDaemonPath;
    }
#   endif
#   ifdef DF_REQUIRED_PARENT_PATH
    if (! securityMonitor.validParentPath())
    {
        DF_DBG(messagePrefix << __func__
                << ": Exiting, invalid parent executable path.");
        loopRunning = false;
        return (int) ExitCode::badParentPath;
    }
#   endif
#   ifdef DF_VERIFY_PATH_SECURITY
    if (! securityMonitor.daemonPathSecured())
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, daemon executable is in"
                << " an unsecured directory.");
        loopRunning = false;
        return (int) ExitCode::insecureDaemonDir;
    }
#   endif
#   ifdef DF_VERIFY_PARENT_PATH_SECURITY
    if (! securityMonitor.parentPathSecured())
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, parent executable is in"
                << " an unsecured directory.");
        loopRunning = false;
        return (int) ExitCode::insecureParentDir;
    }
#   endif

#   ifdef DF_REQUIRE_SINGULAR
    DF_DBG("TODO: Implement singular process check.");
#   endif

    // Check for SIGTERM again before running initLoop():
    if (termSignalReceived)
    {
        DF_DBG(messagePrefix << __func__ << ": Exiting, SIGTERM received.");
        loopRunning = false;
        return (int) ExitCode::success;
    }

    int resultCode = initLoop();

#   ifdef DF_TIMEOUT
    using namespace std::chrono;
    const time_point<system_clock> loopStartTime = system_clock::now();
#   endif

    while (resultCode == 0)
    {
        if (termSignalReceived)
        {
            DF_DBG(messagePrefix << __func__ << ": Exiting, SIGTERM received.");
            loopRunning = false;
            return (int) ExitCode::success;
        }
#       ifdef DF_REQUIRE_RUNNING_PARENT
        if(! securityMonitor.parentProcessRunning())
        {
            DF_DBG(messagePrefix << __func__ << ": Exiting, parent stopped.");
            loopRunning = false;
            return (int) ExitCode::daemonParentEnded;
        }
#       endif
#       ifdef DF_TIMEOUT
        const system_clock::duration runtime = system_clock::now()
                - loopStartTime;
        if (duration_cast<seconds>(system_clock::now() - loopStartTime).count()
                >= DF_TIMEOUT)
        {
            DF_DBG(messagePrefix << __func__ 
                    << ": Exiting, reached end of timeout period.");
            return (int) ExitCode::success;
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