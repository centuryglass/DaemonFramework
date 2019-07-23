"""
Provides constants and functions used when building DaemonFramework applications
with the 'make' command.
"""
import os
import subprocess
import tempfile
import time
from supportModules import pathConstants
paths = pathConstants.paths

"""Defines all relevant makefile variable names."""
class VarNames:
    def __init__(self):
        self._daemonPath        = 'DF_DAEMON_PATH'
        self._parentPath        = 'DF_REQUIRED_PARENT_PATH'
        self._inPipePath        = 'DF_INPUT_PIPE_PATH'
        self._outPipePath       = 'DF_OUTPUT_PIPE_PATH'
        self._lockPath          = 'DF_LOCK_FILE_PATH'
        self._checkPath         = 'DF_VERIFY_PATH'
        self._securePath        = 'DF_VERIFY_PATH_SECURITY'
        self._secureParentPath  = 'DF_VERIFY_PARENT_PATH_SECURITY'
        self._parentRunning     = 'DF_REQUIRE_RUNNING_PARENT'
        self._timeout           = 'DF_TIMEOUT'
        self._configMode        = 'DF_CONFIG'
        self._verbose           = 'DF_VERBOSE'
    """Return the daemon install path variable name."""
    @property
    def daemonPath(self):
        return self._daemonPath
    """
    Return the daemon parent process path variable name.
    If defined, the daemon will only run if launched by an executable stored at
    this path.
    """
    @property
    def parentPath(self):
        return self._parentPath
    """Return the daemon input pipe file path variable name."""
    @property
    def inPipePath(self):
        return self._inPipePath
    """Return the daemon output pipe file path variable name."""
    @property
    def outPipePath(self):
        return self._outPipePath
    """Return the daemon instance lock file path variable name."""
    @property
    def lockPath(self):
        return self._lockPath
    """Return the daemon path verification variable name."""
    @property
    def checkPath(self):
        return self._checkPath
    """Return the daemon path security verification variable name."""
    @property
    def securePath(self):
        return self._securePath
    """Return the daemon parent path security verification variable name."""
    @property
    def secureParentPath(self):
        return self._secureParentPath
    """Return the running daemon parent requirement variable name."""
    @property
    def parentRunning(self):
        return self._parentRunning
    """Return the daemon timeout period build variable name."""
    @property
    def timeout(self):
        return self._timeout
    """Return the Debug/Release mode build variable name."""
    @property
    def configMode(self):
        return self._configMode
    """Return the daemon verbose outbut build variable name."""
    @property
    def verbose(self):
        return self._verbose
varNames = VarNames()

