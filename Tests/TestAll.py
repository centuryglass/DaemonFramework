#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from supportModules import testActions
import buildTest
import buildArgTests
import trackedKeyTests
import keyReadingTest

testActions.setup()
buildTest.runTest()
buildArgTests.runTests()
trackedKeyTests.runTests()
keyReadingTest.runTest()
