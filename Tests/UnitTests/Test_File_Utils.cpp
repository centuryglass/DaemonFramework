#include "catch.hpp"
#include "File_Utils.h"
#include <sys/stat.h>

TEST_CASE("Parent paths are found correctly." "[parentDir]")
{
    INFO("Testing: File::Utils::parentDir");
    using std::string;
    using namespace DaemonFramework::File::Utils;
    const string emptyString;
    REQUIRE(parentDir(emptyString)   == emptyString);
    const string rootPath("/");
    REQUIRE(parentDir(rootPath)      == emptyString);
    const string altRootPath("//");
    REQUIRE(parentDir(altRootPath)   == emptyString);
    const string topLevelPath("/dev");
    REQUIRE(parentDir(topLevelPath)  == rootPath);
    const string altTopPath("/etc/");
    REQUIRE(parentDir(altTopPath)    == rootPath);
    const string invalid("this is not a real path");
    REQUIRE(parentDir(invalid)       == emptyString);
    const string deeperPath("/this/is/an/example/path");
    REQUIRE(parentDir(deeperPath)    == "/this/is/an/example");
    const string altDeeperPath("/yet /another /layered /path/");
    REQUIRE(parentDir(altDeeperPath) == "/yet /another /layered ");
}

TEST_CASE("File types are determined correctly." "[getPathType]")
{
    INFO("Testing: File::Utils::getPathType");
    using std::string;
    using DaemonFramework::File::PathType;
    using namespace DaemonFramework::File::Utils;
    const string invalid;
    REQUIRE(getPathType(invalid) == PathType::invalid);
    const string nonexistent("/valid/but/does/not/exist");
    REQUIRE(getPathType(nonexistent) == PathType::nonexistent);
    const string makefile(MAKEFILE_PATH);
    REQUIRE(getPathType(makefile) == PathType::file);
    const string standardDir("/usr/bin");
    REQUIRE(getPathType(standardDir) == PathType::directory);
    const string charDevice("/dev/urandom");
    REQUIRE(getPathType(charDevice) == PathType::characterDevice);
    const string blockDevice("/dev/sda");
    REQUIRE(getPathType(blockDevice) == PathType::blockDevice);
    system("mkfifo testPipe");
    const string pipe("testPipe");
    REQUIRE(getPathType(pipe) == PathType::namedPipe);
    system("rm testPipe");
    system("ln -s /dev/null");
    const string symbolicLink("null");
    REQUIRE(getPathType(symbolicLink) == PathType::characterDevice);
    REQUIRE(getPathType(symbolicLink, false) == PathType::symbolicLink);
    system("rm null");

    // I'm not sure that any socket files are truly universal for all systems,
    // but this is probably about the closest we're gonna get. If you're using
    // a system without DBus you'll have to figure something else out or just 
    // ignore this test.
    const string standardSocket("/var/run/dbus/system_bus_socket");
    REQUIRE(getPathType(standardSocket) == PathType::socket);
}

TEST_CASE("Directories are created correctly." "[createDir]")
{
    INFO("Testing: File::Utils::createDir\n");
    using std::string;
    using namespace DaemonFramework::File::Utils;
    string newDir;
    SECTION("createDir returns true for directories that already exist.")
    {
        string homeDir("/home");
        REQUIRE(createDir(homeDir));
        newDir = "temporary";
        system("mkdir temporary");
        REQUIRE(createDir(newDir));
    }
    SECTION("createDir returns false for directories that cannot be created.")
    {
        // Don't run this as root...
        const string invalidSysDir("/sys/fakeSystemDir");
        REQUIRE(! createDir(invalidSysDir));
        const string invalidRootDir("/youBetterNotActuallyHaveThisDirectory");
        REQUIRE(! createDir(invalidRootDir));
        const string deeperSysDir("/sys/directories/you/cannot/create");
        REQUIRE(! createDir(deeperSysDir));
    }
    SECTION("createDir correctly creates basic new directories.")
    {
        struct stat dirStat;
        newDir = "TestDir";
        CHECK(stat(newDir.c_str(), &dirStat) == -1);
        REQUIRE(createDir(newDir));
        REQUIRE(stat(newDir.c_str(), &dirStat) == 0);
        const mode_t perms = dirStat.st_mode & ~S_IFMT;
        REQUIRE(perms == S_IRWXU);
    }
    SECTION("createDir creates directories with the expected default mode.")
    {
        struct stat dirStat;
        const mode_t mode = S_IRWXU;
        newDir = "TestDir";
        CHECK(stat(newDir.c_str(), &dirStat) == -1);
        REQUIRE(createDir(newDir));
    }
    SECTION("createDir recursively creates directories.")
    {
        struct stat dirStat;
        newDir = "BaseDir";
        CHECK(stat(newDir.c_str(), &dirStat) == -1);
        const string recursiveDir(newDir + "/new/directory/tree");
        REQUIRE(createDir(recursiveDir));
        REQUIRE(stat(recursiveDir.c_str(), &dirStat) == 0);
    }
    if (! newDir.empty())
    {
        string rmCommand("rm -r ");
        rmCommand += newDir;
        system(rmCommand.c_str());
    }


}
