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
Tests that the daemon path verification security option functions appropriately.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def getTests(testArgs):
    title = 'Daemon path validation tests:'
    testCount = 8
    def testFunction(tests):
        daemonPath = paths.daemonSecureExePath
        altDaemonPath = paths.daemonBuildPath
        testIndex = 0
        while testIndex < testCount:
            runExpected = (testIndex % 2) == 0
            requireExpected = (testIndex // 2 % 2) == 0
            requireSecure = (testIndex // 4 % 2 ) == 0
            testIndex += 1
            description = ('Correct' if runExpected else 'Incorrect') \
                           + ' path, path checking ' \
                           + ('enabled,' if requireExpected else 'disabled,') \
                           + ' path security checks ' \
                           + ('enabled.' if requireSecure else 'disabled.')
            targetPath = daemonPath if runExpected else altDaemonPath
            parentArgs = make.getBuildArgs(testArgs = testArgs, \
                                           securePath = requireSecure, \
                                           daemonPath = targetPath)
            result = tests.parentBuildInstall(parentArgs)
            expectedResult = ExitCode.success
            if requireExpected and not runExpected:
                expectedResult = ExitCode.badDaemonPath
            elif requireSecure and not runExpected:
                expectedResult = ExitCode.insecureDaemonDir
            if result == InitCode.parentInitSuccess:
                daemonArgs = make.getBuildArgs(checkPath = requireExpected, \
                                               securePath = requireSecure, \
                                               testArgs = testArgs)
                result = tests.daemonBuildInstall(daemonArgs)
                if result == InitCode.daemonInitSuccess:
                        result = tests.execTest(paths.parentSecureExePath)
            tests.checkResult(Result(result, expectedResult), description)
    return Test(title, testFunction, testCount, testArgs)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('daemonPathChecking.py', \
                           "Test DaemonFramework's daemon path validation.")
    getTests(args).runAll()
