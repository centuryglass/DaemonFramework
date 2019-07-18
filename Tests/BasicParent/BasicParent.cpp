/**
 * @file  TestParent.cpp
 * 
 * @brief  A minimal parent application implementation used when testing
 *         the daemon framework.
 *
 *  A daemon installation is intended to support only one executable, and will
 * refuse to work unless the process that starts it is running that specific
 * parent executable. TestParent is a minimal application meant to serve as that
 * parent application when testing the DaemonFramework.
 */

#include <string>
#include <iostream>
#include <stdlib.h>
#include <limits>
#include <unistd.h>
#include "DaemonControl.h"

// Print the application name before all info/error output:
static const constexpr char* messagePrefix = "TestParent: ";

// Optional argument to print the daemon path and exit:
#define PRINT_PATH_ARG "-PrintDaemonPath"

#ifndef DF_DAEMON_PATH
    #error "Daemon install path DF_DAEMON_PATH not defined!"
#endif

// Daemon message buffer size:
static const constexpr int pipeBufSize = 128;

// Prints key codes read from the PipeReader:
class Listener : public DaemonFramework::Pipe::Listener
{
private:
    virtual void processData(const unsigned char* data, const size_t size)
    {
        std::cout << messagePrefix << __func__ << ": Read " << size
                << " bytes of data.\n";
    }
};


int main(int argc, char** argv)
{
    using namespace DaemonFramework;
    if (argc > 1 && std::string(argv[1]) == PRINT_PATH_ARG)
    {
        std::cout << DF_DAEMON_PATH;
        return 0;
    }
    Listener eventListener;
    DaemonControl daemonController(&eventListener, pipeBufSize);
    std::cout << messagePrefix << "Starting daemon at \"" << DF_DAEMON_PATH
            << "\"\n";
    std::vector<std::string> args;
    if (argc > 1)
    {
        args.push_back(std::string(argv[1]));
    }
    daemonController.startDaemon(args);
    if (!daemonController.isDaemonRunning())
    {
        std::cerr << messagePrefix << "Failed to start daemon thread.\n";
        return 1;
    }
    const int retVal = daemonController.waitToExit();
    std::cout << messagePrefix << "Daemon exited returning " << retVal << "\n";
    return retVal;
}
