"""
Test that the daemon and parent can compile, install, and execute correctly.
"""

import sys, os
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import make, pathConstants, testActions, testArgs, \
                           testResult

"""
Attempts to build, install and run the daemon and parent with basic options.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def runTest(testArgs):
    print('Running basic build/install/run test:')
    paths       = pathConstants.paths
    daemonPath  = paths.daemonSecureExePath
    parentPath  = paths.parentSecureExePath
    makeArgs    = make.getBuildArgs(testArgs = testArgs)
    logFile     = open(paths.tempLogPath, 'w')
    result = testActions.fullTest(makeArgs, daemonPath, parentPath, \
                                  outFile = logFile)
    testActions.checkResult(result, '1/1', 'Basic build/install/run test', \
            testFile = None)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('buildTest.py', \
                           "Test if DaemonFramework's test parent and daemon " \
                           + 'build and run with basic valid arguments.')
    testActions.setup()
    runTest(args)
