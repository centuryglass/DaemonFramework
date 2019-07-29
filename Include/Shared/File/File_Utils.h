/**
 * @file  File_Utils.h
 *
 * @brief  Handles Linux file operations with error checking.
 */

#pragma once
#include "File_PathType.h"
#include <sys/stat.h>
#include <string>

namespace DaemonFramework { namespace File { namespace Utils { 

    /**
     * @brief  Given a file path, get the path to that file's parent directory.
     * 
     *  The file at the given path does not need to exist, and the returned
     * parent path doesn't necessarily exist either. This function just cuts
     * all characters after the last '/' character in the string that isn't
     * also the absolute last character in the string.
     *
     * @param path  The path string to modify.
     *
     * @return      The parent path string, or the empty string if the parent
     *              path could not be found.
     */
    std::string parentDir(const std::string& path);

    /**
     * @brief  Returns the type of file (if any) encountered at a given path.
     *
     * @param path  The path string to check.
     *
     * @param followLinks  Whether a path to a link should return the type of
     *                     the linked file rather than the
     *                     PathType::symbolicLink type.
     *
     * @return             The PathType value that best describes the path.
     */
    PathType getPathType(const std::string& path,
            const bool followLinks = true);

    /**
     * @brief  Creates a directory at the given path, recursively creating
     *         parent directories if necessary.
     *
     * @param path  The directory path to create.
     *
     * @param mode  The access permissions to apply to created directories. If
     *              not specified, read/write permissions will be given to the
     *              owner only.
     *
     * @return      True if the directory was created or already existed, false
     *              if the directory does not exist and could not be created.
     */
    bool createDir
    (const std::string& path, const mode_t accessMode = S_IRWXU);

} } }

