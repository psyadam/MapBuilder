//!\file Debug.h Debugging helpers
//written by Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"
#include <string>

namespace MPMA
{

// -- symbol/stack tracing functions --

    //!\brief Returns a string representing the call stack for the current thread.
    //!Note that there is a decent amount of overhead involved in this.
#ifdef DEBUG_CALLSTACK_ENABLED
    std::string GetCallStack();
#else
    inline std::string GetCallStack() { return std::string(); }
#endif


// --  misc  --

    //!Invokes a debugger break if a debugger is attached
#if defined (_WIN32) || defined(_WIN64)
    #define BREAKPOINT __asm int 3;
#else
    #define BREAKPOINT asm ("int $3;");
#endif


} //namespace MPMA

