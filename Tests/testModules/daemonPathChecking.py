"""
Tests that the daemon correctly handles daemon executable path verification.
"""

import sys, os
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import make, pathConstants, testObject, testArgs, \
                           testResult
from supportModules.testResult import InitCode, ExitCode, Result
from supportModules.pathConstants import paths
from supportModules.testObject import Test

"""
Tests that the path verification security option functions appropriately.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def getTests(testArgs):
    title = 'Daemon path validation tests:'
    def testFunction(tests):
        daemonPath = paths.daemonSecureExePath
        altDaemonPath = os.path.join(paths.basicDaemonDir, paths.daemon)
        runExpectedPath = [False, True]
        requireExpectedPath = [True, False]
        for runExpected in runExpectedPath:
            for requireExpected in requireExpectedPath:
                description = ('Correct' if runExpected else 'Incorrect') \
                              + ' path, path checking ' \
                              + ('enabled.' if requireExpected else 'disabled.')
                targetPath = daemonPath if runExpected else altDaemonPath
                parentArgs = make.getBuildArgs(testArgs = testArgs, \
                                               daemonPath = targetPath)
                result = tests.parentBuildInstall(parentArgs)
                if result == InitCode.parentInitSuccess:
                    daemonArgs = make.getBuildArgs(checkPath = requireExpected,\
                                                   testArgs = testArgs)
                    result = tests.daemonBuildInstall(daemonArgs)
                    if result == InitCode.daemonInitSuccess:
                        result = tests.execTest(paths.parentSecureExePath)
                expectedResult = ExitCode.success
                if requireExpected and not runExpected:
                    expectedResult = ExitCode.badDaemonPath
                tests.checkResult(Result(result, expectedResult), description)
    testCount = 4
    return Test(title, testFunction, testCount, testArgs)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('buildTest.py', \
                           "Test if DaemonFramework's test parent and daemon " \
                           + 'build and run with basic valid arguments.')
    getTests(args).runAll()
