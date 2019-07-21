"""Defines all possible types of DaemonFramework test result."""
from enum import Enum

"""
Represents a result encountered before the daemon could exit.
"""
class InitCode(Enum):
    daemonBuildFailure = 50
    daemonInstallFailure = 51
    daemonInitSuccess = 52,
    parentBuildFailure = 53
    parentInstallFailure = 54,
    parentInitSuccess = 55,
    parentRunFailure = 56,
    parentRunSuccess = 57,
    daemonRunSuccess = 58

"""
Represents an exit code returned by a daemon.

These codes are also defined in 'Include/Implementation/ExitCode.h'.
"""
class ExitCode(Enum):
    success = 0
    badDaemonPath = 1
    badParentPath = 2
    insecureDaemonDir = 3
    insecureParentDir = 4
    daemonAlreadyRunning = 5
    daemonParentEnded = 6
    fdCleanupFailed = 7
    daemonExecFailed = 8

"""
Return a string describing an ExitCode or InitCode.
Keyword Arguments:
resultCode -- An ExitCode or InitCode value.
"""
def resultCodeDescription(resultCode):
    titleDict = { \
            ExitCode.success : \
                    'BasicDaemon exited normally.',
            ExitCode.badDaemonPath: \
                    'BasicDaemon was not installed at the required path.',
            ExitCode.badParentPath: \
                    'BasicParent was not installed at the required path.',
            ExitCode.insecureDaemonDir: \
                    'BasicDaemon was installed in an unsecured directory.',
            ExitCode.insecureParentDir: \
                    'BasicParent was installed in an unsecured directory.',
            ExitCode.daemonAlreadyRunning: \
                    'BasicDaemon was running in multiple processes.',
            ExitCode.daemonParentEnded: \
                    'BasicDaemon exited because BasicParent stopped running.',
            ExitCode.fdCleanupFailed: \
                    'Failed to clear open file table before launching daemon.',
            ExitCode.daemonExecFailed: \
                    'Failed to run BasicDaemon executable.',
            InitCode.daemonBuildFailure: \
                    'Failed to build BasicDaemon program.',
            InitCode.daemonInstallFailure: \
                    'Failed to install BasicDaemon program.',
            InitCode.daemonInitSuccess: \
                    'Built and installed BasicDaemon program.',
            InitCode.parentBuildFailure: \
                    'Failed to build BasicParent program.',
            InitCode.parentInstallFailure: \
                    'Failed to install BasicParent program.',
            InitCode.parentInitSuccess: \
                    'Built and installed BasicParent program.',
            InitCode.parentRunFailure: \
                    'Failed to run BasicParent.',
            InitCode.parentRunSuccess: \
                    'Successfully started BasicParent.',
            InitCode.daemonRunSuccess: \
                    'Successfully started BasicDaemon.'
    }
    if resultCode in titleDict:
        return titleDict[resultCode]
    return 'Unknown result code ' + str(resultCode)

"""Stores information about a specific test result."""
class Result:
    def __init__(self, resultCode, expectedCode):
        self._resultCode = resultCode
        self._expectedCode = expectedCode
    """Gets the test's result code."""
    def getResultCode(self):
        return self._resultCode
    """Gets the test's expected result code."""
    def getExpectedCode(self):
        return self_expectedCode
    """Checks if the test occurred as expected."""
    def testPassed(self):
        return self._resultCode == self._expectedCode
    """Gets a full description of the test's result."""
    def getResultText(self):
        return resultCodeDescription(self._resultCode)
    """Gets a full description of the test's expected result."""
    def getExpectedResultText(self):
        return resultCodeDescription(self._expectedCode)
