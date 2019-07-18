"""
Tests that the daemon correctly handles parent executable path verification.
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
Tests that the parent path verification security option functions appropriately.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def getTests(testArgs):
    title = 'Parent path validation tests:'
    testCount = 8
    def testFunction(tests):
        parentPath = paths.parentSecureExePath
        altParentPath = os.path.join(paths.basicParentDir, paths.parent)
        buildArgs = make.getBuildArgs(testArgs = testArgs)
        result = tests.parentBuildInstall(buildArgs)
        builtParentCorrectly = result == InitCode.parentInitSuccess
        testIndex = 0
        while testIndex < testCount:
            runExpected = (testIndex % 2) == 0
            requireExpected = (testIndex // 2 % 2) == 0
            requireSecure = (testIndex // 4 % 2 ) == 0
            testIndex += 1
            execPath = parentPath if runExpected else altParentPath
            description = ('Correct' if runExpected else 'Incorrect') \
                          + ' path, path checks ' \
                          + ('enabled,' if requireExpected else 'disabled,') \
                          + ' path security checks ' \
                          + ('enabled.' if requireSecure else 'disabled.')
            expectedResult = ExitCode.success
            if requireExpected and not runExpected:
                expectedResult = ExitCode.badParentPath
            elif requireSecure and not runExpected:
                expectedResult = ExitCode.insecureParentDir
            if builtParentCorrectly:
                pathTarget = parentPath if requireExpected else None
                daemonArgs = make.getBuildArgs(parentPath = pathTarget, \
                                               secureParent = requireSecure, \
                                               testArgs = testArgs)
                result = tests.daemonBuildInstall(daemonArgs)
                if result == InitCode.daemonInitSuccess:
                   result = tests.execTest(execPath)
            tests.checkResult(Result(result, expectedResult), description)
    return Test(title, testFunction, testCount, testArgs)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('parentPathChecking.py', \
                           "Test DaemonFramework's parent path validation.")
    getTests(args).runAll()
