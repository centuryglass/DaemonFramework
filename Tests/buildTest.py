"""
Tests that KeyDaemon can be successfully compiled, installed, and executed.
"""

from supportModules import testActions, testDefs
from supportModules.testActions import TestResult
from abc import ABC, abstractmethod

paths = testDefs.TestPaths()
makeVars = testDefs.MakeVars()

"""Attempts to build, install and run KeyDaemon with basic build options."""
def runTest():
    print('Running basic build/install/run test:')
    paths       = testDefs.TestPaths()
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    makeArgs    = testDefs.getValidTestMakeArgs()
    result = testActions.fullTest(makeArgs, installPath, parentPath, \
                                  outFile = None)
    testActions.checkResult(result, TestResult.success, '1/1', \
                            '', testFile = None)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    testActions.setup()
    runTest()
