/**
 * @file  ExitCode.h
 *
 * @brief  Defines the exit codes the KeyDaemon may return.
 */

#pragma once
enum class ExitCode
{
    // Normal exit, no errors:
    success = 0,
    // The KeyDaemon process was not launched with the expected executable path:
    badDaemonPath,
    // The parent executable is not located at the expected path:
    badParentPath,
    // The KeyDaemon executable is in an unsecured directory:
    insecureDaemonDir,
    // The parent executable is in an unsecured directory:
    insecureParentDir,
    // Tracked key code parameters were invalid:
    badTrackedKeys,
    // No valid keyboard event files were found:
    missingKeyEventFiles
};
