//tests some thread basics work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "base/Thread.h"
using namespace MPMA;

#ifdef DECLARE_TESTS_CODE
class ThreadBasic: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        //test that we can get a thread going at all
        volatile int testInt;
        {
            std::cout<<"Starting a thread... "<<std::endl;
            testInt=0;
            Thread greatThread(UberThread, ThreadParam((void*)&testInt));
            Sleep(100);
            if (testInt==0)
            {
                std::cout<<"It seems the thread never got going.\n";
                return false;
            }

            //will block terminate on destruction here
            std::cout<<"Testing termination..."<<std::endl;
        }
        int compareInt=testInt;
        Sleep(100);
        if (testInt!=compareInt)
        {
            std::cout<<"It seems to have not stopped."<<std::endl;
            passed=false;
        }

        //test thread priorities
        //TODO: soon

        //
        return passed;
    }

    //not-so-intelligent thread that just repeatedly increments an integer
    static void UberThread(Thread &me, ThreadParam param)
    {
        volatile int &theirInt=*(int*)param.ptr;

        while (!me.IsEnding())
        {
            ++theirInt;
            Sleep(5);
        }
    }
};
#endif
DECLARE_TEST_CASE(ThreadBasic)

#ifdef DECLARE_TESTS_CODE
class ThreadPoolBasic: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        //test that we can get a thread going at all
        volatile int testInt;
        {
            MPMA::ThreadPool pool(1,3);
            std::cout<<"Starting 1 thread... "<<std::endl;
            testInt=0;
            pool.RunThread(UberThread, (void*)&testInt);
            Sleep(75);
            if (testInt!=1)
            {
                std::cout<<"It seems the thread never got going.\n";
                return false;
            }
            Sleep(100);

            std::cout<<"Starting more threads... "<<std::endl;
            testInt=0;
            pool.RunThread(UberThread, (void*)&testInt);
            pool.RunThread(UberThread, (void*)&testInt);
            pool.RunThread(UberThread, (void*)&testInt);
            Sleep(50);
            int nowIntVal=testInt;
            if (nowIntVal!=3)
            {
                std::cout<<"It seems not all 3 got going.  int was "<<nowIntVal<<"\n";
                return false;
            }
            Sleep(125);

            std::cout<<"Testing limit... "<<std::endl;
            testInt=0;
            pool.RunThread(UberThread, (void*)&testInt);
            pool.RunThread(UberThread, (void*)&testInt);
            pool.RunThread(UberThread, (void*)&testInt);
            pool.RunThread(UberThread, (void*)&testInt);
            pool.RunThread(UberThread, (void*)&testInt);
            Sleep(20);
            nowIntVal=testInt;
            if (nowIntVal!=2)
            {
                std::cout<<"Expected int to be 2, but it was "<<nowIntVal<<"\n";
                return false;
            }

            //will block terminate on destruction here
            std::cout<<"Testing termination..."<<std::endl;
        }


        //
        return passed;
    }

    //thread that adds then sleeps then substracts from an int
    static void UberThread(Thread &me, ThreadParam param)
    {
        volatile nuint &theirInt=*(nuint*)param.ptr;

        AtomicIntInc(&theirInt);
        Sleep(150);
        AtomicIntDec(&theirInt);
    }
};
#endif
DECLARE_TEST_CASE(ThreadPoolBasic)
