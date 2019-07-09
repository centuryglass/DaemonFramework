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
#include <unistd.h>
#include "../../Include/KeyDaemonControl.h"

// Print the application name before all info/error output:
static const constexpr char* messagePrefix = "TestParent: ";

// Optional argument to print the daemon path and exit:
#define PRINT_PATH_ARG "-PrintDaemonPath"

#ifndef INSTALL_PATH
    #error "KeyDaemon install path INSTALL_PATH not defined!"
#endif


// Prints key codes read from the PipeReader:
class Listener : public PipeReader::Listener
{
private:
    virtual void keyEvent(const int keyCode, const KeyEventType type)
    {
        std::cout << "TestParent Listener: Read code " << keyCode << ", type "
                << ((int) type) << "\n";
    }
};


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << messagePrefix << "Found " << argc 
                << " arguments, expected 2.\n";
        return -1;
    }
    if (std::string(argv[1]) == PRINT_PATH_ARG)
    {
        std::cout << INSTALL_PATH;
        return 0;
    }
    Listener eventListener;
    KeyDaemonControl daemonController(&eventListener);
    std::cout << messagePrefix << "Starting KeyDaemon:\n";
    daemonController.startDaemon(argv[1]);
    if (!daemonController.isDaemonRunning())
    {
        std::cerr << messagePrefix << "Failed to start KeyDaemon thread.\n";
        return 1;
    }
    return daemonController.waitToExit();
}
