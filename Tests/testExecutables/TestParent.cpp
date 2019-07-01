/**
 * @file  TestParent.cpp
 * 
 * @brief  A minimal parent application implementation used when testing
 *         KeyDaemon.
 *
 *  A KeyDaemon installation is intended to support only one executable,
 * and will refuse to work unless the process that starts it is running that
 * specific parent executable. TestParent is a minimal application meant to
 * serve as that parent application when testing KeyDaemon.
 */

#include <string>
#include <iostream>
#include <stdlib.h>

int main(int argc, char** argv)
{
    std::cout << "TestParent: Running from " << argv[0] << "\n";
    std::string command(argv[1]);
    for (int i = 2; i < argc; i++)
    {
        command += " ";
        command += argv[i];
    }
    std::cout << "TestParent: Starting key daemon using command \"" 
            << command << "\"\n";
    return WEXITSTATUS(system(command.c_str()));
}
