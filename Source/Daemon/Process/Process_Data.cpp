#include "Process_Data.h"
#include "Process_State.h"
#include "../Debug.h"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <fstream>
#include <cassert>

// Indices of process data members within the process stat file:
static const constexpr int idIndex        = 0;
static const constexpr int stateIndex     = 2;
static const constexpr int parentIdIndex  = 3;
static const constexpr int startTimeIndex = 21;

// Process stat file name before the pid directory:
static const std::string pathPre = "/proc/";
// Process stat file name after the pid directory:
static const std::string pathPost = "/stat";


// Reads process data from the system.
DaemonFramework::Process::Data::Data(const int processId) :
Data(pathPre + std::to_string(processId) + pathPost)
{
    // The parsed ID should always match the constructor ID if valid:
    DF_ASSERT(!isValid() || this->processId == processId);
}


// Creates an empty, invalid data object.
DaemonFramework::Process::Data::Data() : lastState(State::invalid) { }


// Sorts processes by launch time, newest first.
class
{
public:
    bool operator() (DaemonFramework::Process::Data first,
            DaemonFramework::Process::Data second)
    {
        return second.getStartTime() < first.getStartTime();
    }
} processComparator;


// Gets data for all direct child processes of the process this Data object
// represents.
std::vector<DaemonFramework::Process::Data>
DaemonFramework::Process::Data::getChildProcesses()
{
    using std::vector;
    using std::string;
    string processPath("/proc");
    DIR* processDir = opendir(processPath.c_str());
    struct dirent* dirEntry;
    vector<string> childDirs;
    while ((dirEntry = readdir(processDir)) != nullptr)
    {
        if (dirEntry->d_type == DT_DIR)
        {
            childDirs.push_back(dirEntry->d_name);
        }
    }
    closedir(processDir);
    processDir = nullptr;

    vector<Data> childProcs;
    for (const string& dir : childDirs)
    {
        int childID = std::stoi(dir.substr(processPath.size() + 1));
        if (childID > 0)
        {
            Data processData(childID);
            if (processData.parentId == processId)
            {
                childProcs.push_back(processData);
            }
        }
    }
    std::sort(childProcs.begin(), childProcs.end(), processComparator);
    return childProcs;
}


// Updates this data with the current process state, invalidating it if a new
// process is using its saved process ID.
void DaemonFramework::Process::Data::update()
{
    Data updatedData(processId);
    if (updatedData.executablePath == executablePath)
    {
        *this = updatedData;
    }
    else
    {
        *this = Data();
    }
}


// Checks whether this object found process data on construction.
bool DaemonFramework::Process::Data::isValid() const
{
    return lastState != State::invalid;
}


// Gets the ID of the process this Data object represents.
int DaemonFramework::Process::Data::getProcessId() const
{
    return processId;
}


// Gets the ID of the parent process of this Data process.
int DaemonFramework::Process::Data::getParentId() const
{
    return parentId;
}


// Gets the name of the executable this Data process was created to run.
std::string DaemonFramework::Process::Data::getExecutablePath() const
{
    return executablePath;
}


// Gets the state of the Data process recorded when it was constructed.
DaemonFramework::Process::State
DaemonFramework::Process::Data::getLastState() const
{
    return lastState;
}


// Gets the time this process was created.
unsigned long DaemonFramework::Process::Data::getStartTime() const
{
    return startTime;
}


// Create process data directly from the process stat file object.
DaemonFramework::Process::Data::Data(const std::string statFile)
{
    std::ifstream fileStream;
    fileStream.open(statFile);
    if (fileStream.is_open())
    {
        // Parse process info from stat file strings:
        std::vector<std::string> statItems;
        std::string statItem;
        while (fileStream >> statItem)
        {
            statItems.push_back(statItem);
        }
        fileStream.close();

        processId = std::stoi(statItems[idIndex]);
        parentId = std::stoi(statItems[parentIdIndex]);
        startTime = std::stoul(statItems[startTimeIndex]);
        lastState = readStateChar(statItems[stateIndex][0]);

        // Read executable path from the link within the process directory:
        std::string exePath("/proc/");
        exePath += statItems[idIndex] + "/exe";
        char buffer[PATH_MAX];
        ssize_t length = readlink(exePath.c_str(), buffer, sizeof(buffer) - 1);
        if (length != -1)
        {
            buffer[length] = '\0';
            executablePath = buffer;
        }
    }
    else
    {
        lastState = State::invalid;
    }
}
