"""
Tests that the KeyDaemon successfully reads and reports key codes.

This creates an instance of the KeyDaemon that listens to all valid keyboard
codes, and gives the tester a short period to press keys and see that the
KeyDaemon responds appropriately.
"""

from supportModules import testDefs, testActions, make
from supportModules.testActions import TestResult
from supportModules.testDefs import TestPaths, KeyCodes

""" 
Starts the key event reading test.
If no timeout period is provided, ten seconds is used by default.
Keyword Arguments:
testArgs -- A testActions.TestArgs argument object.
"""
def runTest(testArgs):
    print('Testing key event reading:')
    paths       = TestPaths()
    codes       = KeyCodes()
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    codeRange   = codes.highestValidCode - codes.lowestValidCode
    makeArgs    = make.getBuildArgs(keyLimit = codeRange, \
                                    debugBuild = testArgs.debugBuild, \
                                    verbose = testArgs.verbose, 
                                    timeout = 10)
    make.uninstall(makeArgs)
    result = testActions.testBuildInstall(makeArgs, installPath, \
                                      debugBuild = testArgs.debugBuild)
    resultWasExpected = testActions.checkResult(result, TestResult.success, \
                                                'keyReadingTest', \
                                                'Build/install KeyDaemon')
    if not resultWasExpected:
        print('Failed to build KeyDaemon for key event reading test.')
        return
    # Generate the list of all valid key codes:
    keyArgs = str(codes.lowestValidCode)
    for i in range(codes.lowestValidCode + 1, codes.highestValidCode):
        keyArgs = keyArgs + ' ' + str(i)
    print('Running key event reading test for ' \
            + str(testArgs.timeout if testArgs.timeout is not None else 10) \
            + ' seconds.')
    print('Please press any keyboard key, ' \
          + 'and verify that the code is registered.')
    result = testActions.runTest(installPath, parentPath, keyArgs, None)
    resultWasExpected = testActions.checkResult(result, TestResult.success, \
                                                'keyReadingTest', \
                                                'running KeyDaemon')
    if not resultWasExpected:
        print('Failed to run KeyDaemon for key event reading test.')


# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testActions.readArgs()
    if args.printHelp:
        testDefs.printHelp('keyReadingTest.py', \
                           'Test if KeyDaemon correctly detects key events.')
    testActions.setup()
    runTest(args)
