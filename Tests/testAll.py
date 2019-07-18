#!/usr/bin/python
"""Runs all DaemonFramework tests."""

from testModules import basicBuild, daemonPathChecking, parentPathChecking
from supportModules import testArgs

args = testArgs.read()
if (args.printHelp):
    testDefs.printHelp('TestAll.py', 'Runs all DaemonFramework tests.')
testModules = [basicBuild, daemonPathChecking, parentPathChecking]
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
