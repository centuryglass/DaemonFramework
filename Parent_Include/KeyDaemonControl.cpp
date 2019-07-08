#include "KeyDaemonControl.h"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <iostream>

#ifndef KEY_PIPE_PATH
  #error "Parent_Include::KeyDaemonControl: KEY_PIPE_PATH must be defined."
#endif

#ifndef INSTALL_PATH
  #error "Parent_Include::KeyDaemonControl: INSTALL_PATH must be defined."
#endif

// Seconds to wait before assuming the daemon process needs to be killed:
static const constexpr int daemonTermTimeout = 2;

// Prefix to print before error messages:
static const constexpr char* messagePrefix = "KeyDaemonControl: ";

// Starts the key pipe reader within a new thread.
void* pipeThreadAction(void* codePipe)
{
    PipeReader* pipe = static_cast<PipeReader*>(codePipe);
    pipe->startReading();
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
        return;
    }
    if (trackedKeyCodeArgs == nullptr)
    {
        std::cerr << messagePrefix << __func__ 
                << ": No valid code arguments provided.\n";
        assert(false);
        return;
    }
    if (pipeReader.getState() == InputReader::State::initializing)
    {
        int threadError = pthread_create(&pipeInitThread, nullptr,
                pipeThreadAction, &pipeReader);
        if (threadError != 0)
        {
            std::cerr << messagePrefix << __func__
                    << ": Failed to start pipeReader thread.\n";
            return;
        }
    }
    else
    {
        std::cerr << messagePrefix << __func__ << ": pipeReader was not in "
                << "expected state State::initializing.\n";
        assert(false);
    }
    std::cout << "Codes: ";
    int idx = 0;
    for (const char* arg = *trackedKeyCodeArgs; *arg != '\0'; arg++)
    {
        std::cout << idx << "[" << arg << "] ";
        idx++;
    }
    std::cout << "\n";

    daemonProcess = fork();
    if (daemonProcess == 0) // If runnning the new process
    {
        int result = execv(INSTALL_PATH, (char* const*) trackedKeyCodeArgs);
    }
}

// If the KeyDaemon isn't already running, this launches the daemon and starts
// listening for key codes.
void KeyDaemonControl::startDaemon(const char* trackedKeyCodeString)
{
    if (daemonProcess != 0)
    {
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
        kill(daemonProcess, SIGTERM);
        sleep(daemonTermTimeout);
        if (isDaemonRunning()) // SIGTERM ignored, take more aggressive measures
        {
            kill(daemonProcess, SIGKILL);
            int daemonStatus;
            waitpid(daemonProcess, &daemonStatus, 0);
            exitCode = WEXITSTATUS(daemonStatus);
            daemonProcess = 0;
        }
        // If the pipeReader is still opening, it's stuck and the pipeInitThread
        // will need to be forcibly killed.
        if (pipeReader.getState() == InputReader::State::opening)
        {
            int cancelResult = pthread_cancel(pipeInitThread);
            if (cancelResult == 0)
            {
                pthread_join(pipeInitThread, nullptr);
            }
            else
            {
                std::cerr << messagePrefix << __func__ 
                        << ": pthread_cancel returned error code "
                        << cancelResult << ".\n";
            }
        }
        pipeReader.stopReading();
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
        std::cerr << messagePrefix << __func__ << " Error checking status:\n";
        perror(messagePrefix);
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
    std::cerr << messagePrefix << __func__ << ": Invalid wait result "
            << (int) waitResult << "\n";
    assert(false);
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
        std::cerr << messagePrefix << __func__ 
                << ": Daemon is still running!\n";
        assert(false);
        return 0;
    }
    return exitCode;
}


// Waits until the daemon process terminates and gets the process exit code.
int KeyDaemonControl::waitToExit()
{
    if (isDaemonRunning())
    {
        int daemonStatus;
        waitpid(daemonProcess, &daemonStatus, 0);
        return WEXITSTATUS(daemonStatus);
    }
    return exitCode;
}
