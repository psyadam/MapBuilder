//tests the threaded task manager
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <string>
#include <iostream>
#include "base/ThreadedTask.h"
#include "base/Timer.h"

#ifdef DECLARE_TESTS_CODE
volatile nuint grartest;
inline void GreatnessTest(nuint a, nuint b)
{
    grartest+=a+b + a*b;
}

nuint grrtestarr[50000];
inline void GreatArrayTest(nuint i, nuint *arr)
{
    nuint arrrev=arr[50000-i%50000];
    nuint div=arrrev+1;
    if (div==0) div=1;
    arr[i%50000]=i+i*arrrev+(i*i)/(1+i)-arrrev+i/div+7;
}

inline void SuperIndSet(nuint i, nuint *arr)
{
    arr[i]=i;
}

class ThreadedTaskBasic: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        //low edge cases
        bool lowPass=true;
        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint),GreatnessTest>(2, 7);
        if (grartest==0) lowPass=false;

        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint),GreatnessTest>(1, 2);
        if (grartest!=2) lowPass=false;
        
        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint),GreatnessTest>(0, 3);
        if (grartest!=0) lowPass=false;
        
        if (lowPass==false)
        {
            passed=false;
            std::cout<<"failed the low edge cases."<<std::endl;
        }

        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint),GreatnessTest>(20, 0);
        if (grartest==0) passed=false;

        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint),GreatnessTest>(5000, 1);
        if (grartest==0) passed=false;

        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint),GreatnessTest>(100000, 9);
        if (grartest==0) passed=false;

        if (lowPass && !passed)
        {
            std::cout<<"failed the normal cases."<<std::endl;
        }

        //verify every index is hit now for a large array
        nuint *bigarr=new nuint[10000000];
        memset(bigarr,0,sizeof(bigarr));

        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint*),SuperIndSet>(10000000, bigarr);

        bool spewedbiggarr=false;
        int numbigarrwrong=0;
        for (int i=0; i<10000000; ++i)
        {
            if (bigarr[i]!=(nuint)i)
            {
                if (!spewedbiggarr) std::cout<<"index "<<i<<" is wrong, was "<<bigarr[i]<<std::endl;
                spewedbiggarr=true;
                passed=false;
                ++numbigarrwrong;
            }
        }

        if (spewedbiggarr)
        {
            std::cout<<"The bigarr test failed. "<<numbigarrwrong<<" indices had the wrong value."<<std::endl;
        }

        delete[] bigarr;

        if (!passed)
        {
            return false;
        }

        //now lets time the difference
        for (int prefetch=0; prefetch<50000; ++prefetch)
            ++grrtestarr[prefetch];

        const nuint numToTry=80*1000*1000;
        grartest=0;
        MPMA::Timer timer;
        for (nuint i=0; i<numToTry; ++i)
            GreatArrayTest(i, grrtestarr);
        double timeLoop=timer.Step();
        grartest=0;
        MPMA::ExecuteThreadedTask<void(*)(nuint,nuint*),GreatArrayTest>(numToTry, grrtestarr);
        double timeThread=timer.Step();

        std::cout<<"Time looping on a single thread: "<<timeLoop;
        std::cout<<"\nTime using ExecuteThreadedTask:  "<<timeThread<<std::endl;

        return passed;
    }
};
#endif
DECLARE_TEST_CASE(ThreadedTaskBasic)
