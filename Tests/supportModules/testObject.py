"""
Tests building, installing, and running the BasicDaemon and BasicParent.
"""
import os
import subprocess
import tempfile
import time
import sys
from supportModules import make, pathConstants, testResult, testArgs
from supportModules.testResult import InitCode, ExitCode, Result
from supportModules.pathConstants import paths

"""Runs and logs a set of related DaemonFramework tests."""
class Test:
    """
    Saves test properties on construction.
    Keyword Arguments:
    title        -- A title describing the set of tests.
    testFunction -- A function that runs the full set of tests. 
                    This function takes the Test object as its sole parameter.
    testCount    -- The number of tests the testFunction will run.
    args         -- A testArgs.Values object storing command line test options.
    """
    def __init__(self, title, testFunction, testCount, args):
        self._title = '- ' + title
        self._runTests = testFunction
        self._testCount = testCount
        self._testIndex = 0
        self._passedTests = 0
        self._tempLinePrinted = False
        self._args = args
    """Return the number of individual tests that the object runs."""
    @property
    def testCount(self):
        return self._testCount
    """
    Runs all test actions, printing the results.
    Returns the number of tests that passed.
    """
    def runAll(self):
        print(self._title)
        self.setup()
        self._testIndex = 0
        self._passedTests = 0
        self._runTests(self)
        self._eraseTempLine()
        if self._testIndex != self._testCount:
            print('  Expected ' + str(self._testCount) + ' tests, ' \
                  + 'but only ran ' + str(self._testIndex))
        print('  Passed ' + str(self._passedTests) + ' of ' \
              + str(self._testIndex) + ' tests.')
        return self._passedTests
    """
    Ensures the testing environment is properly set up.
    """
    def setup(self):
        self._printTempLine('Initial test setup:')
        # Remove old failure log instances:
        if os.path.isfile(paths.failureLogPath):
            os.remove(paths.failureLogPath)
        # Ensure unsecured test directory is initialized:
        if not os.path.isdir(paths.unsecureExeDir):
            os.mkdir(paths.unsecureExeDir)
            subprocess.call('chmod "o=w" ' + paths.unsecureExeDir, shell = True)
        # Ensure secured test directory is initialized:
        if not os.path.isdir(paths.secureExeDir):
            os.mkdir(paths.secureExeDir)
            subprocess.call('sudo chown root.root ' + paths.secureExeDir, \
                            shell = True)
            subprocess.call('sudo chmod "o-w" ' + paths.secureExeDir, \
                            shell = True)
        os.chdir(paths.projectDir)
    """
    Returns either a log file opened for appending, or /dev/null if not logging.
    Keyword Arguments:
    logOutput  -- Whether logs should be saved.
    """
    def _openOutFile(self, logOutput):
        return open(paths.tempLogPath, 'a') if logOutput else subprocess.DEVNULL
    """
    Prints a temporary line of output to the console.
    Keyword Arguments:
    line -- The line of text to print.
    """
    def _printTempLine(self, line):
        self._eraseTempLine()
        print('  Test ' + str(self._testIndex + 1) + ': ' + line)
        self._tempLinePrinted = True
    """
    Erases temporary output from the console.
    """
    def _eraseTempLine(self):
        if self._tempLinePrinted and not self._args.useVerbose:
            cursorUp = '\x1b[1A'
            eraseLine = '\x1b[2K'
            print(cursorUp + eraseLine + cursorUp)
            self._tempLinePrinted = False
            
    """
    Build and install the daemon, returning an appropriate InitCode.
    Keyword Arguments:
    makeArgs    -- The set of arguments to pass to the `make` process.
    logOutput   -- Whether test output will be saved to the test log.
                   (default: True)
    """
    def daemonBuildInstall(self, makeArgs, logOutput = True):
        with self._openOutFile(logOutput) as outFile:
            self._printTempLine('Cleaning daemon:')
            make.cleanDaemon(outFile)
            self._printTempLine('Building daemon:')
            if not make.buildBasicDaemon(makeArgs, outFile):
                return InitCode.daemonBuildFailure
            self._printTempLine('Installing daemon:')
            if not make.installBasicDaemon(makeArgs, outFile):
                return InitCode.daemonInstallFailure
            return InitCode.daemonInitSuccess
    """
    Build and install the parent, returning an appropriate InitCode.
    Keyword Arguments:
    makeArgs    -- The set of arguments to pass to the `make` process.
    logOutput   -- Whether test output will be saved to the test log.
                   (default: True)
    """
    def parentBuildInstall(self, makeArgs, logOutput = True):
        with self._openOutFile(logOutput) as outFile:
            self._printTempLine('Cleaning parent:')
            make.cleanParent(outFile)
            self._printTempLine('Building parent:')
            if not make.buildBasicParent(makeArgs, outFile):
                return InitCode.parentBuildFailure
            self._printTempLine('Installing parent:')
            if not make.installBasicParent(makeArgs, outFile):
                return InitCode.parentInstallFailure
            return InitCode.parentInitSuccess
    """
    Runs an executable, returning an appropriate InitCode or ExitCode.
    Keyword Arguments:
    execPath   -- The full path to the executable that should run.
    argList    -- An optional list of arguments to pass to the executable.
    logOutput  -- Whether test output will be saved to the test log.
                  (default: True)
    """
    def execTest(self, execPath, argList = [], logOutput = True): 
        with self._openOutFile(logOutput) as outFile:
            execArgs = [execPath] + argList
            exitCode = ExitCode.success
            try:
                self._printTempLine('Running ' + os.path.basename(execPath) \
                                    + ':')
                completedProcess = subprocess.run(execArgs, stdout = outFile, \
                                                  stderr = outFile)
                exitCode = ExitCode(completedProcess.returncode)
            except OSError as e:
                exitCode = InitCode.parentRunFailure
            return exitCode

    """
    Attempts to clean, build, install, and test the parent and daemon.

    Keyword Arguments:
    makeArgs        -- The set of arguments to pass to the `make` process.
    execPath        -- The full path to the executable that should run.
    expectedOutcome -- The ExitCode or InitCode that the test should return.
                       (default: ExitCode.success)
    argList         -- An optional list of arguments to pass to the BasicParent.
                       (default: [])
    logOutput       -- Whether test output will be saved to the test log.
                       (default: True)
    Return a Result object describing the test's actual and expected results.
    """
    def fullTest(self, makeArgs, execPath, expectedOutcome = ExitCode.success, \
                 argList = [], logOutput = True):
        buildResult = self.parentBuildInstall(makeArgs, logOutput)
        if buildResult is not InitCode.parentInitSuccess:
            return Result(buildResult, expectedOutcome)
        buildResult = self.daemonBuildInstall(makeArgs, logOutput)
        if buildResult is not InitCode.daemonInitSuccess:
            return Result(buildResult, expectedOutcome)
        runResult = self.execTest(execPath, argList, logOutput)
        return Result(runResult, expectedOutcome)
    """
    Check on the result of a test, reporting whether it succeeded or failed.
    This also clears the test output file, copying its text to the failure log
    file first if the test failed.
    Keyword Arguments:
    result      -- The Result object describing the test's outcome.
    description -- A description of the specific test that's being checked.
    Returns true if the test result was as expected, false otherwise.
    """
    def checkResult(self, result, description):
        self._testIndex += 1
        testPrefix = '  ' + str(self._testIndex) + '/' + str(self._testCount) \
                     + (': PASS: ' if result.testPassed() else ': FAIL: ')
        lineMargin = ' ' * len(testPrefix)
        self._eraseTempLine()
        print(testPrefix + description)
        print(lineMargin + 'Result: ' + result.getResultText())
        if result.testPassed():
            self._passedTests += 1
            if (os.path.isfile(paths.tempLogPath)):
                os.remove(paths.tempLogPath)
            return True
        else:
            print(lineMargin + 'Expected: ' + result.getExpectedResultText())
            print(lineMargin + 'See ' + paths.failureLog + \
                  ' for more information.')
            if (os.path.isfile(paths.tempLogPath)):
                with open(paths.tempLogPath, 'r') as tempLog:
                    with open(paths.failureLogPath, 'a') as failureLog:
                        failureLog.write('\n' + index + ' ' + description + '\n')
                        failureLog.write('    ' + result.getResultText() + '\n')
                        failureLog.write('Test output:\n')
                        errorLines = tempLog.readlines()
                        errorLines = [ '\t' + line for line in errorLines]
                        failureLog.writelines(errorLines)
                os.remove(paths.tempLogPath)
            if self._args.exitOnFailure:
                self.exit(result.getResultCode().value)   
            return False