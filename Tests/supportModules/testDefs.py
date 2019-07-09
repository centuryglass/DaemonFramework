"""testDefs defines constant values used when testing the KeyDaemon project."""
import os

"""Defines all relevant makefile variable names."""
class MakeVars:
    def __init__(self):
        self._installPath = 'INSTALL_PATH'
        self._parentPath  = 'PARENT_PATH'
        self._pipePath    = 'KEY_PIPE_PATH'
        self._keyLimit    = 'KEY_LIMIT'
        self._timeout     = 'TIMEOUT'
        self._configMode  = 'CONFIG'
        self._verbose     = 'V'
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
    """Return the KeyDaemon timeout period build variable name."""
    @property
    def timeout(self):
        return self._timeout
    """Return the Debug/Release mode build variable name."""
    @property
    def configMode(self):
        return self._configMode
    """Return the KeyDaemon verbose outbut build variable name."""
    @property
    def verbose(self):
        return self._verbose
makeVars = MakeVars()

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
Gets a complete list of build arguments to pass to the 'make' command.

All parameters have default test values provided. If all defaults are used, the
KeyDaemon should always build, install, and run correctly.
    
Keyword Arguments:
installPath  -- The path where the KeyDaemon will be installed.
                (default: paths.appSecureExePath)
parentPath   -- The path where the KeyDaemon's parent application should have.
                (default: paths.parentSecureExePath)
pipePath     -- The path to the named pipe the KeyDaemon uses to send codes.
                (default: paths.keyPipePath)
keyLimit     -- The maximum number of tracked key codes allowed. (default: 1)
testArgs     -- A testActions.TestArgs object. (default: None)
                If not None, this object's properties override all parameters
                listed below.
debugBuild   -- Whether the KeyDaemon builds in debug mode instead of release.
                (default: True)
verbose      -- Whether the KeyDaemon prints verbose build and runtime messages.
                (default: False)
timeout      -- Seconds before the KeyDaemon exits, or False to disable timeout.
                (default: 1)
"""
def getMakeArgs(installPath = paths.appSecureExePath, \
                         parentPath = paths.parentSecureExePath, \
                         pipePath = paths.keyPipePath, \
                         keyLimit = 1, \
                         testArgs = None, \
                         debugBuild = True, \
                         verbose = False,
                         timeout = 1):
    if testArgs is not None:
        debugBuild = testArgs.debugBuild
        verbose = testArgs.verbose
        if testArgs.timeout is not None:
            timeout = testArgs.timeout
    argList = [makeVars.installPath + '=' + installPath, \
               makeVars.parentPath  + '=' + parentPath, \
               makeVars.pipePath    + '=' + pipePath, \
               makeVars.keyLimit    + '=' + str(keyLimit), \
               makeVars.configMode  + '=' + 'Debug' if debugBuild \
                                                       else 'Release']
    if verbose:
        argList.append(makeVars.verbose + '= 1')
    if timeout:
        argList.append(makeVars.timeout + '=' + str(timeout))
    return argList

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
