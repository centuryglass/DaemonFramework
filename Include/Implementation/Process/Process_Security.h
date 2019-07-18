/**
 * @file  Process_Security.h
 *
 * @brief  Keeps the application secured by checking the state of its process
 *         and parent process.
 */

#pragma once
#include "Process_Data.h"
#include <string>

namespace DaemonFramework
{
    namespace Process
    {
        class Security;
    }
}

/**
 * @brief  Ensures that this application's process and its parent process have
 *         certain traits in order to secure the application.
 * 
 *  Process::Security only checks if the process follows security rules.
 * Actually handling rule violations must be done elsewhere.
 *
 *  Any of these rules may be disabled in build configuration. If disabled, the
 * function that checks the rule will be removed.
 *
 *  Application process security:
 *
 *   This process should be running from a specific path, set at compile time.
 *  That path should be within a folder that may only be modified with root
 *  permissions. If these conditions are not met, the application is not secured
 *  and should exit.
 *
 *  Parent process security:
 *
 *   This application may only be launched by a process that is running a
 *  specific executable at a specific path, set at compile time.  That
 *  executable must be in a directory that can only be modified with root
 *  permissions. If these conditions are not met, the application is not secured
 *  and should exit.
 *
 *   This application should only run as long as its parent process is running.
 *  if it detects that the parent process has closed, it should immediately
 *  exit.
 *
 */
class DaemonFramework::Process::Security
{
public:
    /**
     * @brief  Loads process data on construction.
     */
    Security();

    ~Security() { }

#   ifdef DF_VERIFY_PATH
    /**
     * @brief  Checks if the daemon executable is running from the expected
     *         path.
     *
     * @return   Whether the daemon is running from the correct executable path,
     *           as set at compile time.
     */
    bool validDaemonPath();
#   endif

#   ifdef DF_REQUIRED_PARENT_PATH
    /**
     * @brief  Checks if the daemon was launched by an executable at the
     *         expected path.
     *
     * @return  Whether the daemon's parent process is running from the correct
     *          executable path, as set at compile time.
     */
    bool validParentPath();
#   endif

#   ifdef DF_VERIFY_PATH_SECURITY
    /**
     * @brief  Checks if the daemon's directory is secure.
     *
     * @return  Whether the daemon's executable is in a directory that can only
     *          be modified by root.
     */
    bool daemonPathSecured();
#   endif

#   ifdef DF_VERIFY_PARENT_PATH_SECURITY
    /**
     * @brief  Checks if the parent application's directory is secure.
     *
     * @return  Whether the parent application's directory can only be modified
     *          by root.
     */
    bool parentPathSecured();
#   endif

#   ifdef DF_REQUIRE_RUNNING_PARENT
    /**
     * @brief  Checks if this application's parent process is still running.
     *
     * @return  Whether the parent process is running.
     */
    bool parentProcessRunning();
#   endif

#   ifdef DF_REQUIRE_SINGULAR
    /**
     * @brief  Checks that only one daemon process is running.
     *
     * @return  Whether only one process is running the daemon's executable.
     */
    bool daemonProcessIsSingular();
#   endif

private:
    /**
     * @brief  Checks if a specific process is running from a specific expected
     *         path within a secure directory.
     *
     * @param process  Data describing the process to check.
     *
     * @param path     A path to an executable the process should be running.
     *
     * @return         Whether the process is running from the expected path.
     */
    bool processSecured
    (const Process::Data& process, const std::string& path) const;

    /**
     * @brief  Checks if a given directory is secure.
     *
     * @param dirPath  The absolute path to a directory.
     *
     * @return         Whether the given directory can only be modified with
     *                 root permissions.
     */
    bool directorySecured(const std::string& dirPath) const;

    // The daemon's process data:
    Process::Data daemonProcess;
    // The parent process data:
    Process::Data parentProcess;
};
