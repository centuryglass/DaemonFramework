"""
Handles basic tasks needed to test compiling, installing, and running KeyDaemon.
"""
import os.path
import subprocess
from supportModules import testDefs
from enum import Enum

paths = testDefs.TestPaths()

"""Defines possible test results, from least to most successful.""" 
class TestResult(Enum):
    buildFailure = 0
    installError = 1
    runtimeError = 2
    success = 3

"""Ensures the testing environment is properly set up."""
def setup():
    if (not os.path.isfile(paths.parentBuildPath)): # Ensure TestParent exists
        buildArgs = ['g++', paths.parentSourcePath, '-o', paths.parentBuildPath]
        subprocess.call(buildArgs)
    assert(os.path.isfile(paths.parentBuildPath))
    # Ensure unsecured test directory is initialized:
    if (not os.path.isdir(paths.unsecureExeDir)):
        os.mkdir(paths.unsecureExeDir)
        subprocess.call('chmod "o=w" ' + paths.unsecureExeDir, shell = True)
    if (not os.path.isdir(paths.parentUnsecureExePath)):
        subprocess.call('cp ' + paths.parentBuildPath + ' ' \
                              + paths.parentUnsecureExePath, shell = True)
    # Ensure secured test directory is initialized:
    if (not os.path.isdir(paths.secureExeDir)):
        os.mkdir(paths.secureExeDir)
        subprocess.call('sudo chown root.root ' + paths.secureExeDir, \
                        shell = True)
        subprocess.call('sudo chmod "o-w" ' + paths.secureExeDir, shell = True)
    if (not os.path.isdir(paths.parentSecureExePath)):
        subprocess.call('sudo cp ' + paths.parentBuildPath + ' ' \
                                   + paths.parentSecureExePath, shell = True)
        os.chdir(paths.projectDir)

"""
Uninstalls KeyDaemon and deletes all build files.

Keyword Arguments:
makeArgs -- The set of command line arguments to pass to the `make` process.
            These are required to ensure that KeyDaemon is removed from the
            correct installation path.

outFile  -- A file where test output from stdout and stderr will be sent.
            The default subprocess.DEVNULL value discards all output.
"""
def uninstall(makeArgs, outFile = subprocess.DEVNULL):
    os.chdir(paths.projectDir)
    subprocess.call(['make', 'uninstall'] + makeArgs, \
                    stdout = outFile, \
                    stderr = outFile)
    subprocess.call(['make', 'clean'], stdout = outFile, stderr = outFile)

"""
Builds and install KeyDaemon, returning an appropriate ResultCode.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.
               These are required to ensure that KeyDaemon is removed from the
               correct installation path.

installPath -- The full path where the KeyDaemon executable will be installed.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.

runtime     -- An optional duration in seconds before the KeyDaemon closes.
               Default runtime is one second.

Return TestResult.buildFailure if compilation fails, TestResult.installError
if installation fails, or TestResult.success if no errors occur.
"""
def buildInstall(makeArgs, installPath, outFile = subprocess.DEVNULL, \
                 runtime = 1):
    os.chdir(paths.projectDir)
    # Try to build:
    subprocess.call(['make', 'TIMEOUT=' + str(runtime)] + makeArgs, \
                    stdout = outFile, \
                    stderr = outFile)
    if (not os.path.isfile(paths.appBuildPath)):
        return TestResult.buildFailure
    # Try to install:
    subprocess.call(['make', 'install'] + makeArgs, \
                    stdout = outFile, \
                    stderr = outFile)
    if (not os.path.isfile(installPath)):
        return TestResult.installError
    return TestResult.success

"""
Runs KeyDaemon, returning an appropriate ResultCode.

Keyword Arguments:
installPath -- The full path to the KeyDaemon executable file.

parentPath  -- The path to the executable that is allowed to launch KeyDaemon.
               If this is not a valid file, KeyDaemon will be launched directly.

keyArgs     -- The list tracked keycodes passed to the KeyDaemon on launch.
               The default value of "1" is just an arbitrary valid code.

outFile     -- The file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.

Return TestResult.runtimeError if the KeyDaemon aborted due to invalid input or
insufficient security, TestResult.success if the KeyDaemon ran successfully.
"""
def runTest(installPath, parentPath, keyArgs = '"1"', \
            outFile = subprocess.DEVNULL): 
    os.chdir(paths.projectDir)
    runTestArgs = []
    if (os.path.isfile(parentPath)):
        runTestArgs.append(parentPath)
    runTestArgs.append(installPath)
    runTestArgs.append(keyArgs)
    completedProcess = subprocess.run(runTestArgs, \
                                  stdout = outFile, \
                                  stderr = outFile)
    if (completedProcess.returncode == 0):
        return TestResult.success
    return TestResult.runtimeError

"""
Attempts to uninstall, clean, build, install, and run KeyDaemon.

Keyword Arguments:
parentPath -- The path to the executable that is allowed to launch KeyDaemon.
              If this is not a valid file, KeyDaemon will be launched directly.

keyArgs:   -- The list of tracked keycodes to pass to the KeyDaemon on launch.
              The default value of "1" is just an arbitrary valid code.

outFile:  --  The file where test output from stdout and stderr will be sent.
             The default subprocess.DEVNULL value discards all output.

Return a testDefs.TestResult result code describing whether or not the test
failed, and if so, what step it failed on.
"""
def fullTest(makeArgs, installPath, parentPath, keyArgs = '"1"', \
             outFile = subprocess.DEVNULL):
    os.chdir(paths.projectDir)
    uninstall(makeArgs, outFile)
    makeResult = buildInstall(makeArgs, installPath, outFile)
    if (makeResult != TestResult.success):
        return makeResult
    return runTest(installPath, parentPath, keyArgs, outFile)

"""
Check on the result of a test, reporting whether it succeeded or failed.
This also closes and deletes the test output file, copying its text to the
failure log file first if appropriate.

Keyword Arguments:
result      -- The TestResult code describing the test's outcome.
expected    -- The TestResult code describing the test's expected outcome.
index       -- An index string describing the test.
description -- A longer test description to print if the test fails.
testFile    -- A file object storing test output from stdout and stderr.

Returns true if the test result was as expected, false otherwise.
"""
def checkResult(result, expected, index, description, testFile):
    if (testFile != None):
        testFile.close()
    if (result.value == expected.value):
        print(index + ' passed with expected result ' + expected.name)
        os.remove(paths.tempLogPath)
        return True
    else:
        print(index + ' failed!')
        print('Expected result: ' + expected.name \
              + ' actual result: ' + result.name)
        print('Test description: ' + description)
        print('See ' + paths.failureLog + ' for more information.')
        if (testFile != None):
            with open(paths.tempLogPath, 'r') as tempLog:
                with open(paths.failureLogPath, 'a') as failureLog:
                    failureLog.write(index + ' ' + description + ':\n')
                    failureLog.writelines(tempLog.readlines())
        os.remove(paths.tempLogPath)
        return False
