"""
testArgs manages command line arguments entered when running daemon tests.
"""
import os

"""Holds the values of a test's command line arguments."""
class Values():
    def __init__(self, verbose, debugBuild, printHelp, timeout, untilFailure):
        self._verbose      = verbose
        self._debugBuild   = debugBuild
        self._printHelp    = printHelp
        self._timeout      = timeout
        self._untilFailure = untilFailure
    """Return whether the test should print verbose output messages."""
    @property
    def useVerbose(self):
        return self._verbose
    """Return whether the test should build in Debug mode."""
    @property
    def debugBuild(self):
        return self._debugBuild
    """Return whether the test should print help information and exit."""
    @property
    def printHelp(self):
        return self._printHelp
    """Return how long the daemon should run before closing automatically."""
    @property
    def timeout(self):
        return self._timeout
    """Return whether all tests should stop at the first encountered error."""
    @property
    def exitOnFailure(self):
        return self._untilFailure

"""Read command line arguments and returns them as a TestArgs object."""
def read():
    verbose      = False
    debug        = True
    printHelp    = False
    timeout      = None
    untilFailure = False
    import sys
    for arg in sys.argv[1:]:
        if arg == '-v' or arg == '--verbose':
            verbose = True
        elif arg == '-r' or arg == '--release':
            debug = False  #Use release mode instead
        elif arg == '-h' or arg == '--help':
            printHelp = True
        elif arg == '-u' or arg == '--until-failure':
            untilFailure = True
        elif arg[:3] == '-t=':
            timeout = int(arg[3:])
        elif arg[:11] == '--timeout=':
            timeout = int(arg[11:])
        else:
            print('Warning: argument "' + arg + '" not recognized.')
    return Values(verbose, debug, printHelp, timeout, untilFailure)

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
    print('\t-v, --verbose:          ' \
          + 'Use verbose build/test logging.')
    print('\t-r, --release:          ' \
          + 'Build in Release mode instead of Debug.')
    print('\t-t, --timeout=[number]: ' \
          + 'Seconds to run the daemon before exiting.')
    print('\t-u, --until-failure:    ' \
          + 'Stop after the first failed test.')
    print('\t-h, --help:  Print this help text and exit.')
    import sys
    sys.exit('')
