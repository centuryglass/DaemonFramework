/**
 * @file  ExitCode.h
 *
 * @brief  Defines the default list of errors a daemon may return.
 */

#pragma once

namespace DaemonFramework { enum class ExitCode; }

enum class DaemonFramework::ExitCode
{
    // Normal exit, no errors:
    success = 0,
    // The daemon process was not launched with the expected executable path:
    badDaemonPath = 1,
    // The parent executable is not located at the expected path:
    badParentPath = 2,
    // The daemon executable is in an unsecured directory:
    insecureDaemonDir = 3,
    // The parent executable is in an unsecured directory:
    insecureParentDir = 4,
    // The daemon is already running:
    daemonAlreadyRunning = 5,
    // The daemon's parent process is not running:
    daemonParentEnded = 6,
    // Unable to clean up open file descriptors before running the daemon:
    fdCleanupFailed = 7,
    // Unable to run the daemon executable:
    daemonExecFailed = 8
};
