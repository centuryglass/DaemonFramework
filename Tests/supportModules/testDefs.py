"""testDefs defines constant values used when testing the KeyDaemon project."""
import os

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
