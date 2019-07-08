/**
 * @file  Debug.h
 *
 * @brief  Provides debugging macros that are removed in release builds.
 */

#pragma once
#ifdef DEBUG
    #include <iostream>
    #include <cassert>

    // Prints a line of debug output:
    #define DBG(toPrint) std::cout << toPrint << "\n";

    // Prints a line of verbose debug output:
    #ifdef DEBUG_VERBOSE
        #define DBG_V(toPrint) std::cout << toPrint << "\n";
    #else
        #define DBG_V(toPrint)
    #endif

    // Terminates the program if a test condition is not met:
    #define ASSERT(condition) assert(condition);

#else
    // Redefine debug macros as empty statements outside of debug builds:
    #define DBG(toPrint)
    #define DBG_V(toPrint)
    #define ASSERT(condition)
#endif
