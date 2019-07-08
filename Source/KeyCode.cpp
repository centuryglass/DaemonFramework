#include "KeyCode.h"
#include "Debug.h"
#include <algorithm>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <errno.h>

#ifdef DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix = "KeyDaemon: KeyCode::";
#endif

// Parses keyboard codes from a set of command line arguments, returning the
// list of codes only when no errors or invalid codes are encountered.
std::vector<int> KeyCode::parseCodes(const int argc, char** argv)
{
    using std::vector;
    if (argc > KEY_LIMIT)
    {
        DBG(messagePrefix << __func__ << ": Key code argument count "
                << (argc - 1) << " exceeds maximum key code count " 
                << KEY_LIMIT);
        return vector<int>();
    }
    if (argc < 1)
    {
        DBG(messagePrefix << __func__ <<  ": Key code argument count " << argc
            << " is less than expected minimum count of 1.");
        return vector<int>();
    }

    vector<int> keyCodes(argc);

    for (int i = 0; i < argc; i++)
    {
        char* endPtr = nullptr;
        errno = 0;
        long codeValue = strtol(argv[i], &endPtr, 10);
        switch (errno)
        {
            case 0:
                break;
            case ERANGE:
                DBG(messagePrefix << __func__ << ": Key code \"" << argv[i]
                        << "\" exceeds numeric limits.");
                return vector<int>();
            default:
                DBG(messagePrefix << __func__ << ": Unexpected error code "
                        << errno << " when processing argument \"" << argv[i]
                        << "\".");
                return vector<int>();
        }
        if (endPtr == argv[i])
        {
            DBG(messagePrefix << __func__
                    << ": Failed to parse key code from argument \"" << argv[i]
                    << "\".");
            return vector<int>();
        }
        if (*endPtr != '\0')
        {
            DBG(messagePrefix << __func__ << ": Argument \"" << argv[i] 
                    << "\" contained non-numeric values.");
            return vector<int>();
        }
        if (codeValue <= KEY_RESERVED || codeValue >= KEY_UNKNOWN)
        {
            DBG(messagePrefix << __func__ << ": Argument \"" << argv[i] 
                    << "\" is not within the range of valid keyboard codes.");
            return vector<int>();
        }
        keyCodes.push_back((int) codeValue);
    }
    // Make sure key codes are sorted for fast code lookup:
    std::sort(keyCodes.begin(), keyCodes.end());
    DBG_V(messagePrefix << __func__ << ": Returning " << keyCodes.size()
            << " sorted key codes.");
    return keyCodes;
}
