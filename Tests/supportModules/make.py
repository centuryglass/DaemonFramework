"""
Provides constants and functions used when building KeyDaemon with the Unix
'make' command.
"""
import os
import subprocess
from supportModules import testDefs, pathConstants
paths = pathConstants.paths

"""Defines all relevant makefile variable names."""
class VarNames:
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
varNames = VarNames()

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
def getBuildArgs(installPath = paths.appSecureExePath, \
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
    argList = [varNames.installPath + '=' + installPath, \
               varNames.parentPath  + '=' + parentPath, \
               varNames.pipePath    + '=' + pipePath, \
               varNames.keyLimit    + '=' + str(keyLimit), \
               varNames.configMode  + '=' + 'Debug' if debugBuild \
                                                       else 'Release']
    if verbose:
        argList.append(varNames.verbose + '= 1')
    if timeout:
        argList.append(varNames.timeout + '=' + str(timeout))
    return argList

"""
Uninstalls KeyDaemon and deletes all build files.

Keyword Arguments:
makeArgs -- The set of command line arguments to pass to the `make` process.
            These are required to ensure that KeyDaemon is removed from the
            correct installation path.

outFile  -- A file where test output from stdout and stderr will be sent.
            The default subprocess.DEVNULL value discards all output.
"""
def uninstall(makeArgs, outFile = subprocess.DEVNULL):
    os.chdir(paths.projectDir)
    subprocess.call(['make', 'uninstall'] + makeArgs, \
                    stdout = outFile, \
                    stderr = outFile)
    subprocess.call(['make', 'clean'] + makeArgs, stdout = outFile, \
                    stderr = outFile)

"""
Attempts to builds and install KeyDaemon.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.
               These are required to ensure that KeyDaemon is removed from the
               correct installation path.

installPath -- The full path where the KeyDaemon executable will be installed.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.

debugBuild  -- Whether the Keydaemon builds in debug mode. (default: True)
"""
def buildInstall(makeArgs, installPath, outFile = subprocess.DEVNULL, \
                 debugBuild = True):
    os.chdir(paths.projectDir)
    subprocess.call(['make'] + makeArgs, stdout = outFile, stderr = outFile)
    subprocess.call(['make', 'install'] + makeArgs, stdout = outFile, \
                    stderr = outFile)
