#include "KeyDaemonControl.h"
#include "Debug.h"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <sstream>

#ifndef KEY_PIPE_PATH
  #error "Parent_Include::KeyDaemonControl: KEY_PIPE_PATH must be defined."
#endif

#ifndef INSTALL_PATH
  #error "Parent_Include::KeyDaemonControl: INSTALL_PATH must be defined."
#endif

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix
    = "KeyDaemon Parent: KeyDaemonControl::";
#endif

// Seconds to wait before assuming the daemon process needs to be killed:
static const constexpr int daemonTermTimeout = 2;


// Starts the key pipe reader within a new thread.
void* pipeThreadAction(void* codePipe)
{
    DBG_V(messagePrefix << __func__ << ": Opening PipeReader:");
    PipeReader* pipe = static_cast<PipeReader*>(codePipe);
    pipe->startReading();
    DBG_V(messagePrefix << __func__ << ": PipeReader opened.");
    return nullptr;
}

// Saves the paths used to initialize the key pipe and launch the KeyDaemon.
KeyDaemonControl::KeyDaemonControl(PipeReader::Listener* keyListener) : 
        pipeReader(keyListener) { }

// If the KeyDaemon isn't already running, this launches the daemon and starts
// listening for key codes.
void KeyDaemonControl::startDaemon(const char** trackedKeyCodeArgs)
{
    if (daemonProcess != 0)
    {
        DBG(messagePrefix << __func__
                << ": Aborting, daemon process is already running.");
        return;
    }
    if (trackedKeyCodeArgs == nullptr)
    {
        DBG(messagePrefix << __func__ << ": No valid code arguments provided.");
        ASSERT(false);
        return;
    }
    if (pipeReader.getState() == InputReader::State::initializing)
    {
        int threadError = pthread_create(&pipeInitThread, nullptr,
                pipeThreadAction, &pipeReader);
        if (threadError != 0)
        {
            DBG(messagePrefix << __func__
                    << ": Failed to start pipeReader thread.");
            return;
        }
    }
    else
    {
        DBG(messagePrefix << __func__ << ": PipeReader was not in "
                << "expected state State::initializing.");
        ASSERT(false);
    }

    daemonProcess = fork();
    if (daemonProcess == 0) // If runnning the new process
    {
        DBG_V(messagePrefix << __func__
                << ": Daemon process started, launching " << INSTALL_PATH);
        int result = execv(INSTALL_PATH, (char* const*) trackedKeyCodeArgs);
    }
}

// If the KeyDaemon isn't already running, this launches the daemon and starts
// listening for key codes.
void KeyDaemonControl::startDaemon(const char* trackedKeyCodeString)
{
    if (daemonProcess != 0)
    {
        DBG(messagePrefix << __func__
                << ": Aborting, daemon process is already running.");
        return;
    }
    using std::string;
    std::vector<string> codeStrings;
    std::istringstream codeStream((string(trackedKeyCodeString)));
    string keyCode;
    while (codeStream >> keyCode)
    {
        codeStrings.push_back(keyCode);
    }
    const char** argStrings = new const char* [codeStrings.size() + 1];
    for (int i = 0; i < codeStrings.size(); i++)
    {
        argStrings[i] = codeStrings[i].c_str();
    }
    argStrings[codeStrings.size()] = nullptr;
    startDaemon(argStrings);
    delete [] argStrings;
}


// If the KeyDaemon isn't already running, this launches the daemon and starts
// listening for key codes.
void KeyDaemonControl::startDaemon(const std::vector<int>& trackedKeyCodes)
{
    if (daemonProcess != 0)
    {
        DBG(messagePrefix << __func__
                << ": Aborting, daemon process is already running.");
        return;
    }
    using std::string;
    std::vector<string> codeStrings;
    for (const int& code : trackedKeyCodes)
    {
        codeStrings.push_back(std::to_string(code));
    }
    const char** argStrings = new const char* [codeStrings.size() + 1];
    for (int i = 0; i < codeStrings.size(); i++)
    {
        argStrings[i] = codeStrings[i].c_str();
    }
    argStrings[codeStrings.size()] = nullptr;
    startDaemon(argStrings);
    delete [] argStrings;
}


// If the KeyDaemon is running, this stops the process and closes the key input
// pipe.
void KeyDaemonControl::stopDaemon()
{
    if (daemonProcess != 0)
    {
        DBG_V(messagePrefix << __func__ << ": Terminating daemon process "
                << (int) daemonProcess);
        kill(daemonProcess, SIGTERM);
        sleep(daemonTermTimeout);
        if (isDaemonRunning()) // SIGTERM ignored, take more aggressive measures
        {
            DBG_V(messagePrefix << __func__ 
                    << ": Daemon process ignored SIGTERM, sending SIGKILL.");
            kill(daemonProcess, SIGKILL);
            int daemonStatus;
            waitpid(daemonProcess, &daemonStatus, 0);
            exitCode = WEXITSTATUS(daemonStatus);
            daemonProcess = 0;
            DBG(messagePrefix << __func__
                    << ": Daemon process exited with code " << exitCode);
        }
        // If the pipeReader is still opening, it's stuck and the pipeInitThread
        // will need to be forcibly killed.
        if (pipeReader.getState() == InputReader::State::opening)
        {
            DBG_V(messagePrefix << __func__
                    << ": PipeReader stuck opening the key event pipe, "
                    << "cancelling the reader thread.");
            int cancelResult = pthread_cancel(pipeInitThread);
            if (cancelResult == 0)
            {
                pthread_join(pipeInitThread, nullptr);
            }
            else
            {
                DBG(messagePrefix << __func__ 
                        << ": pthread_cancel returned error code "
                        << cancelResult);
            }
        }
        else
        {
            DBG_V(messagePrefix << __func__ << ": Closing PipeReader:");
            pipeReader.stopReading();
        }
    }
}


// Checks if the KeyDaemon is running.
bool KeyDaemonControl::isDaemonRunning()
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
        DBG(messagePrefix << __func__ << ": Error checking status:\n");
        #ifdef DEBUG
            perror(messagePrefix);
        #endif
        daemonProcess = 0;
        return false;
    }
    if (waitResult == 0) // Daemon is still running 
    {
        return true;
    }
    if (waitResult == daemonProcess) // Process finished
    {
        daemonProcess = 0;
        exitCode = WEXITSTATUS(daemonStatus);
        return false;
    }
    // Result should always be one of the options above
    DBG(messagePrefix << __func__ << ": Invalid wait result "
            << (int) waitResult);
    ASSERT(false);
    return false;
}


// Gets the ID of the daemon process if running.
pid_t KeyDaemonControl::getDaemonProcessID()
{
    if (isDaemonRunning())
    {
        return daemonProcess;
    }
    return 0;
}


// Gets the exit code returned by the daemon process after it finished.
int KeyDaemonControl::getExitCode()
{
    if (isDaemonRunning())
    {
        DBG(messagePrefix << __func__ << ": Daemon is still running!");
        ASSERT(false);
        return 0;
    }
    return exitCode;
}


// Waits until the daemon process terminates and gets the process exit code.
int KeyDaemonControl::waitToExit()
{
    DBG_V(messagePrefix << __func__ << ": Waiting for daemon to exit...");
    if (isDaemonRunning())
    {
        int daemonStatus;
        waitpid(daemonProcess, &daemonStatus, 0);
        return WEXITSTATUS(daemonStatus);
    }
    DBG_V(messagePrefix << __func__ << ": Daemon exited with code "
            << exitCode);
    return exitCode;
}
