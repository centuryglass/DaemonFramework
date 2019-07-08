#include "KeyEventFiles.h"
#include "Debug.h"
#include <fstream>
#include <sstream>

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: KeyEventFiles::";
#endif

// Directory where input device event queues are found:
static const std::string eventDirPath = "/dev/input/";

// Path to the device file used to select appropriate event files:
static const constexpr char* devFilePath = "/proc/bus/input/devices";

// Prefix to lines that may contain event file names:
static const constexpr char* eventLinePrefix = "H: Handlers=";

// Substring found in event lines that handle keyboard input:
static const constexpr char* keyEventSubstring = "kbd";

// Gets paths for all valid keyboard input event files.
std::vector<std::string> KeyEventFiles::getPaths()
{
    using std::string;
    std::vector<string> paths;
    std::ifstream devFileReader(devFilePath);
    if (!devFileReader.is_open())
    {
        DBG(messagePrefix << __func__ << ": Failed to open device file \""
                << devFilePath << "\"");
        return paths;
    }
    string line;
    const int prefixLength = std::char_traits<char>::length(eventLinePrefix);
    while (std::getline(devFileReader, line))
    {
        if (line.compare(0, prefixLength, eventLinePrefix) == 0)
        {
            std::istringstream lineStream(line.substr(prefixLength));
            string eventFile, token;
            bool isKbdFile = false;
            while (lineStream >> token)
            {
                if (! isKbdFile && token.compare(keyEventSubstring) == 0)
                {
                    isKbdFile = true;
                }
                else if (eventFile.empty() && token.compare(0, 5, "event") == 0)
                {
                    eventFile = token;
                }
                if (isKbdFile && ! eventFile.empty())
                {
                    paths.push_back(eventDirPath + eventFile);
                    break;
                }
            }
        }
    }
    DBG_V(messagePrefix << __func__ << ": Found " << paths.size()
            << " keyboard event file paths.");
    return paths;
}
