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
#include "../../Parent_Include/PipeReader.h"

#define KEY_PIPE_PATH ".keyPipe";

std::atomic_int returnValue;

// Used to pass program arguments to a new thread.
struct SharedArgs
{
    int argc;
    char** argv;
};

// Runs KeyDaemon within a new thread.
void* daemonThreadAction(void* args)
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

// Starts the key pipe reader within a new thread.
void* pipeThreadAction(void* codePipe)
{
    PipeReader* pipe = static_cast<PipeReader*>(codePipe);
    pipe->startReading();
    return nullptr;
}

// Prints key codes read from the PipeReader:
class Listener : public PipeReader::Listener
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
    PipeReader pipeReader(&eventListener);
    pthread_t daemonThread = 0;
    SharedArgs args = {argc, argv};
    int threadError = pthread_create(&daemonThread, nullptr, daemonThreadAction,
            (void *) &args);
    if (threadError != 0)
    {
        std::cerr << "Failed to start KeyDaemon thread.\n";
        return 1;
    }
    // pipeReader starts in its own thread so that the application doesn't hang
    // if the KeyDaemon fails and the pipe can't be opened:
    pthread_t pipeOpenThread = 0;
    threadError = pthread_create(&pipeOpenThread, nullptr, pipeThreadAction,
            (void *) &pipeReader);
    pthread_join(daemonThread, nullptr);
    return returnValue;
}
