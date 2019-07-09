#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from testModules import basicBuild, buildArguments, trackedKeys, keyReading
from supportModules import testActions, testArgs

args = testArgs.read()
if (args.printHelp):
    testDefs.printHelp('TestAll.py', 'Runs all KeyDaemon tests.')
testActions.setup()
basicBuild.runTest(args)
buildArguments.runTests(args)
trackedKeys.runTests(args)
keyReading.runTest(args)
