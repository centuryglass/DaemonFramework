"""
Tests how KeyDaemon handles different values set on compilation.

On execution, KeyDaemon checks several security conditions, and refuses to run
if those conditions are not met. These conditions are set at compilation time as
arguments passed to the `make` command. These tests provides a collection of
possible definitions for each of those arguments, and verifies that all possible
combinations of those arguments succeed or fail as expected. 
"""

from supportModules import testActions, testDefs
from supportModules.testActions import TestResult
from abc import ABC, abstractmethod

paths = testDefs.TestPaths()
makeVars = testDefs.MakeVars()

"""An abstract basis for classes that store information about a tested path."""
class AbstractPath(ABC):
    """
    Saves path information on construction.

    Keyword Arguments:
    path           -- An absolute path string.
    description    -- A brief description of this test value's purpose.
    expectedResult -- The best ResultCode expected when using this path.
    """
    def __init__(self, path, description, expectedResult):
        self._path = path
        self._description = description
        self._expectedResult = expectedResult
    """Return a string describing the type of path this object stores."""
    @abstractmethod
    def typeDescription(self):
        pass
    """Return the full path."""
    @property
    def path(self):
        return self._path
    """Return the expected result if this path is used."""
    @property
    def expectedResult(self):
        return self._expectedResult
    """Return a string describing this tested path."""
    def description(self):
        return '[' + self.typeDescription() + ']:' + self._description

"""
Stores information about an application installation path that should be tested.
"""
class InstallPath(AbstractPath):
    """Return a string describing the type of path this object stores."""
    def typeDescription(self):
        return 'Install Path'

"""
Stores information about a parent application path that should be tested.

The KeyDaemon should only run when launched by a process that is running an
executable at this path.
"""
class ParentPath(AbstractPath):
    """Return a string describing the type of path this object stores."""
    def typeDescription(self):
        return 'Parent Path'

"""
Stores information about a tracked key code limit that should be tested.

This limit controls the maximum number of key codes that the KeyDaemon will be
allowed to track.
"""
class KeyLimit:
    """
    Saves information about the tested limit value on construction.

    Keyword Arguments:
    limit          -- A string to provide as the maximum tracked key count.
    description    -- A brief description of this test value's purpose.
    expectedResult -- The best ResultCode expected when using this limit.
    """
    def __init__(self, limit, description, expectedResult):
        self._limit = limit
        self._description = description
        self._expectedResult = expectedResult
    """Return the key limit argument string."""
    @property
    def limit(self):
        return self._limit
    """Return the expected result when using this limit."""
    @property
    def expectedResult(self):
        return self._expectedResult
    """Return a string describing this tested limit."""
    def description(self):
        return '[Key Limit]:' + self._description

"""
Stores a set of makefile variables to test and the expected test result.
"""
class BuildConfigTest:
    """
    Stores makefile variables and finds the expected result on construction.

    Keyword Arguments:
    installPath -- The InstallPath object used in this test.
    parentPath  -- The ParentPath object used in this test.
    keyLimit    -- The KeyLimit object used in this test.
    """
    def __init__(self, installPath, parentPath, keyLimit):
        self._installPath = installPath
        self._parentPath = parentPath
        self._keyLimit = keyLimit
        # Tests should always finish with the worst possible expected result
        # of all of their build options:
        worstResult = installPath.expectedResult
        nextResult = parentPath.expectedResult
        if(nextResult.value < worstResult.value):
            worstResult = nextResult
        nextResult = keyLimit.expectedResult
        if(nextResult.value < worstResult.value):
            worstResult = nextResult
        self._expectedResult = worstResult
    """Return the makefile argument list used when running the test."""
    def getMakeArgs(self):
        varNames = testDefs.MakeVars()
        return [varNames.installPath + '=' + str(self._installPath.path), \
                varNames.parentPath  + '=' + str(self._parentPath.path), \
                varNames.keyLimit    + '=' + str(self._keyLimit.limit)]
    """Return the full description of this test's build arguments."""
    def description(self):
        return self._installPath.description() + ' ' \
             + self._parentPath.description() + ' ' \
             + self._keyLimit.description()
    """Return the expected result when running this test."""
    @property
    def expectedResult(self):
        return self._expectedResult
    """Return the installation path used by this test."""
    def installPath(self):
        return self._installPath.path
    """Return the parent application path used by this test."""
    def parentPath(self):
        return self._parentPath.path

