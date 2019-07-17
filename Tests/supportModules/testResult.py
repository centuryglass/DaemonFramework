"""Defines all possible types of DaemonFramework test result."""
from enum import Enum

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
Represents an error encountered before the daemon could run.
"""
class FailureCode(Enum):
    daemonBuildFailure = 50
    daemonInstallFailure = 51
    parentBuildFailure = 52
    parentInstallFailure = 53
    parentRunFailure = 54

"""
Return a string describing an ExitCode or FailureCode.
Keyword Arguments:
resultCode -- An ExitCode or FailureCode value.
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
            FailureCode.daemonBuildFailure: \
                    'Failed to build BasicDaemon program.',
            FailureCode.daemonInstallFailure: \
                    'Failed to install BasicDaemon program.',
            FailureCode.parentBuildFailure: \
                    'Failed to build BasicParent program.',
            FailureCode.parentInstallFailure: \
                    'Failed to install BasicParent program.',
            FailureCode.parentRunFailure: \
                    'Failed to run BasicParent.'
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
        if self.testPassed():
            return 'Test Passed: ' + resultCodeDescription(self._resultCode)
        else:
            return 'Test Failed: ' + resultCodeDescription(self._resultCode) + \
                ' Expected result: ' + resultCodeDescription(self._expectedCode)
    """Prints the full description of the test's results to stdout."""
    def printResultText(self):
        print(self.getResultText())
