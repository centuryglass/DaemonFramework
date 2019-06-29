#include "KeyReader.h"
#include "KeyCode.h"
#include "Process_Security.h"
#include <iostream>
#include <unistd.h>

#ifndef KEYPATH
    #define KEYPATH ""
#endif

static const constexpr char* keyPath = "/dev/input/event7";
static int lastCode = 0;

class KeyListener : public KeyReader::Listener
{
public:
    KeyListener() { }

private:
    virtual void keyPressed(const int keyCode) override
    {
        std::cout << "Key " << keyCode << " pressed.\n";
    }

    virtual void keyReleased(const int keyCode) override
    {
        std::cout << "Key " << keyCode << " released.\n";
        lastCode = keyCode;
    }
};

int main(int argc, char** argv)
{
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

    KeyListener listener;
    KeyReader reader(KEYPATH, testCodes, &listener);
    while (security.parentProcessRunning())
    {
#ifdef TIMEOUT
        sleep(TIMEOUT);
        return 0;
#else
        sleep(5);
#endif
    }
}

