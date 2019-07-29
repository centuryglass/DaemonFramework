/**
 * @file   File_PathType.h
 *
 * @brief  Represents the status of a path string.
 */

#pragma once

namespace DaemonFramework { namespace File { enum class PathType; } }

enum class DaemonFramework::File::PathType
{
    invalid,
    nonexistent,
    file,
    directory,
    characterDevice,
    blockDevice,
    namedPipe,
    symbolicLink,
    socket
};
