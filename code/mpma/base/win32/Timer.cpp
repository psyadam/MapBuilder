//class for measuring time passed with a high degree of precision (sub ms level)
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Timer.h"
#include "evil_windows.h"

namespace MPMA
{

Timer::Timer()
{
    LARGE_INTEGER &lastTime=(LARGE_INTEGER&)lastTime64;
    QueryPerformanceCounter(&lastTime);
}

//Returns how much time has passed (in seconds) since the last call, or since construction if never.
double Timer::Step(bool updateStartTime)
{
    LARGE_INTEGER &lastTime=(LARGE_INTEGER&)lastTime64;

    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    double diff=(double)(curTime.QuadPart-lastTime.QuadPart)/(double)freq.QuadPart;
    if (updateStartTime) lastTime=curTime;
    return diff;
}

}
