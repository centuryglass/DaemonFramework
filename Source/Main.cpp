#include "KeyReader.h"
#include "KeyCode.h"
#include "PipeWriter.h"
#include "Process_Security.h"
#include "KeyEventFiles.h"
#include "ExitCode.h"
#include "Debug.h"
#include <vector>
#include <unistd.h>
#include <signal.h>

#ifdef DEBUG
// Text to print before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: Main: ";
#endif

// Seconds to wait between each check on whether the parent process is running:
static const constexpr int secondsBetweenParentChecks = 2;

// Whether the daemon has received a termination signal:
volatile sig_atomic_t termSignalReceived = 0;

// Sets termSignalReceived when a termination signal is caught:
void flagTermSignal(int signum)
{
    ASSERT(signum == SIGTERM);
    DBG(messagePrefix << __func__ << ": Received SIGTERM");
    termSignalReceived = 1;
}


int main(int argc, char** argv)
{
    // Initialize termination signal handling:
    struct sigaction action = {};
    action.sa_handler = flagTermSignal;
    sigaction(SIGTERM, &action, NULL);

    // Check security requirements:
    Process::Security security;
    if (! security.validDaemonPath())
    {
        DBG(messagePrefix << "Exiting, invalid daemon executable path.");
        return (int) ExitCode::badDaemonPath;
    }
    if (! security.validParentPath())
    {
        DBG(messagePrefix << "Exiting, invalid parent executable path.");
        return (int) ExitCode::badParentPath;
    }
    if (! security.daemonPathSecured())
    {
        DBG(messagePrefix 
                << "Exiting, daemon executable is in an unsecured directory.");
        return (int) ExitCode::insecureDaemonDir;
    }
    if (! security.parentPathSecured())
    {
        DBG(messagePrefix 
                << "Exiting, parent executable is in an unsecured directory.");
        return (int) ExitCode::insecureParentDir;
    }
    
    // read code arguments:
    std::vector<int> testCodes = KeyCode::parseCodes(argc, argv);
    if (testCodes.empty())
    {
        DBG(messagePrefix << "Exiting: tracked key codes were invalid.");
        return (int) ExitCode::badTrackedKeys;
    }
    PipeWriter pipe;

    // Create KeyReader objects for each keyboard event file:
    std::vector<KeyReader*> eventFileReaders;
    std::vector<std::string> eventFilePaths = KeyEventFiles::getPaths();
    DBG_V(messagePrefix << "Creating KeyReader objects for "
            << eventFilePaths.size() << " event files:");
    for (const std::string& path : eventFilePaths)
    {
        eventFileReaders.push_back(
                new KeyReader(path.c_str(), testCodes, &pipe));
    }
    if (eventFileReaders.empty())
    {
        DBG(messagePrefix << "Exiting: no valid event files found.");
        return (int) ExitCode::missingKeyEventFiles;
    }

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
                DBG(messagePrefix << "Reader for path \"" 
                        << eventFileReaders[i]->getPath() 
                        << "\" stopped unexpectedly, " 
                        << (eventFileReaders.size() - 1) 
                        << " readers remaining.");
                KeyReader* removedReader = eventFileReaders[i];
                eventFileReaders.erase(eventFileReaders.begin() + i);
                delete removedReader;
                i--;
            }
        }
#ifdef TIMEOUT
        sleep(TIMEOUT);
        DBG(messagePrefix << "Timeout period ended, shutting down daemon.");
        break;
#else
        sleep(secondsBetweenParentChecks);
#endif
    }
    DBG_V(messagePrefix << __func__ << ": Closing " << eventFileReaders.size()
            << " KeyReader objects:");

    for (KeyReader* reader : eventFileReaders)
    {
        reader->stopReading();
        delete reader;
    }
    eventFileReaders.clear();
    pipe.closePipe();
    DBG(messagePrefix << "Exiting KeyDaemon.");
    return (int) ExitCode::success;
}
