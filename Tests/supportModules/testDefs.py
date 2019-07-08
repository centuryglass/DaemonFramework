"""testDefs defines constant values used when testing the KeyDaemon project."""
import os

"""Defines all relevant makefile variable names."""
class MakeVars:
    def __init__(self):
        self._installPath = 'INSTALL_PATH'
        self._parentPath  = 'PARENT_PATH'
        self._pipePath    = 'KEY_PIPE_PATH'
        self._keyLimit    = 'KEY_LIMIT'
    """Return the KeyDaemon install path variable name."""
    @property
    def installPath(self):
        return self._installPath
    """
    Return the KeyDaemon parent process path variable name.
    KeyDaemon will only run if launched by an executable stored at this path.
    """
    @property
    def parentPath(self):
        return self._parentPath
    """Return the KeyDaemon pipe file path variable name."""
    @property
    def pipePath(self):
        return self._pipePath
    """
    Return the maximum tracked key codes variable name.
    KeyDaemon will only track a limited number of key codes, not exceeding this
    value.
    """
    @property
    def keyLimit(self):
        return self._keyLimit

"""Defines Linux key code values."""
class KeyCodes:
    def __init__(self):
        self._lowestValidCode = 1
        self._highestValidCode = 239
    """Return the lowest valid keyboard event code."""
    @property
    def lowestValidCode(self):
        return self._lowestValidCode
    """Return the highest valid keyboard event code."""
    @property
    def highestValidCode(self):
        return self._highestValidCode

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
        self._buildDir = os.path.join(self._projectDir, 'build', 'Debug')

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
    def buildDir(self):
        return self._buildDir
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
    """Return the path where the KeyDaemon is found after compilation."""
    @property
    def appBuildPath(self):
        return os.path.join(self.buildDir, self.testedApp)
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

"""
Gets a basic set of build arguments that should always work correctly.

These options use the secure executable directory for installation. To execute
the KeyDaemon without runtime errors, it needs to be launched through an
instance of the TestParent executable located in the same directory.
    
Keyword Arguments:
keyLimit -- Optionally sets a specific tracked key code limit. (default: 1)
"""
def getValidTestMakeArgs(keyLimit = 1):
    paths       = TestPaths()
    makeVarDefs = MakeVars()
    installPath = paths.appSecureExePath
    parentPath  = paths.parentSecureExePath
    pipePath    = paths.keyPipePath
    return [ makeVarDefs.installPath + '=' + installPath, \
             makeVarDefs.parentPath  + '=' + parentPath, \
             makeVarDefs.pipePath    + '=' + pipePath, \
             makeVarDefs.keyLimit    + '=' + str(keyLimit), \
             'CONFIG=Debug' ]