"""
Generates all possible BuildConfigTest objects for a set of test arguments.

BuildConfigTestSet stores lists of all InstallPath, ParentPath, and KeyLimit
objects. It uses these objects to create and iterate through all possible
combinations of these list values, returned as BuildConfigTest objects.
"""
class BuildConfigTestSet:
    """Construct an empty test set."""
    def __init__(self):
        self._installPaths = []
        self._parentPaths = []
        self._keyLimits = []
        self._testIndex = 0
    """Add a new InstallPath object to the list of tested option values."""
    def addInstallPath(self, path):
        self._installPaths.append(path)
    """Add a new ParentPath object to the list of tested option values."""
    def addParentPath(self, path):
        self._parentPaths.append(path)
    """Add a new KeyLimit object to the list of tested option values."""
    def addKeyLimit(self, limit):
        self._keyLimits.append(limit)
    """Return the number of unique BuildConfigTest objects available"""
    def numBuildConfigTests(self):
        count = len(self._installPaths)
        count = count * len(self._parentPaths)
        count = count * len(self._keyLimits)
        return count
    """Select a BuildConfigTest object using an index value."""
    def getBuildConfigTest(self, index):
        count = self.numBuildConfigTests()
        assert index >= 0
        assert index < count
        installPathCount = len(self._installPaths)
        parentPathCount  = len(self._parentPaths)
        keyLimitCount    = len(self._keyLimits)
        installIndex = index % installPathCount
        index = index // installPathCount
        parentIndex = index % parentPathCount
        index = index // parentPathCount
        assert index < keyLimitCount
        return BuildConfigTest(self._installPaths[installIndex], \
                               self._parentPaths[parentIndex], \
                               self._keyLimits[index])
    """Allow the test set to be used as an iterator."""
    def __iter__(self):
        return self
    """Return the next BuildConfigTest when iterating through the set."""
    def __next__(self):
        index = self._testIndex
        if (index < self.numBuildConfigTests()):
            self._testIndex += 1
            return self.getBuildConfigTest(index)
        else:
            raise StopIteration

#### Prepare build variable tests:
buildConfigTests = BuildConfigTestSet()

### Test installation directories:
# The application should refuse to run from unsecured directories.
buildConfigTests.addInstallPath(InstallPath('', \
                                            'empty', \
                                            TestResult.buildFailure))
buildConfigTests.addInstallPath(InstallPath(paths.appUnsecureExePath, \
                                            'unsecured', \
                                            TestResult.runtimeError))
buildConfigTests.addInstallPath(InstallPath(paths.appSecureExePath, \
                                            'secured', \
                                            TestResult.success))

### Test parent application directories:
# The application should refuse to run unless launched by a valid parent in a
# secured directory.
buildConfigTests.addParentPath(ParentPath('', \
                                          'empty', \
                                          TestResult.buildFailure))
buildConfigTests.addParentPath(ParentPath(paths.parentUnsecureExePath, \
                                          'unsecured', \
                                          TestResult.runtimeError))
buildConfigTests.addParentPath(ParentPath(paths.parentSecureExePath, \
                                          'secured', \
                                          TestResult.success))

### Test key definition limits: ###
# The application should refuse to run without a valid limit on the number of
# tracked keys, or fail to build if given a non-numeric limit value.
buildConfigTests.addKeyLimit(KeyLimit('', \
                                      'empty', \
                                      TestResult.buildFailure))
buildConfigTests.addKeyLimit(KeyLimit('five', \
                                      'bad type', \
                                      TestResult.buildFailure))
buildConfigTests.addKeyLimit(KeyLimit(-20, \
                                      'negative', \
                                      TestResult.runtimeError))
buildConfigTests.addKeyLimit(KeyLimit(0, \
                                      'zero', \
                                      TestResult.runtimeError))
buildConfigTests.addKeyLimit(KeyLimit(20, \
                                      'valid', \
                                      TestResult.success))

"""Runs all build variable tests."""
def runTests():
    print('Running compilation argument tests:')
    testCount = buildConfigTests.numBuildConfigTests()
    testsPassed = 0
    for index, buildTest in enumerate(buildConfigTests):
        indexStr    = 'Test ' + str(index + 1) + '/' + str(testCount)
        logFile     = open(paths.tempLogPath, 'w')
        description = buildTest.description()
        makeArgs    = buildTest.getMakeArgs()
        installPath = buildTest.installPath()
        parentPath  = buildTest.parentPath()
        expected    = buildTest.expectedResult
        result = testActions.fullTest(makeArgs, installPath, parentPath, \
                                      outFile = logFile)
        resultMatched = testActions.checkResult(result, expected, indexStr, \
                                                description, logFile)
        if (resultMatched):
            testsPassed += 1
    print('Passed ' + str(testsPassed) + '/' + str(testCount) \
          + ' build argument tests.\n')
