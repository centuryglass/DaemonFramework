"""keyCodes defines constants related to Linux key code values."""

"""Defines Linux key code values."""
class CodeValues:
    def __init__(self):
        self._lowestValidCode = 1
        self._highestValidCode = 239
    """Return the lowest valid keyboard event code."""
    @property
    def lowestValidCode(self):
        return self._lowestValidCode
    """Return the highest valid keyboard event code."""
    @property
    def highestValidCode(self):
        return self._highestValidCode
constants = CodeValues()