"""
Gets a complete list of build arguments to pass to the 'make' command.

All parameters have default test values provided. If all defaults are used, the
daemon should always build, install, and run correctly.
    
Keyword Arguments:
daemonPath      -- The path where the daemon will be installed.
                   (default: paths.daemonSecureExePath)
parentPath      -- The required path to the daemon's parent application.
                   (default: paths.parentSecureExePath)
inPipePath      -- The daemon's input pipe path.
                   (default: paths.inPipePath)
outPipePath     -- The daemon's output pipe path.
                   (default: paths.outPipePath)
lockPath        -- The daemon's instance lock path.
                   (default: paths.lockPath)
checkPath       -- Sets if the daemon must run from installPath.
                   (default: True)
securePath      -- Sets if the daemon must run from parentPath
                   (default: True)
secureParent    -- Whether the daemon should check if its parent is secured.
                   (default: True)
requireSingular -- Sets if only one daemon instance may run.
                   (default: True)
parentRunning   -- Sets if the daemon should exit when its parent exits.
                   (default: true)
testArgs        -- A testArgs.Values object. (default: None)
                   If not None, this object's properties override all parameters
                   listed below.
debugBuild      -- Whether the daemon builds in debug mode instead of release.
                   (default: True)
verbose         -- Whether the daemon prints verbose messages.
                   (default: False)
timeout         -- Seconds before the daemon exits, or False to disable timeout.
                   (default: 1)
"""
def getBuildArgs(daemonPath = paths.daemonSecureExePath, \
                         parentPath = paths.parentSecureExePath, \
                         inPipePath = paths.inPipePath, \
                         outPipePath = paths.outPipePath, \
                         lockPath = paths.lockPath, \
                         checkPath = True, \
                         securePath = True, \
                         secureParent = True, \
                         parentRunning = True, \
                         testArgs = None, \
                         debugBuild = True, \
                         verbose = False,
                         timeout = 1):
    if testArgs is not None:
        debugBuild = testArgs.debugBuild
        verbose = testArgs.useVerbose
        if testArgs.timeout is not None:
            timeout = testArgs.timeout
    argList = [varNames.configMode + '=' + ('Debug' if debugBuild \
                                            else 'Release')]
    stringArgs = [(daemonPath, varNames.daemonPath), \
                  (parentPath, varNames.parentPath), \
                  (inPipePath, varNames.inPipePath), \
                  (outPipePath, varNames.outPipePath), \
                  (lockPath, varNames.lockPath)]
    for stringValue, varName in stringArgs:
        if stringValue is not None:
            argList.append(varName + '=' + stringValue)

    booleanArgs = [(checkPath, varNames.checkPath), \
                   (securePath, varNames.securePath), \
                   (secureParent, varNames.secureParentPath), \
                   (parentRunning, varNames.parentRunning), \
                   (verbose, varNames.verbose)]
    for boolValue, varName in booleanArgs:
        argList.append(varName + ('=1' if boolValue else '=0'))
    argList.append(varNames.timeout + '=' + str(timeout))
    return argList

"""
Writes a set of build arguments to a log file.
Keyword Arguments:
makeArgs -- A list of build arguments.
logFile  -- The file where the build arguments will be logged.
title    -- A title to print before the argument list.
indent   -- The number of spaces to indent each logged line. (default: 2)
"""
def logBuildArgs(makeArgs, logFile, title, indent = 2):
    if title is not None:
        logFile.write((' ' * indent) + title + '\n')
    for arg in makeArgs:
        logFile.write((' ' * indent) + '-' + arg + '\n')


"""
Given a build argument set, return the value of a specific makefile variable.
Keyword Arguments:
makeArgs -- A set of makefile arguments that may affect the install path.

varName  -- The name of the variable to check.

makePath -- The path to the makefile where the variable should be checked.
            (default: 'Makefile')
"""
def readMakeVar(makeArgs, varName, makePath = 'Makefile'):
    with tempfile.TemporaryFile() as tempOutputFile:
        subprocess.run(['make', '-f', makePath, 'print-' + varName] + makeArgs,\
                         stdout = tempOutputFile)
        tempOutputFile.seek(0)
        return tempOutputFile.read().decode('utf-8').rstrip()

"""
Attempts to build a target, returning whether the build succeeded.

Keyword Arguments:
makeDir     -- The directory where the target's makefile is found

makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def buildTarget(makeDir, makeArgs, outFile = subprocess.DEVNULL):
    os.chdir(makeDir)
    targetName = readMakeVar(makeArgs, 'APP_TARGET')
    targetPath = os.path.join(makeDir, targetName)
    if os.path.isfile(targetPath):
        os.remove(targetPath)
    subprocess.call(['make'] + makeArgs, stdout = outFile, stderr = outFile)
    return os.path.isfile(targetPath)

"""
Attempts to install a target, returning whether the installation succeeded.

Keyword Arguments:
makeDir     -- The directory where the target's makefile is found

makeArgs    -- The set of command line arguments to pass to the `make` process.

