#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from supportModules import testActions
from supportModules import testDefs
import buildTest
import buildArgTests
import trackedKeyTests
import keyReadingTest

args = testActions.readArgs()
if (args.printHelp):
    testDefs.printHelp('TestAll.py', 'Runs all KeyDaemon tests.')
testActions.setup()
buildTest.runTest(args)
buildArgTests.runTests(args)
trackedKeyTests.runTests(args)
keyReadingTest.runTest(args)
