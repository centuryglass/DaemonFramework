#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from supportModules import testActions, testArgs
import buildTest
import buildArgTests
import trackedKeyTests
import keyReadingTest

args = testArgs.read()
if (args.printHelp):
    testDefs.printHelp('TestAll.py', 'Runs all KeyDaemon tests.')
testActions.setup()
buildTest.runTest(args)
buildArgTests.runTests(args)
trackedKeyTests.runTests(args)
keyReadingTest.runTest(args)
