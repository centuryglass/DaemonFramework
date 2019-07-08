/**
 * @file  Process_Data.h
 *
 * @brief  Stores information about a Linux process.
 */

#pragma once
#include <sys/types.h>
#include <vector>
#include <string>

namespace Process
{
    class Data;
    enum class State;
}

/**
 * @brief  Reads and stores basic information about a single process.
 */
class  Process::Data
{
public:
    /**
     * @brief  Reads process data from the system.
     *
     * @param processId  The system process ID used to look up the process.
     */
    Data(const int processId);

    /**
     * @brief  Creates an empty, invalid data object.
     */
    Data();

    ~Data() { }

    /**
     * @brief  Gets data for all direct child processes of the process this
     *         Data object represents.
     *
     * @return  An array of process data structures, containing Data objects
     *          for each process that lists the stored processId as their
     *          parent ID. This array will be sorted with the newest elements
     *          listed first.
     */
    std::vector<Data> getChildProcesses();

    /**
     * @brief  Updates this data with the current process state, invalidating
     *         it if a new process is using its saved process ID.
     */
    void update();

    /**
     * @brief  Checks whether this object found process data on construction.
     *
     * @return  True if process data was accessed, false if no process was
     *          found using the process ID given on construction.
     */
    bool isValid() const;

    /**
     * @brief  Gets the ID of the process this Data object represents.
     *
     * @return  The system process ID given to this process on construction.
     */
    int getProcessId() const;

    /**
     * @brief  Gets the ID of the parent process of this Data process.
     *
     * @return  The parent process Id, or -1 if this Data is invalid.
     */
    int getParentId() const;

    /**
     * @brief  Gets the path to the executable this Data process was created to
     *         run.
     *
     * @return  The process executable path, or the empty string if this Data
     *          is invalid.
     */
    std::string getExecutablePath() const;

    /**
     * @brief  Gets the state of the Data process recorded when it was
     *         constructed or last updated.
     *
     * @return  The saved process state, or State::invalid if this Data is
     *          invalid.
     */
    State getLastState() const;

    /**
     * @brief  Gets the time this process was created, expressed in number of
     *         clock ticks since system boot.
     *
     * @return  The process start time, or zero if this Data is invalid.
     */
    unsigned long getStartTime() const;

private:
    /**
     * @brief  Create process data directly from the process stat file object.
     *
     * @param statFile  The path to a /proc/<pid>/stat file.
     */
    Data(const std::string statFile);

    // Process ID number:
    int processId = -1;
    // ID of the parent process that created this process:
    int parentId = -1;
    // Executable the process was created to launch:
    std::string executablePath;
    // Last recorded process state:
    State lastState;
    // Time when the process was started:
    unsigned long startTime = 0;
};
