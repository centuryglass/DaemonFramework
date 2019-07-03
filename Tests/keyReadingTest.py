"""
Tests that the KeyDaemon successfully reads and reports key codes.

This creates an instance of the KeyDaemon that listens to all valid keyboard
codes, and gives the tester a short period to press keys and see that the
KeyDaemon responds appropriately.
"""

from supportModules import testDefs, testActions
from supportModules.testActions import TestResult
from supportModules.testDefs import TestPaths, KeyCodes

""" 
Starts the key event reading test.
Keyword Arguments:
timeout -- Number of seconds to listen for input events. (default: 10)
"""
def runTest(timeout = 10):
    print('Testing key event reading:')
    paths       = TestPaths()
    codes       = KeyCodes()
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    codeRange   = codes.highestValidCode - codes.lowestValidCode
    makeArgs = testDefs.getValidTestMakeArgs(codeRange)
    testActions.uninstall(makeArgs)
    result = testActions.buildInstall(makeArgs, installPath, runtime = timeout)
    if (not testActions.checkResult(result, TestResult.success, \
                                    'keyReadingTest', \
                                    'Build/install KeyDaemon')):
        return;
    # Generate the list of all valid key codes:
    keyArgs = str(codes.lowestValidCode)
    for i in range(codes.lowestValidCode + 1, codes.highestValidCode):
        keyArgs = keyArgs + ' ' + str(i)
    print('Running key event reading test for ' + str(timeout) + ' seconds.')
    print('Please press any keyboard key, ' \
          + 'and verify that the code is registered.')
    result = testActions.runTest(installPath, parentPath, keyArgs, None)
    if (not testActions.checkResult(result, TestResult.success, \
                                    'keyReadingTest', \
                                    'running KeyDaemon')):
        print('Failed to run KeyDaemon for key event reading test.')


# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    import sys
    testActions.setup()
    if (len(sys.argv) > 1):
        runTest(sys.argv[1])
    else:
        runTest()
