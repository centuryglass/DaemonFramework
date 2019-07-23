"""
Tests that the daemon can correctly ensure only one instance is running.
"""

import sys, os, subprocess
moduleDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(moduleDir, os.pardir))
from supportModules import make, pathConstants, testObject, testArgs, \
                           testResult
from supportModules.testResult import InitCode, ExitCode, Result
from supportModules.pathConstants import paths
from supportModules.testObject import Test

"""
Tests that the daemon can correctly ensure only one instance is running.
Keyword Arguments:
testArgs -- A testArgs.Values argument object.
"""
def getTests(testArgs):
    title = 'Singular daemon process validation tests:'
    testCount = 6
    def testFunction(tests):
        """ 
        Test that daemon builds fail with pipes enabled and no lock file:
        Keyword Arguments:
        enableIn  -- Whether the daemon is built with the input pipe enabled.
        enableOut -- Whether the daemon is built with the output pipe enabled.
        """
        def testFailedBuild(enableIn, enableOut):
            inPath = paths.inPipePath if enableIn else None
            outPath = paths.outPipePath if enableOut else None
            buildArgs = make.getBuildArgs(testArgs = testArgs, \
                                          lockPath = None, \
                                          inPipePath = inPath, \
                                          outPipePath = outPath)
            resultCode = tests.daemonBuildInstall(buildArgs)
            description = 'Checking that builds fail with the ' \
                        + 'DF_LOCK_FILE_PATH undefined, input pipe ' \
                        + ('enabled' if enableIn else 'disabled') \
                        + ', output pipe ' \
                        + ('enabled.' if enableOut else 'disabled.')
            tests.checkResult(Result(resultCode, InitCode.daemonBuildFailure), \
                              description)
        testFailedBuild(True, False)
        testFailedBuild(False, True)
        testFailedBuild(True, True)

        # Build parent with standard options:
        buildArgs = make.getBuildArgs(testArgs = testArgs)
        parentBuildResult = tests.parentBuildInstall(buildArgs)

        """
        Test if the BasicParent runs as expected.
        Keyword Arguments:
        description    -- The description to print with the test results.
        buildResult    -- The result code obtained by running testBuild.
                          If this is a failed test result, the test execution
                          will be cancelled and the test will use this as its
                          result code.
        expectedResult -- The expected result of the test.<F2>
        """
        def testRun(description, buildResult, expectedResult):
            parentRunArgs = ['--timeout', '1']
            resultCode = buildResult
            if buildResult == InitCode.daemonInitSuccess:
                resultCode = tests.execTest(paths.parentSecureExePath, \
                                            parentRunArgs)
            tests.checkResult(Result(resultCode, expectedResult), description)

        # Build/install the daemon as long as the parent was built and 
        # installed correctly:
        daemonBuildResult = parentBuildResult
        if parentBuildResult == InitCode.parentInitSuccess:
            buildArgs = make.getBuildArgs(timeout = 0, \
                                          debugBuild = testArgs.debugBuild,\
                                          verbose = testArgs.useVerbose)
            daemonBuildResult = tests.daemonBuildInstall(buildArgs)

        # Test launching the background parent/daemon processes:
        bgProcess = None
        bgLaunchResult = daemonBuildResult
        if daemonBuildResult == InitCode.daemonInitSuccess:
            bgProcess = subprocess.Popen(paths.parentSecureExePath, \
                                         stdout = subprocess.DEVNULL)
            bgProcess.poll()
            if bgProcess.returncode is None:
                bgLaunchResult = InitCode.daemonRunSuccess
            elif bgProcess.returncode < 0:
                print('Background daemon process exited with signal ' \
                      + str(resultCode * -1))
            else:
                bgLaunchResult = ExitCode(bgProcess.returncode)
        description = 'Testing background daemon launch.'
        tests.checkResult(Result(bgLaunchResult, \
                          InitCode.daemonRunSuccess), description)

        # Test execution while the background processes are running:
        description = 'Testing execution with another instance already running.'
        expected = ExitCode.daemonAlreadyRunning
        testRun(description, daemonBuildResult, expected)

        # Test execution after killing background processes:
        if bgProcess is not None:
            bgProcess.kill()
            bgProcess.wait()
            if bgProcess.returncode is None:
                print('Error: Failed to kill background daemon process.')
        description = 'Testing execution after closing other daemon process.'
        testRun(description, daemonBuildResult, ExitCode.success)
    return Test(title, testFunction, testCount, testArgs)

# Run this file's tests alone if executing this module as a script:
if __name__ == '__main__':
    args = testArgs.read()
    if args.printHelp:
        testDefs.printHelp('singleRunningDaemon.py', \
                           "Test DaemonFramework's restriction on running " \
                           + "multiple daemon instances simultaneously.")
    getTests(args).runAll()
