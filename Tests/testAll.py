#!/usr/bin/python
"""Runs all DaemonFramework tests."""

from testModules import basicBuild, daemonPathChecking, parentPathChecking, \
                        singleRunningDaemon
from supportModules import testArgs, make, pathConstants
import sys, subprocess

args = testArgs.read()
if (args.printHelp):
    testArgs.printHelp('TestAll.py', 'Runs all DaemonFramework tests.')

print("Building and running unit tests:")
if make.buildUnitTests(make.getBuildArgs(testArgs = args), None):
    process = subprocess.run(pathConstants.paths.unitTestBuildPath)
    if process.returncode != 0 and args.exitOnFailure:
        sys.exit(process.returncode)
else:
    print("Failed to build unit tests!")
    if args.exitOnFailure:
        sys.exit(1)

print("Running python tests:")
testModules = [basicBuild, daemonPathChecking, parentPathChecking, \
               singleRunningDaemon]
testObjects = []
testCount = 0
testsPassed = 0
for testModule in testModules:
    testObject = testModule.getTests(args)
    testCount += testObject.testCount
    testObjects.append(testObject)
print('Running ' + str(testCount) + ' tests in ' + str(len(testObjects)) \
      + ' categories:')
for test in testObjects:
    testsPassed += test.runAll()
print('Passed ' + str(testsPassed) + ' of ' + str(testCount) + ' tests.')
