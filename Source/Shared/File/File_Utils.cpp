#include "File_Utils.h"
#include "Debug.h"

#ifdef DF_DEBUG
static const constexpr char* messagePrefix = "DaemonFramework::File::Utils::";
#endif

namespace FileUtils = DaemonFramework::File::Utils;

// Given a file path, get the path to that file's parent directory.
std::string FileUtils::parentDir(const std::string& path)
{
    if (path.empty() || path == "/" || path == "//")
    {
        return std::string();
    }
    size_t pathLength = path.length();
    size_t dirCharIdx = path.find_last_of('/', pathLength - 2);
    if (dirCharIdx == std::string::npos)
    {
        return std::string();
    }
    if (dirCharIdx == 0)
    {
        return "/";
    }
    return path.substr(0, dirCharIdx);
}


// Returns the type of file (if any) encountered at a given path.
DaemonFramework::File::PathType FileUtils::getPathType
(const std::string& path, const bool followLinks)
{
    if (path.empty())
    {
        return PathType::invalid;
    }
    struct stat fileStats;
    errno = 0;
    int statResult = -1;
    if (followLinks)
    {
        statResult = stat(path.c_str(), &fileStats);
    }
    else
    {
        statResult = lstat(path.c_str(), &fileStats);
    }
    if (statResult == -1)
    {
        if (errno == ENOENT)
        {
            return PathType::nonexistent;
        }
        DF_DBG(messagePrefix << __func__ << ": Unable to check path \""
                << path << "\"");
        DF_PERROR("Path stat error");
        return PathType::invalid;
    }
    if (S_ISREG(fileStats.st_mode))
    {
        return PathType::file;
    }
    if (S_ISDIR(fileStats.st_mode))
    {
        return PathType::directory;
    }
    if (S_ISCHR(fileStats.st_mode))
    {
        return PathType::characterDevice;
    }
    if (S_ISBLK(fileStats.st_mode))
    {
        return PathType::blockDevice;
    }
    if (S_ISFIFO(fileStats.st_mode))
    {
        return PathType::namedPipe;
    }
    if (S_ISSOCK(fileStats.st_mode))
    {
        return PathType::socket;
    }
    if (S_ISLNK(fileStats.st_mode))
    {
        return PathType::symbolicLink;
    }

    // Something is wrong if no type is selected by this point:
    DF_DBG(messagePrefix << __func__ << ": Failed to read the mode of path \""
            << path << "\"");
    DF_ASSERT(false);
    return PathType::invalid;
}


// Creates a directory at the given path, recursively creating parent
// directories if necessary.
bool DaemonFramework::File::Utils::createDir
(const std::string& path, const mode_t accessMode)
{
    if (path.empty())
    {
        return false;
    }
    PathType pathType = getPathType(path);
    if (pathType == PathType::directory)
    {
        return true; // Directory already exists.
    }
    if (pathType != PathType::nonexistent)
    {
        return false; // Path leads to some sort of non-directory file.
    }
    // Check if parent is valid, creating it if necessary and possible.
    const std::string parentPath = parentDir(path);
    if (! parentPath.empty())
    {
        const bool validParent = createDir(parentPath);
        if (! validParent)
        {
            return false;
        }
    }
    errno = 0;
    if (mkdir(path.c_str(), accessMode) != 0)
    {
        DF_DBG(messagePrefix << __func__ << ": Failed to create directory \""
                << path << "\"");
        DF_PERROR("mkdir error");
        return false;
    }
    return true;
}
