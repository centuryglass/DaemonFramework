#include "KeyEventFiles.h"
#include <iostream>
#include <fstream>
#include <sstream>

static const constexpr char* messagePrefix = "KeyDaemon: KeyEventFiles:";

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
        std::cerr << messagePrefix << "Failed to open device file "
                << devFilePath << "\n";
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
                    std::cout << "Found keyboard event file " << eventDirPath
                            << eventFile << "\n";
                    break;
                }
            }
        }
    }
    return paths;
}
