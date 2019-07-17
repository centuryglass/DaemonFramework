"""
Test that the daemon and parent can compile, install, and execute correctly.
"""

import sys, os
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import make, pathConstants, testActions, testArgs
from supportModules.testActions import TestResult
from abc import ABC, abstractmethod

"""
Attempts to build, install and run the daemon and parent with basic options.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
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
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('buildTest.py', \
                           'Test if KeyDaemon builds and runs with basic ' \
                           + 'valid arguments.')
    testActions.setup()
    runTest(args)
