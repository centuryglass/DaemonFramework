#include "KeyReader.h"
#include "KeyCode.h"
#include "PipeWriter.h"
#include "Process_Security.h"
#include "KeyEventFiles.h"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>

// Seconds to wait between each check on whether the parent process is running:
static const constexpr int secondsBetweenParentChecks = 2;

// Whether the daemon has received a termination signal:
volatile sig_atomic_t termSignalReceived = 0;

// Sets termSignalReceived when a termination signal is caught:
void flagTermSignal(int signum)
{
    termSignalReceived = 1;
}


int main(int argc, char** argv)
{
    std::cout << "Launched KeyDaemon.\n";
    // Initialize termination signal handling:
    struct sigaction action = {};
    action.sa_handler = flagTermSignal;
    sigaction(SIGTERM, &action, NULL);

    // Check security requirements:
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
    PipeWriter pipe;

    // Create KeyReader objects for each keyboard event file:
    std::vector<KeyReader*> eventFileReaders;
    std::vector<std::string> eventFilePaths = KeyEventFiles::getPaths();
    std::cout << "Found " << eventFilePaths.size() << " paths.\n";
    for (const std::string& path : eventFilePaths)
    {
        eventFileReaders.push_back(
                new KeyReader(path.c_str(), testCodes, &pipe));
    }
    std::cout << "Created " << eventFileReaders.size() << " readers.\n";

    // Allow KeyReaders to run, periodically removing failed readers and
    // checking that the parent is still running.
    while (security.parentProcessRunning() && ! eventFileReaders.empty()
            && termSignalReceived == 0)
    {
        // Find and remove failed file readers:
        for (int i = 0; i < eventFileReaders.size(); i++)
        {
            InputReader::State readerState = eventFileReaders[i]->getState();
            if (readerState == InputReader::State::closed 
                    || readerState == InputReader::State::failed)

            {
                std::cerr << "Reader for path \"" 
                        << eventFileReaders[i]->getPath() 
                        << "\" stopped unexpectedly.\n";
                KeyReader* removedReader = eventFileReaders[i];
                eventFileReaders.erase(eventFileReaders.begin() + i);
                delete removedReader;
                i--;
            }
        }
#ifdef TIMEOUT
        sleep(TIMEOUT);
        std::cout << "KeyDaemon: Timeout period ended, exiting normally.\n";
        break;
#else
        sleep(secondsBetweenParentChecks);
#endif
    }
    for (KeyReader* reader : eventFileReaders)
    {
        reader->stopReading();
        delete reader;
    }
    eventFileReaders.clear();
    return 0;
}
