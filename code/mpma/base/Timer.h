//!\file Timer.h A class for measuring time passed with a high degree of precision (sub ms level).
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    //windows.h drags too much in and causes conflicts, so moved to .cpp
    #include "Types.h"
#else
    #include <sys/time.h>
#endif

namespace MPMA
{
    //!A timer.
    class Timer
    {
    public:
        Timer();
        
        //!Returns how much time has passed (in seconds) since the last call, or since construction if never.
        double Step(bool updateStartTime=true);
        
    private:
        #if defined(_WIN32) || defined(_WIN64)
            uint64 lastTime64;
        #else
            timeval lastTime;
        #endif
    
    };
}

