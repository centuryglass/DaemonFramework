/**
 * @file  Debug.h
 *
 * @brief  Provides debugging macros that are removed in release builds.
 */

// Use different colors for the parent, daemon, and others.
#define PROC_COLOR_CODED 1

#pragma once
#ifdef DF_DEBUG
#   include <iostream>
#   include <cassert>
#   include <cstdio>
#   ifdef PROC_COLOR_CODED
#       if DF_IS_DAEMON
#           define PROC_COLOR "\033[31mD: " <<
#       elif DF_IS_PARENT
#           define PROC_COLOR "\033[36mP: " <<
#       else
#           error "Failed to define daemon or parent!"
#           define PROC_COLOR "\033[37m?: " <<
#       endif
#       define PROC_RESET "\033[0m" <<
#   else  
#       define PROC_COLOR
#       define PROC_RESET
#   endif

// Prints a line of debug output:
#   define DF_DBG(toPrint) std::cout << PROC_COLOR toPrint << PROC_RESET "\n";

// Prints a line of verbose debug output:
#   ifdef DF_VERBOSE
#       define DF_DBG_V(toPrint) DF_DBG(toPrint)
#   else
#       define DF_DBG_V(toPrint)
#   endif

// Prints a C-style error message:
#   define DF_PERROR(toPrint) perror(toPrint);

// Terminates the program if a test condition is not met:
#   define DF_ASSERT(condition) assert(condition);

// Redefine debug macros as empty statements outside of debug builds:
#else
#   define DF_DBG(toPrint)
#   define DF_DBG_V(toPrint)
#   define DF_PERROR(toPrint)
#   define DF_ASSERT(condition)
#endif
