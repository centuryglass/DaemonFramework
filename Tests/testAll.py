#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from testModules import basicBuild, daemonPathChecking
from supportModules import testArgs

args = testArgs.read()
if (args.printHelp):
    testDefs.printHelp('TestAll.py', 'Runs all KeyDaemon tests.')
testModules = [basicBuild, daemonPathChecking]
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
