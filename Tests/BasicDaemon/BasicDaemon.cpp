#include "DaemonLoop.h"
#include <atomic>
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <ctime>
#include <cassert>
    
// Print the application name before all info/error output:
static const constexpr char* messagePrefix = "BasicDaemon: ";

// Input buffer size:
static const constexpr size_t bufSize = 128;

// Daemon loop duration in nanoseconds:
static const constexpr size_t loopNS = 10000;

// Message indicating that the daemon should exit:
static const constexpr char* exitMessage = "exit";

class BasicDaemon : public DaemonFramework::DaemonLoop
{
public:
    BasicDaemon() : DaemonFramework::DaemonLoop(bufSize), shouldExit(false)
    {
        std::cout << messagePrefix << "BasicDaemon object created.\n";
    }

    virtual ~BasicDaemon()
    {
        std::cout << messagePrefix << "BasicDaemon object destroyed.\n";
    }

private:
    virtual int initLoop() override
    {
        std::cout << messagePrefix << "Initializing daemon loop.\n";
        return 0;
    }

    virtual int loopAction() override
    {
        static bool firstRun = true;
        if (firstRun)
        {
            std::cout << messagePrefix << "Running loop action.\n";
            firstRun = false;
        }
        if (shouldExit)
        {
            return 1;
        }
        typedef std::chrono::nanoseconds Nanoseconds;
        typedef std::chrono::time_point<std::chrono::high_resolution_clock,
                Nanoseconds> Time;
        const Time loopStart = std::chrono::high_resolution_clock::now();
        static const Nanoseconds loopDuration(loopNS);
        struct timespec sleepTimer = {0, 0};
        // Do stuff here:
        // Sleep until loop duration ends:
        const Time loopEnd = std::chrono::high_resolution_clock::now();
        Nanoseconds timePassed = loopEnd - loopStart;
        if (timePassed < loopDuration)
        {
            Nanoseconds sleepTime = loopDuration - timePassed;
            sleepTimer.tv_nsec = sleepTime.count();
            nanosleep(&sleepTimer, nullptr);
        }
        return 0;
    }

    virtual void handleParentMessage(const unsigned char* messageData,
            const size_t messageSize) override
    {
        assert(messageSize <= bufSize);
        if (messageSize == 0)
        {
            std::cout << messagePrefix << "Received empty message.\n";
            return;
        }
        if (messageData[messageSize - 1] == 0)
        {
            std::string messageString((const char*) messageData);
            std::cout << messagePrefix << "Received message string "
                    << messageString << ", length " << messageSize << "\n";
            if (messageString == exitMessage)
            {
                std::cout << messagePrefix
                        << "Got exit message, requesting exit.\n";
                shouldExit = true;
            }
            return;
        }
        std::cout << messagePrefix << "Received non-string message of length "
                << messageSize << "\n";
        for (int i = 0; i < messageSize; i++)
        {
            if ((i % 16) == 0)
            {
                std::cout << "\n0x" << std::hex << i;
                std::cout << ":";
            }
            std::cout << "  0x" << std::hex << messageData[i];
        }
        std::cout << "\n";
    }
private:
    unsigned char buffer [bufSize] = {0};
    std::atomic_bool shouldExit;
};

int main(int argc, char** argv)
{
    BasicDaemon daemon;
    return daemon.runLoop();
}
