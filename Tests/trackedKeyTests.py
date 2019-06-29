"""
Tests how KeyDaemon handles different valid and invalid key code arguments.
"""

from supportModules import testDefs, testActions
from supportModules.testActions import TestResult
from supportModules.testDefs import MakeVars, TestPaths

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

#### Prepare key code argument tests:
argTestKeyLimit = 5
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
        KeycodeArgTest('237 238 239', \
                       'upper valid keycode range', \
                       TestResult.success), \
        KeycodeArgTest('1 2 3', \
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
        KeycodeArgTest('240', \
                       'value outside the valid keycode range', \
                       TestResult.runtimeError), \
        KeycodeArgTest('9223372036854775808', \
                       'value outside of numeric limits', \
                       TestResult.runtimeError), \
        KeycodeArgTest('1 2 3 4 5 6', \
                       'exceeding tracked key code count limits', \
                       TestResult.runtimeError), \
        ]

""" Run all key code argument tests. """
def runTests():
    print('Running tracked keycode argument tests:')
    testCount   = len(keyArgTests)
    testsPassed = 0
    paths       = TestPaths()
    makeVarDefs = MakeVars()
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    makeArgs    = [ makeVarDefs.installPath + '=' + installPath, \
                    makeVarDefs.parentPath  + '=' + parentPath, \
                    makeVarDefs.keyLimit    + '=' + str(argTestKeyLimit)]
    testActions.uninstall(makeArgs)
    testActions.buildInstall(makeArgs, installPath)
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
        if (resultMatched):
            testsPassed += 1
    print('Passed ' + str(testsPassed) + '/' + str(testCount) \
            + ' tracked keycode argument tests.\n')
