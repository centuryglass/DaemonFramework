/**
 * @file  KeyEventType.h
 *
 * @brief  Defines key event codes used when reading keyboard event files or
 *         sending keyboard events to KeyDaemon's parent process.
 */

#pragma once
enum class KeyEventType
{
    released = 0,
    pressed = 1,
    held = 2,

    trackedTypeCount = 3
};