installVar  -- The makefile variable name that sets the installation path.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def installTarget(makeDir, makeArgs, installVar, outFile = subprocess.DEVNULL):
    os.chdir(makeDir)
    targetPath = readMakeVar(makeArgs, installVar)
    preBuildTime = time.time()
    subprocess.call(['make'] + makeArgs, stdout = outFile, stderr = outFile)
    subprocess.call(['make', 'install'] + makeArgs, stdout = outFile, \
                    stderr = outFile)
    if not os.path.isfile(targetPath):
        outFile.write('make.installTarget: No file installed to path "' \
                      + targetPath + '"')
        return False
    if preBuildTime >= os.path.getmtime(targetPath):
        outFile.write('make.installTarget: File installed to path "' \
                      + targetPath + '" was not updated.')
        return False
    return True


"""
Deletes a target's build files.
Keyword Arguments:
makeDir     -- The directory where the target's makefile is found.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def cleanTarget(makeDir, pathVarName, outFile = subprocess.DEVNULL):
    os.chdir(makeDir)
    subprocess.call(['make', 'clean', 'CONFIG=Debug'], \
                    stdout = outFile, \
                    stderr = outFile)
    subprocess.call(['make', 'clean', 'CONFIG=Release'], \
                    stdout = outFile, \
                    stderr = outFile)

"""
Uninstalls a target.
Keyword Arguments:
makeDir     -- The directory where the target's makefile is found.

pathVarName  -- The makefile variable name that sets the target install path.

execPath     -- The path to the target's installed executable.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def uninstallTarget(makeDir, pathVarName, execPath, \
                    outFile = subprocess.DEVNULL):
    os.chdir(makeDir)
    subprocess.call(['make', 'uninstall', pathVarName + '=' + execPath], \
                    stdout = outFile, \
                    stderr = outFile)

"""
Attempts to build the BasicDaemon, returning whether the build succeeded.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def buildBasicDaemon(makeArgs, outFile = subprocess.DEVNULL):
    return buildTarget(paths.basicDaemonDir, makeArgs, outFile)

"""
Attempts to install the BasicDaemon, returning whether installation succeeded.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def installBasicDaemon(makeArgs, outFile = subprocess.DEVNULL):
    return installTarget(paths.basicDaemonDir, makeArgs, varNames.daemonPath, \
                         outFile)

"""
Deletes the daemon's build files.
Keyword Arguments:

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def cleanDaemon(outFile = subprocess.DEVNULL):
    cleanTarget(paths.basicDaemonDir, outFile)

"""
Uninstalls the daemon.
Keyword Arguments:
daemonPath  -- The path to the installed daemon executable to remove.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def uninstallDaemon(daemonPath, outFile = subprocess.DEVNULL):
    uninstallTarget(paths.basicDaemonDir, varNames.daemonPath, daemonPath, \
                    outFile)

"""
Attempts to build the BasicParent, returning whether the build succeeded.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def buildBasicParent(makeArgs, outFile = subprocess.DEVNULL):
    return buildTarget(paths.basicParentDir, makeArgs, outFile)

"""
Attempts to install the BasicParent, returning whether installation succeeded.

Keyword Arguments:
makeArgs    -- The set of command line arguments to pass to the `make` process.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def installBasicParent(makeArgs, outFile = subprocess.DEVNULL):
    return installTarget(paths.basicParentDir, makeArgs, varNames.parentPath, \
                         outFile)

"""
Deletes the parent's build files.
Keyword Arguments:
outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def cleanParent(outFile = subprocess.DEVNULL):
    cleanTarget(paths.basicParentDir, outFile)

"""
Uninstalls the parent.
Keyword Arguments:
parentPath  -- The path to the installed parent executable to remove.

outFile     -- A file where test output from stdout and stderr will be sent.
               The default subprocess.DEVNULL value discards all output.
"""
def uninstallParent(parentPath, outFile = subprocess.DEVNULL):
    uninstallTarget(paths.basicParentDir, varNames.parentPath, parentPath, \
                    outFile)
