#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from supportModules import testActions
import buildTest
import buildArgTests
import trackedKeyTests

testActions.setup()
buildTest.runTests()
buildArgTests.runTests()
trackedKeyTests.runTests()
