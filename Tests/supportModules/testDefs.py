"""testDefs defines constant values used when testing the KeyDaemon project."""
import os

"""Defines path and filename constants used in testing."""
class TestPaths:
    def __init__(self):
        self._testedApp  = 'KeyDaemon'
        self._parentApp  = 'TestParent'
        self._tempLog    = 'tempLog.txt'
        self._failureLog = 'failureLog.txt'
        self._pipeFile   = '.keyPipe'
        self._moduleDir  = os.path.dirname(os.path.realpath(__file__))
        self._testDir    = os.path.normpath(os.path.join(self._moduleDir, \
                                                         os.pardir))
        self._projectDir = os.path.normpath(os.path.join(self._testDir, \
                                                         os.pardir))
        self._includeDir     = os.path.join(self.projectDir, 'Parent_Include')
        self._testExecDir    = os.path.join(self.testDir, 'testExecutables')
        self._secureExeDir   = os.path.join(self.testExecDir, 'secured')
        self._unsecureExeDir = os.path.join(self.testExecDir, 'unsecured')
        self._debugBuildDir = os.path.join(self._projectDir, 'build', 'Debug')
        self._releaseBuildDir = os.path.join(self._projectDir, 'build', \
                                                               'Release')

    # Directory paths:
    """Return the path to the main project directory. """
    @property
    def projectDir(self):
        return self._projectDir
    """Return the path to the project's main test directory."""
    @property
    def testDir(self):
        return self._testDir
    """Return the path to the project's Debug build directory."""
    @property
    def debugBuildDir(self):
        return self._debugBuildDir
    """Return the path to the project's Release build directory."""
    @property
    def releaseBuildDir(self):
        return self._releaseBuildDir
    """Return the path to the parent application #include directory."""
    @property
    def includeDir(self):
        return self._includeDir
    """Return the path to the main test executable directory."""
    @property
    def testExecDir(self):
        return self._testExecDir
    """Return the path to the secured executable test directory."""
    @property
    def secureExeDir(self):
        return self._secureExeDir
    """Return the path to the unsecured executable test directory."""
    @property
    def unsecureExeDir(self):
        return self._unsecureExeDir

    # File names:
    """Return the name of the tested application file."""
    @property
    def testedApp(self):
        return self._testedApp
    """Return the name of the parent app used to launch the KeyDaemon."""
    @property
    def parentApp(self):
        return self._parentApp
    """Return the name of the temporary test output log file."""
    @property
    def tempLog(self):
        return self._tempLog
    """Return the name of the test failure log file."""
    @property
    def failureLog(self):
        return self._failureLog
    """Return the name of the pipe file used to share keyboard events."""
    @property
    def pipeFile(self):
        return self._pipeFile

    # KeyDaemon executable paths:
    """Return the path where the KeyDaemon is found after Debug compilation."""
    @property
    def daemonDebugBuildPath(self):
        return os.path.join(self.debugBuildDir, self.testedApp)
    """
    Return the path where the KeyDaemon is found after Release compilation.
    """
    @property
    def daemonReleaseBuildPath(self):
        return os.path.join(self.releaseBuildDir, self.testedApp)
    """Return the path to the KeyDaemon in the secured executable directory."""
    @property
    def appSecureExePath(self):
        return os.path.join(self.secureExeDir, self.testedApp)
    """
    Return the path to the tested app in the unsecured executable directory.
    """
    @property
    def appUnsecureExePath(self):
        return os.path.join(self.unsecureExeDir, self.testedApp)

    # Parent application paths:
    """Return the path where parent app is found when it is built."""
    @property
    def parentBuildPath(self):
        return os.path.join(self.testExecDir, self.parentApp)
    """Return the path to the parent app's source file."""
    @property
    def parentSourcePath(self):
        return self.parentBuildPath + '.cpp'
    """Return the path to the parent app in the secured executable directory."""
    @property
    def parentSecureExePath(self):
        return os.path.join(self.secureExeDir, self.parentApp)
    """
    Return the path to the parent app in the unsecured executable directory.
    """
    @property
    def parentUnsecureExePath(self):
        return os.path.join(self.unsecureExeDir, self.parentApp)
    """Return the path where the daemon's pipe file will be created."""
    @property
    def keyPipePath(self):
        return os.path.join(self.testExecDir, self.pipeFile)
    """Return the path where temporary log files will be stored."""
    @property
    def tempLogPath(self):
        return os.path.join(self.testDir, self._tempLog)
    """Return the path where the failure log file will be saved."""
    @property
    def failureLogPath(self):
        return os.path.join(self.testDir, self._failureLog)
paths = TestPaths()


"""
Prints help text describing the purpose of a test and all available command
line arguments, then stops the script.

Keyword Arguments:
testName        -- The title of the main test being run.
testDescription -- A brief description of the main test being run.
"""
def printHelp(testName, testDescription):
    if isinstance(testName, str) and len(testName) > 0:
        print(testName + ':')
    if isinstance(testDescription, str) and len(testDescription) > 0:
        print(testDescription)
    print('Command line options:')
    print('\t-v:          Use verbose build/test logging.')
    print('\t-r:          Build in Release mode instead of Debug.')
    print('\t-t=[number]: Seconds to run the KeyDaemon before exiting.')
    print('\t-h, --help:  Print this help text and exit.')
    import sys
    sys.exit('')
