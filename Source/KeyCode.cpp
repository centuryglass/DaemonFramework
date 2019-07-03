#include "KeyCode.h"
#include <iostream>
#include <algorithm>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <errno.h>


// Parses keyboard codes from a set of command line arguments, returning the
// list of codes only when no errors or invalid codes are encountered.
std::vector<int> KeyCode::parseCodes(const int argc, char** argv)
{
    using std::vector;

#ifdef KEY_LIMIT
    if (argc > (KEY_LIMIT + 1))
    {
        std::cerr << "Error: Key code argument count " << (argc - 1)
            << " exceeds maximum key code count " << KEY_LIMIT << "\n";
        return vector<int>();
    }

    vector<int> keyCodes(argc - 1);

    for (int i = 1; i < argc; i++)
    {
        char* endPtr = nullptr;
        errno = 0;
        long codeValue = strtol(argv[i], &endPtr, 10);
        switch (errno)
        {
            case 0:
                break;
            case ERANGE:
                std::cerr << "Error: key code \""
                        << argv[i] << "\" exceeds numeric limits.\n";
                return vector<int>();
            default:
                std::cerr << "Error: unexpected error code " << errno
                        << " when processing argument \"" << argv[i] << "\".\n";
                return vector<int>();
        }
        if (endPtr == argv[i])
        {
            std::cerr << "Error: Failed to parse key code from argument \""
                    << argv[i] << "\".\n";
            return vector<int>();
        }
        if (*endPtr != '\0')
        {
            std::cerr << "Error: Argument \"" << argv[i] 
                    << "\" contained non-numeric values\n";
            return vector<int>();
        }
        if (codeValue <= KEY_RESERVED || codeValue >= KEY_UNKNOWN)
        {
            std::cerr << "Error: Argument \"" << argv[i] 
                    << "\" is not within the range of valid keyboard codes.\n";
            return vector<int>();
        }
        keyCodes.push_back((int) codeValue);
    }
    // Make sure key codes are sorted for fast code lookup:
    std::sort(keyCodes.begin(), keyCodes.end());
    return keyCodes;
#else
    std::cerr << "KEY_LIMIT is not defined. This value must be defined at "
            << "compile time to prevent unrestricted keyboard access.\n";
    return vector<int>();
#endif
}
