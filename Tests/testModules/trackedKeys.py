"""
Tests how KeyDaemon handles different valid and invalid key code arguments.
"""

import sys, os
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import keyCodes, make, pathConstants, testActions, testArgs
from supportModules.testActions import TestResult

"""
Holds a key code argument string and the expected result of testing that string.

Key code argument strings are passed to the KeyDaemon application to set which
keys it should track. These tests verify that valid code argument sets are
recognized as valid, and invalid code argument sets are rejected.
"""
class KeycodeArgTest:
    """
    Constructs a new keycode test object.

    Keyword Arguments:
    argString      -- The keycode argument string passed to KeyDaemon on launch.
    description    -- A brief description of this test's intent.
    expectedResult -- The TestResult expected when using these arguments.
    """
    def __init__(self, argString, description, expectedResult):
        self._argString = argString
        self._description = description
        self._expectedResult = expectedResult
    """Return the code argument string used when running this test."""
    @property
    def argString(self):
        return self._argString
    """Return the description of this test."""
    @property
    def description(self):
        return self._description
    """Return the expected result of this test."""
    @property
    def expectedResult(self):
        return self._expectedResult
        
"""
Returns a string containing three sequential numbers, separated by spaces.
Keyword Arguments:
firstNumber -- The first number in the sequence.
"""
def numberStringSequence(firstNumber):
    return str(firstNumber) + ' ' + str(firstNumber + 1) + ' ' \
         + str(firstNumber + 2)

#### Prepare key code argument tests:
maxKeyCodeLimit = 5
codes = keyCodes.constants
maxCode = codes.highestValidCode
minCode = codes.lowestValidCode
keyArgTests = [ \
        KeycodeArgTest('3', \
                       'minimum valid key count', \
                       TestResult.success), \
        KeycodeArgTest('22 55 220', \
                       'median valid key count', \
                       TestResult.success), \
        KeycodeArgTest('44 66 88 100 101', \
                       'maximum valid key count', \
                       TestResult.success), \
        KeycodeArgTest('            4   5', \
                       'ignoring whitespace', \
                       TestResult.success), \
        KeycodeArgTest(numberStringSequence(maxCode - 2), \
                       'upper valid keycode range', \
                       TestResult.success), \
        KeycodeArgTest(numberStringSequence(minCode), \
                       'lower valid keycode range', \
                       TestResult.success), \
        KeycodeArgTest('', \
                       'no arguments', \
                       TestResult.runtimeError), \
        KeycodeArgTest('""', \
                       'empty arguments', \
                       TestResult.runtimeError), \
        KeycodeArgTest('non-numeric', \
                       'entirely non-numeric arguments', \
                       TestResult.runtimeError), \
        KeycodeArgTest('3 5 9!', \
                       'bad characters within otherwise valid arguments', \
                       TestResult.runtimeError), \
        KeycodeArgTest('-2 -4 -6', \
                       'negative key codes', \
                       TestResult.runtimeError), \
        KeycodeArgTest(str(maxCode + 1), \
                       'value outside the valid keycode range', \
                       TestResult.runtimeError), \
        KeycodeArgTest('9223372036854775808', \
                       'value outside of numeric limits', \
                       TestResult.runtimeError), \
        KeycodeArgTest('1 2 3 4 5 6', \
                       'exceeding tracked key code count limits', \
                       TestResult.runtimeError), \
        ]

"""
Run all tracked key code argument tests. 
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def runTests(testArgs):
    print('Running tracked keycode argument tests:')
    testCount   = len(keyArgTests)
    testsPassed = 0
    paths       = pathConstants.paths
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    makeArgs    = make.getBuildArgs(keyLimit = maxKeyCodeLimit, \
                                    testArgs = testArgs)
    make.uninstall(makeArgs)
    make.buildInstall(makeArgs, installPath)
    for index, argTest in enumerate(keyArgTests):
        logFile = open(paths.tempLogPath, 'w')
        indexString = 'Test ' + str(index + 1) + '/' + str(testCount)
        result = testActions.runTest(installPath, parentPath, \
                                     argTest.argString, logFile)
        resultMatched = testActions.checkResult(result, \
                                                argTest.expectedResult,\
                                                indexString, \
                                                argTest.description, \
                                                logFile)
        if resultMatched:
            testsPassed += 1
    print('Passed ' + str(testsPassed) + '/' + str(testCount) \
            + ' tracked keycode argument tests.\n')

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('trackedKeyTests.py', \
                           'Test different types of valid and invalid tracked '\
                           + 'key code KeyDaemon argument lists.')
    testActions.setup()
    runTests(args)
