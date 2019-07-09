"""
Tests that KeyDaemon can be successfully compiled, installed, and executed.
"""

from supportModules import testActions, testDefs, make, pathConstants
from supportModules.testActions import TestResult
from abc import ABC, abstractmethod

"""
Attempts to build, install and run KeyDaemon with basic build options.
Keyword Arguments:
testArgs -- A testActions.TestArgs argument object.
"""
def runTest(testArgs):
    print('Running basic build/install/run test:')
    paths       = pathConstants.paths
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    makeArgs    = make.getBuildArgs(testArgs = testArgs)
    result = testActions.fullTest(makeArgs, installPath, parentPath, \
                                  outFile = None, \
                                  debugBuild = testArgs.debugBuild)
    testActions.checkResult(result, TestResult.success, '1/1', \
                            '', testFile = None)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testActions.readArgs()
    if args.printHelp:
        testDefs.printHelp('buildTest.py', \
                           'Test if KeyDaemon builds and runs with basic ' \
                           + 'valid arguments.')
    testActions.setup()
    runTest(args)
