#include "KeyReader.h"
#include "KeyCode.h"
#include "CodePipe.h"
#include "Process_Security.h"
#include <iostream>
#include <unistd.h>

#ifndef KEYPATH
    #define KEYPATH ""
#endif

static const constexpr char* keyPath = "/dev/input/event7";
static int lastCode = 0;


int main(int argc, char** argv)
{
    std::cout << "Launched KeyDaemon.\n";
    CodePipe pipe(".keyPipe");
    Process::Security security;
    if (! security.appProcessSecured() || ! security.parentProcessSecured())
    {
        std::cerr << "Insufficient security, stopping key daemon.\n";
        return 1;
    }
    
    // read code arguments:
    std::vector<int> testCodes = KeyCode::parseCodes(argc, argv);
    if (testCodes.empty())
    {
        std::cerr << "Failed to get valid test codes, stopping key daemon.\n";
        return 1;
    }
    KeyReader reader(keyPath, testCodes, &pipe);
    while (security.parentProcessRunning())
    {
#ifdef TIMEOUT
        sleep(TIMEOUT);
        std::cout << "KeyDaemon: Timeout period ended, exiting normally.\n";
        reader.stopReading();
        return 0;
#else
        sleep(5);
#endif
    }
}

