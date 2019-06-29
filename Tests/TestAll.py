#!/usr/bin/python
"""Runs all KeyDaemon tests scripts."""

from supportModules import testActions
import buildArgTests
import trackedKeyTests

testActions.setup()
buildArgTests.runTests()
trackedKeyTests.runTests()
