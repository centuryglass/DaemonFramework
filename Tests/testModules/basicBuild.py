"""
Test that the daemon and parent can compile, install, and execute correctly.
"""

import sys, os
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import make, testArgs, pathConstants, testObject
from supportModules.pathConstants import paths
from supportModules.testObject import Test

"""
Creates a Test that builds and runs the parent and daemon with default options.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def getTests(testArgs):
    title = 'Basic build/install/run test:'
    def testFunction(testObject):
        parentPath  = paths.parentSecureExePath
        makeArgs    = make.getBuildArgs(testArgs = testArgs)
        result      = testObject.fullTest(makeArgs, parentPath)
        testObject.checkResult(result, 'Basic build/install/run test')
    testCount = 1
    return Test(title, testFunction, testCount, testArgs)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('basicBuild.py', \
                           "Test if DaemonFramework's test parent and daemon " \
                           + 'build and run with basic valid arguments.')
    basicTests = getTests(args).runAll()
