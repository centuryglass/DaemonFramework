/**
 * @file  TestParent.cpp
 * 
 * @brief  A minimal parent application implementation used when testing
 *         KeyDaemon.
 *
 *  A KeyDaemon installation is intended to support only one executable,
 * and will refuse to work unless the process that starts it is running that
 * specific parent executable. TestParent is a minimal application meant to
 * serve as that parent application when testing KeyDaemon.
 */

#include <string>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include <atomic>
#include <unistd.h>
#include <pthread.h>
#include "../../Parent_Include/CodePipe.h"

static const constexpr char* pipePath = ".keyPipe";

std::atomic_int returnValue;

// Used to pass program arguments to a new thread.
struct SharedArgs
{
    int argc;
    char** argv;
};

// Used to run KeyDaemon within a new thread.
void* threadAction(void* args)
{
    SharedArgs* launchArgs = static_cast<SharedArgs*>(args);
    std::cout << "TestParent: Running from " << launchArgs->argv[0] << "\n";
    std::string command(launchArgs->argv[1]);
    for (int i = 2; i < launchArgs->argc; i++)
    {
        command += " ";
        command += launchArgs->argv[i];
    }
    std::cout << "TestParent: Starting key daemon using command \"" 
            << command << "\"\n";
    returnValue = WEXITSTATUS(system(command.c_str()));
    return nullptr;
}

// Prints key codes read from the CodePipe:
class Listener : public CodePipe::Listener
{
private:
    virtual void keyEvent(const int keyCode, const KeyEventType type)
    {
        std::cout << "TestParent Listener: Read code " << keyCode
                << ", type " << ((int) type) << "\n";
    }
};


int main(int argc, char** argv)
{
    const int maxInt = std::numeric_limits<int>::max();
    returnValue = maxInt;
    Listener eventListener;
    CodePipe pipeReader(pipePath, &eventListener);
    pthread_t daemonThread = 0;
    SharedArgs args = {argc, argv};
    int threadError = pthread_create(&daemonThread, nullptr, threadAction,
            (void *) &args);
    if (threadError != 0)
    {
        std::cerr << "Failed to start KeyDaemon thread.\n";
        return 1;
    }
    pipeReader.startReading();
    pthread_join(daemonThread, nullptr);
    return returnValue;
}
