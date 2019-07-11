/**
 * @file  Pipe.h
 *
 * @brief  Provides helper functions for the Pipe namespace.
 */

#include "Debug.h"
#include <sys/stat.h>

namespace DaemonFramework
{
namespace Pipe
{
        /**
         * @brief  Attempts to create a new pipe file at a specific path.
         *
         * @param path  The path where the pipe file will be created.
         *
         * @param mode  Access permissions to use for the new file.
         *
         * @return      True if and only if the file was created or a file
         *              already exists at that path with the correct properties.
         */
        bool createPipe(const char* path, const mode_t mode);
    }
}
