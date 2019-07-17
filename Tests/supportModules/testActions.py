"""
Handles basic tasks needed to test compiling, installing, and running KeyDaemon.
"""
import os
import subprocess
import tempfile
import time
from supportModules import make, pathConstants, testResult
from enum import Enum

paths = pathConstants.paths

"""Ensures the testing environment is properly set up."""
def setup():
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
        subprocess.call('sudo chmod "o-w" ' + paths.secureExeDir, shell = True)
    os.chdir(paths.projectDir)


"""
Builds and install the daemon, returning an appropriate testResult.FailureCode 
if any step fails.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def testDaemonBuildInstall(makeArgs, installPath, outFile = subprocess.DEVNULL):
    if not make.buildBasicDaemon(makeArgs, outFile):
        return FailureCode.daemonBuildFailure
    if not make.installBasicDaemon(makeArgs, outFile):
        return FailureCode.daemonInstallFailure

"""
Builds and install the parent, returning an appropriate testResult.FailureCode 
if any step fails.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def testParentBuildInstall(makeArgs, outFile = subprocess.DEVNULL):
    if not make.buildBasicParent(makeArgs, outFile):
        return FailureCode.parentBuildFailure
    if not make.installBasicParent(makeArgs, outFile):
        return FailureCode.parentInstallFailure

"""
Runs BasicParent, returning an appropriate testResult.Result.

Keyword Arguments:
daemonPath      -- The full path to the daemon executable file.

parentPath      -- The path to the executable that is allowed to launch the
                   daemon. If this is not a valid file, BasicDaemon will be
                   launched directly.

expectedOutcome -- The testResult.ExitCode or testResult.FailureCode that the
                   test is expected to return.

argList         -- An optional list of arguments to pass to the BasicParent.

outFile         -- The file where test output from stdout and stderr will be
                   sent. The default subprocess.DEVNULL value discards all
                   output.
"""

def runTest(installPath, parentPath, expectedOutcome, argList = [], \
            outFile = subprocess.DEVNULL): 
    os.chdir(paths.projectDir)
    runTestArgs = []
    if os.path.isfile(parentPath):
        runTestArgs.append(parentPath)
    else:
        runTestArgs.append(installPath)
    runTestArgs.append(argList)
    completedProcess = subprocess.run(runTestArgs, \
                                  stdout = outFile, \
                                  stderr = outFile)
    return testResult.Test(testResult.ExitCode(completedProcess.returncode), \
                           expectedOutcome)

"""
Attempts to uninstall, clean, build, install, and test the parent and daemon.

Keyword Arguments:
makeArgs        -- The set of command line arguments to pass to the `make`
                    process.

daemonPath      -- The full path to the daemon executable file.

parentPath      -- The path to the executable that is allowed to launch the
                   daemon. If invalid, the daemon will be launched directly.

expectedOutcome -- The testResult.ExitCode or testResult.FailureCode that the
                   test is expected to return.

argList         -- An optional list of arguments to pass to the BasicParent.

outFile:       --  The file where test output from stdout and stderr will be
                   sent. The default subprocess.DEVNULL value discards all
                   output.

Return a testResult.Result object describing the test's actual and expected
results.
"""
def fullTest(makeArgs, daemonPath, parentPath, expectedOutcome, argList = [] \
             outFile = subprocess.DEVNULL):
    if parentPath is not None:
        buildResult = testParentBuildInstall(makeArgs, outFile)
        if buildResult is not None:
            return testResult.Result(buildResult, expectedOutcome)
    buildResult = testDaemonBuildInstall(makeArgs, outFile)
    if buildResult is not None:
        return testResult.Result(buildResult, expectedOutcome)
    runResult = runTest(daemonPath, parentPath, expectedOutcome, argList, \
                        outFile)
    if runResult is not None:
        return testResult.Result(runResult, expectedOutcome)
    return testResult.Result(testResult.ExitCode.success, expectedOutcome)

"""
Check on the result of a test, reporting whether it succeeded or failed.
This also closes and deletes the test output file, copying its text to the
failure log file first if appropriate.

Keyword Arguments:
result      -- The testResult.Result object describing the test's outcome.
index       -- An index string describing the test.
description -- A longer test description to print if the test fails.
testFile    -- A file object storing test output from stdout and stderr.

Returns true if the test result was as expected, false otherwise.
"""
def checkResult(result, expected, index, description, testFile = None):
    if testFile is not None:
        testFile.close()
    result.printResultText()
    if result.testPassed():
        if (os.path.isfile(paths.tempLogPath)):
            os.remove(paths.tempLogPath)
        return True
    else:
        print(index + ' failed!')
        failureDescription = 'Expected result: ' + expected.name \
                           + ', actual result: ' + result.name
        print(failureDescription)
        print('Test description: ' + description)
        if testFile is not None:
            print('See ' + paths.failureLog + ' for more information.')
            with open(paths.tempLogPath, 'r') as tempLog:
                with open(paths.failureLogPath, 'a') as failureLog:
                    failureLog.write('\n' + index + ' ' + description + '\n')
                    failureLog.write('Error: ' + failureDescription + '\n')
                    failureLog.write('Test output:\n')
                    errorLines = tempLog.readlines()
                    errorLines = [ '\t' + line for line in errorLines]
                    failureLog.writelines(errorLines)
        if os.path.isfile(paths.tempLogPath):
            os.remove(paths.tempLogPath)
        return False
