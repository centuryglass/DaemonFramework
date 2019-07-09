"""
Handles basic tasks needed to test compiling, installing, and running KeyDaemon.
"""
import os
import subprocess
from supportModules import make, pathConstants
from enum import Enum

paths = pathConstants.paths

"""Defines possible test results, from least to most successful.""" 
class TestResult(Enum):
    buildFailure = 0
    installError = 1
    runtimeError = 2
    success = 3

"""
Builds and installs the TestParent application, setting the KeyDaemon file path.
Keyword Arguments:
daemonPath  -- The path to the KeyDaemon executable launched by the TestParent.
               (default: The secure daemon test executable path.)
"""
def buildParent(daemonPath = paths.appSecureExePath):
    buildTestParent = False
    # Check if test parent is already built and has the right daemon path:
    if not os.path.isfile(paths.parentBuildPath):
        buildTestParent = True
    else:
        printPathArg = '-PrintDaemonPath'
        with open(paths.tempLogPath, 'w') as tempLog:
            subprocess.run([paths.parentBuildPath, printPathArg], \
                           stdout = tempLog)
        with open(paths.tempLogPath, 'r') as tempLog:
            currentPathDef = tempLog.readline()
            if (currentPathDef != daemonPath):
                buildTestParent = True
    # If test parent exists with the right path, only rebuild if source files
    # were updated:
    parentSrcFiles = [paths.parentSourcePath]
    for filename in os.listdir(paths.includeDir):
        if filename.endswith('.cpp'):
            parentSrcFiles.append(os.path.join(paths.includeDir, filename))
    if not buildTestParent:
        buildTime = os.path.getmtime(paths.parentBuildPath)
        for filename in parentSrcFiles:
            if os.path.getmtime(filename) > buildTime:
                buildTestParent = True
                break
    if buildTestParent:
        varNames = make.varNames
        buildArgs = ['g++', '-I' + paths.includeDir, '-pthread', '-g', '-ggdb',\
                     '-D' + varNames.installPath + '="' + daemonPath + '"', \
                     '-D' + varNames.pipePath + '="' + paths.keyPipePath + '"']\
                    + parentSrcFiles + ['-o', paths.parentBuildPath]
        subprocess.call(buildArgs)
        subprocess.call('cp ' + paths.parentBuildPath + ' ' \
                              + paths.parentUnsecureExePath, shell = True)
        subprocess.call('sudo cp ' + paths.parentBuildPath + ' ' \
                                   + paths.parentSecureExePath, shell = True)
    assert(os.path.isfile(paths.parentBuildPath))

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
    buildParent() # Rebuild test parent if necessary.
    os.chdir(paths.projectDir)


"""
Builds and install KeyDaemon, returning an appropriate ResultCode.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.
               These are required to ensure that KeyDaemon is removed from the
               correct installation path.

installPath -- The full path where the KeyDaemon executable will be installed.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.

debugBuild  -- Whether the Keydaemon builds in debug mode. (default: True)

Return TestResult.buildFailure if compilation fails, TestResult.installError
if installation fails, or TestResult.success if no errors occur.
"""
def testBuildInstall(makeArgs, installPath, outFile = subprocess.DEVNULL, \
                 debugBuild = True):
    # Make sure the TestParent sets the correct daemon path:
    buildParent(installPath)
    make.buildInstall(makeArgs, installPath, outFile, debugBuild)
    buildPath = paths.daemonDebugBuildPath if debugBuild \
                else paths.daemonReleaseBuildPath
    if not os.path.isfile(buildPath):
        return TestResult.buildFailure
    if not os.path.isfile(installPath):
        return TestResult.installError
    return TestResult.success

"""
Runs KeyDaemon, returning an appropriate ResultCode.

Keyword Arguments:
installPath -- The full path to the KeyDaemon executable file.

parentPath  -- The path to the executable that is allowed to launch KeyDaemon.
               If this is not a valid file, KeyDaemon will be launched directly.

keyArgs     -- The list tracked keycodes passed to the KeyDaemon on launch.
               The default value of 1 is just an arbitrary valid code.

outFile     -- The file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.

Return TestResult.runtimeError if the KeyDaemon aborted due to invalid input or
insufficient security, TestResult.success if the KeyDaemon ran successfully.
"""
def runTest(installPath, parentPath, keyArgs = '1', \
            outFile = subprocess.DEVNULL): 
    os.chdir(paths.projectDir)
    runTestArgs = []
    if os.path.isfile(parentPath):
        runTestArgs.append(parentPath)
    else:
        runTestArgs.append(installPath)
    runTestArgs.append(keyArgs)
    completedProcess = subprocess.run(runTestArgs, \
                                  stdout = outFile, \
                                  stderr = outFile)
    if completedProcess.returncode == 0:
        return TestResult.success
    return TestResult.runtimeError

"""
Attempts to uninstall, clean, build, install, and run KeyDaemon.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

installPath -- The full path to the KeyDaemon executable file.

parentPath  -- The path to the executable that is allowed to launch KeyDaemon.
               If this is not a valid file, KeyDaemon will be launched directly.

keyArgs:    -- The list of tracked keycodes to pass to the KeyDaemon on launch.
               The default value of 1 is just an arbitrary valid code.

outFile:   --  The file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.

debugBuild  -- Whether the Keydaemon builds in debug or release mode.
               (default: True)

Return a testDefs.TestResult result code describing whether or not the test
failed, and if so, what step it failed on.
"""
def fullTest(makeArgs, installPath, parentPath, keyArgs = '1', \
             outFile = subprocess.DEVNULL, debugBuild = True):
    os.chdir(paths.projectDir)
    make.uninstall(makeArgs, outFile)
    makeResult = testBuildInstall(makeArgs, installPath, outFile, debugBuild)
    if makeResult != TestResult.success:
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
def checkResult(result, expected, index, description, testFile = None):
    if testFile is not None:
        testFile.close()
    if result.value == expected.value:
        print(index + ' passed with expected result ' + expected.name)
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
