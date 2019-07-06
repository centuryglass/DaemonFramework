#pragma once
/**
 * @file  KeyEventFiles.h
 *
 * @brief  Finds, identifies, and opens Linux keyboard event files.
 */

#include <vector>
#include <string>

namespace KeyEventFiles
{
    /**
     * @brief  Gets paths for all valid keyboard input event files.
     *
     * @return  The list of valid keyboard file paths.
     */
    std::vector<std::string> getPaths();
}
