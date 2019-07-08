/**
 * @file  KeyCode.h
 *
 * @brief  Reads and validates keyboard code values.
 */

#pragma once
#include <vector>

namespace KeyCode
{
    /**
     * @brief  Parses keyboard codes from a set of command line arguments, 
     *         returning the list of codes only when no errors or invalid
     *         codes are encountered.
     *
     * @param argc  The number of command line arguments to parse.
     *
     * @param argv  An array of arguments. All arguments from index 1 onward
     *              must be valid key codes.
     *
     * @return      The sorted list of parsed codes, or an empty list if the
     *              codes were invalid or any errors occur.
     */
    std::vector<int> parseCodes(const int argc, char** argv);
}
