//tests that the timer functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include <ctime>
#include "../UnitTest.h"
#include <iostream>
#include <cmath>
#include "base/Types.h"
#include "base/Timer.h"
#include "base/Thread.h"
using namespace MPMA;

#ifdef DECLARE_TESTS_CODE
class TimerAndSleep: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        //sleep and timer
        {
            std::cout<<"Sleeping for 1200ms..."<<std::endl;
            Timer timer;
            Sleep(1200);
            float timePassed=(float)timer.Step();
            std::cout<<"Timer reported "<<((float)((int)(timePassed*10000))/10)<<"ms of time."<<std::endl;

            if (std::fabs(timePassed-1.2f)>0.015f) //10ms leeway..
            {
                passed=false;
                std::cout<<"BAD: Difference between timer and sleep param was more than 15ms."<<std::endl;
            }
        }

        {
            std::cout<<"Sleeping for 0..."<<std::endl;
            Timer timer;
            Sleep(0);
            float timePassed=(float)timer.Step();
            std::cout<<"Timer reported "<<timePassed<<"ms of time."<<std::endl;

            if (timePassed>0.015f) //10ms leeway..
            {
                passed=false;
                std::cout<<"BAD: Timer reported more than 15ms."<<std::endl;
            }
        }

        //
        return passed;
    }

};
#endif
DECLARE_TEST_CASE(TimerAndSleep)
