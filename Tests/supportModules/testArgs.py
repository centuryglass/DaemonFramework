"""
testArgs manages command line arguments entered when running KeyDaemon tests.
"""
import os

"""Holds the values of a test's command line arguments."""
class Values():
    def __init__(self, verbose, debugBuild, printHelp, timeout = None):
        self._verbose    = verbose
        self._debugBuild = debugBuild
        self._printHelp  = printHelp
        self._timeout    = timeout
    """Return whether the test should print verbose output messages."""
    @property
    def verbose(self):
        return self._verbose
    """Return whether the test should build in Debug mode."""
    @property
    def debugBuild(self):
        return self._debugBuild
    """Return whether the test should print help information and exit."""
    @property
    def printHelp(self):
        return self._printHelp
    """Return how long the KeyDaemon should run before closing automatically."""
    @property
    def timeout(self):
        return self._timeout

"""Read command line arguments and returns them as a TestArgs object."""
def read():
    verbose   = False
    debug     = True
    printHelp = False
    timeout   = None
    import sys
    for arg in sys.argv[1:]:
        if arg == '-v':
            verbose = True
        elif arg == '-r':
            debug = False  #Use release mode instead
        elif arg == '-h' or arg == '--help':
            printHelp = True
        elif arg[:3] == '-t=':
            timeout = int(arg[3:])
        else:
            print('Warning: argument "' + arg + '" not recognized.')
    return Values(verbose, debug, printHelp)

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
