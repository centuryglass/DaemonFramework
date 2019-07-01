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
    virtual void keyEvent(const int keyCode, const KeyEventType type) override
    {
        if (type != KeyEventType::held)
        {
            std::cout << "Key " << keyCode << ((type == KeyEventType::pressed)
                    ? " pressed\n" : " released\n");
        }
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
    KeyReader reader(keyPath, testCodes, &listener);
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

