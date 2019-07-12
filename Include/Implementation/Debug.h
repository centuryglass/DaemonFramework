/**
 * @file  Debug.h
 *
 * @brief  Provides debugging macros that are removed in release builds.
 */

#pragma once
#ifdef DF_DEBUG
    #include <iostream>
    #include <cassert>

    // Prints a line of debug output:
    #define DF_DBG(toPrint) std::cout << toPrint << "\n";

    // Prints a line of verbose debug output:
    #ifdef DF_VERBOSE
        #define DF_DBG_V(toPrint) std::cout << toPrint << "\n";
    #else
        #define DF_DBG_V(toPrint)
    #endif

    // Terminates the program if a test condition is not met:
    #define DF_ASSERT(condition) assert(condition);

#else
    // Redefine debug macros as empty statements outside of debug builds:
    #define DF_DBG(toPrint)
    #define DF_DBG_V(toPrint)
    #define DF_ASSERT(condition)
#endif
