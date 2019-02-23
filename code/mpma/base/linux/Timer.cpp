//class for measuring time passed with a high degree of precision (sub ms level)
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Timer.h"
#include <sys/time.h>

namespace MPMA
{

    Timer::Timer()
    {
        gettimeofday(&lastTime,0);
    }

    //Returns how much time has passed (in seconds) since the last call, or since construction if never.
    double Timer::Step(bool updateStartTime)
    {
        timeval curTime;
        gettimeofday(&curTime,0);
        
        double diff=(double)(curTime.tv_sec-lastTime.tv_sec) + (curTime.tv_usec-lastTime.tv_usec)/1000000.0;
        if (updateStartTime) lastTime=curTime;
        return diff;
    }

}
