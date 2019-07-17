"""
Tests that the daemon correctly handles daemon executable path verification.
"""

import sys, os
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import make, pathConstants, testActions, testArgs, \
                           testResult
from supportModules.testResult import InitCode, ExitCode, Result
from supportModules.testActions import testDaemonBuildInstall, \
                                       testParentBuildInstall

"""
Tests that the path verification security option functions appropriately.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def runTest(testArgs):
    print('Testing daemon path validation:')
    paths       = pathConstants.paths
    parentPath  = paths.parentSecureExePath
    daemonPath  = paths.daemonSecureExePath
    altDaemonPath = os.path.join(paths.basicDaemonDir, paths.daemon)
    logFile = lambda : open(paths.tempLogPath, 'a')
    runExpectedPath = [False, True]
    requireExpectedPath = [True, False]
    numTests = 4
    testIndex = 0
    passedTests = 0

    def checkResult(resultCode, expectedCode, description):
        nonlocal testIndex
        testIndex += 1
        index = str(testIndex) + '/' + str(numTests) + ' '
        result = testResult.Result(resultCode, expectedCode)
        return testActions.checkResult(result, index, description)

    for runExpected in runExpectedPath:
        for requireExpected in requireExpectedPath:
            description = ('Correct' if runExpected else 'Incorrect') \
                          + ' path, path checking ' \
                          + ('enabled.' if requireExpected else 'disabled.')
            parentArgs = None
            if runExpected:
                parentArgs = make.getBuildArgs(testArgs = testArgs)
            else:
                parentArgs = make.getBuildArgs(daemonPath = altDaemonPath, \
                                               testArgs = testArgs)
            result = testParentBuildInstall(parentArgs, logFile())
            if result == InitCode.parentInitSuccess:
                daemonArgs = None
                daemonArgs = make.getBuildArgs(checkPath = requireExpected, \
                                               testArgs = testArgs)
                result = testDaemonBuildInstall(daemonArgs, logFile())
                if result == InitCode.daemonInitSuccess:
                    result = testActions.runTest(daemonPath, parentPath, \
                                                 outFile = logFile())
            expectedResult = ExitCode.success
            if requireExpected and not runExpected:
                expectedResult = ExitCode.badDaemonPath
            if checkResult(result, expectedResult, description):
                passedTests += 1
    print(str(passedTests) + ' of ' + str(numTests) + ' tests passed.')

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('buildTest.py', \
                           "Test if DaemonFramework's test parent and daemon " \
                           + 'build and run with basic valid arguments.')
    testActions.setup()
    runTest(args)
