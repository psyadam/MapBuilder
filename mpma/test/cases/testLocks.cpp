//tests that locks work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <vector>
#include "base/Thread.h"
#include "base/Locks.h"
#include <cmath>
using namespace MPMA;

#ifdef DECLARE_TESTS_CODE
class AtomicOps: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        //simple inc, read, and dec
        {
            std::cout<<"Simple AtomicIntInc from 1 thread... "; std::cout.flush();
            nuint someInt=1000;
            AtomicIntInc(&someInt);
            if (someInt!=1001)
            {
                passed=false;
                std::cout<<"FAILED.  Was 1000, expected 1001 after inc, but it was "<<someInt<<std::endl;
            }
            else
                std::cout<<"Passed."<<std::endl;
        }
        {
            std::cout<<"Simple AtomicIntDec from 1 thread... "; std::cout.flush();
            nuint someInt=1000;
            AtomicIntDec(&someInt);
            if (someInt!=999)
            {
                passed=false;
                std::cout<<"FAILED.  Was 1000, expected 999 after dec, but it was "<<someInt<<std::endl;
            }
            else
                std::cout<<"Passed."<<std::endl;
        }
        {
            std::cout<<"Simple AtomicIntAdd from 1 thread... "; std::cout.flush();
            nsint someInt=1000;
            AtomicIntAdd(&someInt,123);
            if (someInt!=1123)
            {
                passed=false;
                std::cout<<"FAILED.  Was 1000, expected 1123 after adding 123, but it was "<<someInt<<std::endl;
            }
            else
                std::cout<<"Passed."<<std::endl;
        }
        {
            bool localPass=true;
            std::cout<<"AtomicCompareExchange(match) from 1 thread... "; std::cout.flush();
            nuint someInt=1000;
            nuint newVal=0;
            bool changed=AtomicCompareExchange(&someInt,1000,900,newVal);
            if (!changed)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  Expected to have the exchange occur."<<std::endl;
            }
            if (someInt!=900)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  pInt was expected to be 900 now, but it was "<<someInt<<std::endl;
            }
            if (newVal!=900)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  Expected new value to be 900, but it was "<<newVal<<std::endl;
            }
            if (localPass)
                std::cout<<"Passed."<<std::endl;
        }
        {
            bool localPass=true;
            std::cout<<"AtomicCompareExchange(mismatch) from 1 thread... "; std::cout.flush();
            nuint someInt=1000;
            nuint newVal=0;
            bool changed=AtomicCompareExchange(&someInt,900,800,newVal);
            if (changed)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  Did not expect to have the exchange occur."<<std::endl;
            }
            if (someInt!=1000)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  pInt was expected to be 1000 now, but it was "<<someInt<<std::endl;
            }
            if (newVal!=1000)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  Expected new value to be 1000, but it was "<<newVal<<std::endl;
            }
            if (localPass)
                std::cout<<"Passed."<<std::endl;
        }
        {
            bool localPass=true;
            std::cout<<"AtomicCompareExchange(reduced match) from 1 thread... "; std::cout.flush();
            nuint someInt=40;
            nuint ignore;
            bool changed=AtomicCompareExchange(&someInt,40,90,ignore);
            if (!changed)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  Expected to have the exchange occur."<<std::endl;
            }
            if (someInt!=90)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  pInt was expected to be 90 now, but it was "<<someInt<<std::endl;
            }
            if (localPass)
                std::cout<<"Passed."<<std::endl;
        }
        {
            bool localPass=true;
            std::cout<<"AtomicCompareExchange(reduced mismatch) from 1 thread... "; std::cout.flush();
            nuint someInt=300;
            nuint ignore;
            bool changed=AtomicCompareExchange(&someInt,700,800,ignore);
            if (changed)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  Did not expect to have the exchange occur."<<std::endl;
            }
            if (someInt!=300)
            {
                passed=false; localPass=false;
                std::cout<<"FAILED.  pInt was expected to be 300 now, but it was "<<someInt<<std::endl;
            }
            if (localPass)
                std::cout<<"Passed."<<std::endl;
        }

        if (!passed) return false;

        //make 20 test threads that all act on the same int with inc and dec
        {
            std::cout<<"AtomicIntInc and AtomicIntDec from 20 threads at once... "; std::cout.flush();
            int testInt=0;
            std::vector<Thread*> threads;
            for (int i=0; i<20; ++i)
                threads.push_back(new2(Thread(UberThreadIncDec, ThreadParam((void*)&testInt)),Thread));
            while (!threads.empty())
            {
                delete2(threads.back());
                threads.pop_back();
            }
    
            //our int should still be 0 after that
            if (testInt!=0)
            {
                passed=false;
                std::cout<<"FAILED: test int was not 0 after all the ops finished\n";
            }
            else
                std::cout<<"Passed.\n";
        }
        
        //make 20 test threads that all act on the same int with cmpxchg
        badness=false;
        {
            std::cout<<"AtomicCompareExchange from 20 threads at once... "; std::cout.flush();
            nuint testInt=0;
            std::vector<Thread*> threads;
            for (int i=0; i<20; ++i)
                threads.push_back(new2(Thread(UberThreadCX, ThreadParam((void*)&testInt)),Thread));
            while (!threads.empty())
            {
                delete2(threads.back());
                threads.pop_back();
            }
    
            //check return from threads
            if (badness)
            {
                passed=false;
                std::cout<<"FAILED.\n";
            }
            else
                std::cout<<"Passed.\n";
        }

        //
        return passed;
    }

    //thread that atomically increments and decrements an integer
    static void UberThreadIncDec(Thread &me, ThreadParam param)
    {
        volatile nuint &theirInt=*(nuint*)param.ptr;
        
        //main test
        for (int i=0; i<500000; ++i)
        {
            AtomicIntDec(&theirInt);
            AtomicIntInc(&theirInt);
            AtomicIntDec(&theirInt);
            AtomicIntInc(&theirInt);
            AtomicIntDec(&theirInt);
            AtomicIntInc(&theirInt);
            AtomicIntDec(&theirInt);
            AtomicIntInc(&theirInt);
            AtomicIntDec(&theirInt);
            AtomicIntInc(&theirInt);
        }
    }
    
    //thread that does a bunch of compare and exchanges
    static void UberThreadCX(Thread &me, ThreadParam param)
    {
        volatile nuint &theirInt=*(nuint*)param.ptr;
        
        int numLocks=0;
        int numRecentFails=0;
        
        //main test
        volatile int i;
        for (i=0; i<2000000; ++i)
        {
            if (badness) return;
            if (i%5<2) //60% of the time do full
            {
                nuint realVal;
                bool ret=AtomicCompareExchange((nuint*)&theirInt,0,1,(nuint&)realVal);
                if (ret) //we got the "lock"
                {
                    //quickly check that these line up
                    if (realVal!=theirInt)
                    {
                        std::cout<<"returned other value was different from our view of that value.\n";
                        badness=true;
                    }
                    
                    numRecentFails=0;
                    ++numLocks;
                    
                    //mess with it's value a little
                    //++theirInt;
                    AtomicIntInc(&theirInt);
                    AtomicIntDec(&theirInt);
                    //--theirInt;
                    AtomicIntInc(&theirInt);
                    
                    //check value
                    nuint test0=theirInt;
                    ++i; --i;
                    nuint test1=theirInt;
                    if (test0!=test1)
                    {
                        std::cout<<"Inconsistancy between test0 and test1 value...\n";
                        badness=true;
                    }
                    
                    if (test0!=2)
                    {
                        std::cout<<"Expected value to still be 2.\n";
                        badness=true;
                    }
                    
                    //mess with, then exchange it back
                    AtomicIntInc(&theirInt);
                    
                    ret=AtomicCompareExchange(&theirInt,3,0,realVal);
                    if (!ret)
                    {
                        std::cout<<"Our restoring it to 0 should have not been contested.\n";
                        badness=true;
                    }
                    else if (realVal!=0)
                    {
                        std::cout<<"After restoration, the returned current value was "<<realVal<<" instead of 0.\n";
                        badness=true;
                    }
                    
                    //waste a few cycles since we blocked the others some
                    ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i; ++i; --i;
                }
                else //didn't get it
                {
                    //actual value better not be what we asked for
                    if (realVal==0)
                    {
                        std::cout<<"actual value was what we asked for, but exchange didn't happen.\n";
                        badness=true;
                    }
                    
                    ++numRecentFails;
                    if (numRecentFails==1000) //let the busier things go on then since we're choked
                    {
                        Sleep(0);
                        numRecentFails=0;
                    }
                    
                    //waste a few cycles
                    ++i; --i; ++i; --i; ++i; --i;
                }
            }
            else //40% of time do reduced
            {
                nuint ignore;
                bool ret=AtomicCompareExchange((nuint*)&theirInt,0,1,ignore);
                if (ret) //we got the "lock"
                {
                    theirInt=2;
                    
                    //waste a few cycles
                    ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i; ++i; --i;
                    
                    if (theirInt!=2)
                    {
                        badness=true;
                        std::cout<<"failed in reduced got lock case."<<std::endl;
                    }
                    
                    theirInt=0;
                }
                else //we didn't
                {
                    //waste a few cycles
                    ++i; --i; ++i; --i; ++i; --i;
                    ++i; --i; ++i; --i;
                }
            }
        }
        
        //
        if (numLocks<1000)
        {
            //maybe the traffic was just too much.. 1 more chance after a rest to let others calm down
            Sleep(500);
            
            nuint realVal;
            bool ret;
            ret=AtomicCompareExchange(&theirInt,0,1,realVal);
            if (!ret) //gosh dern last chance really...
            {
                for (int grr=0; grr<10; ++grr)
                {
                    Sleep(100);
                    ret=AtomicCompareExchange(&theirInt,0,1,realVal);
                    if (ret) break;
                }
            }
            
            if (ret) //release lock that we got
            {
                ret=AtomicCompareExchange(&theirInt,1,0,realVal);
                if (!ret)
                {
                    badness=true;
                    std::cout<<"Failed to restore lock value to 0 in second chance code.\n";
                }
            }
            else //we never got the lock
            {
                badness=true;
                std::cout<<"This thread only got "<<numLocks<<" lock successes of the "<<i<<" attempts.\n";
                std::cout<<"Value of lock: "<<theirInt<<"\n";
            }
        }
    }
    
private:
    volatile static bool badness;
};
volatile bool AtomicOps::badness;
#endif
DECLARE_TEST_CASE(AtomicOps);

// --

#ifdef DECLARE_TESTS_CODE
template <typename TLock, typename TLockTaker, bool doRentrantTest>
class BasicLockTest: public UnitTest
{
private:
    static bool badness;
    static TLock *lock;

public:
    bool Run()
    {
        badness=false;

        lock=new2(TLock,TLock);
        AutoDelete<TLock> delLock(lock);
        
        if (doRentrantTest)
        {
            //twice from same thread
            std::cout<<"twice from same thread... ";
            std::cout.flush();
            {
                TLockTaker takeLock0(*lock);
                {
                    TLockTaker takeLock1(*lock);
                }
            }
            std::cout<<"ok.\n";
        }

        //make 20 test threads that all act on the same int
        std::cout<<"from 20 threads...";
        std::cout.flush();
        nuint testInt=0;
        std::vector<Thread*> threads;
        for (int i=0; i<20; ++i)
            threads.push_back(new2(Thread(UberThread, ThreadParam((void*)&testInt)),Thread));
        while (!threads.empty())
        {
            delete2(threads.back());
            threads.pop_back();
        }

        //our int should still be 0 after that
        if (testInt!=0)
        {
            badness=true;
            std::cout<<"test int was not 0 after all the ops finished\n";
        }
        else if (badness==false)
            std::cout<<"ok.\n";

        //
        return !badness;
    }
    
    //thread that locks and increments and decrements an integer
    static void UberThread(Thread &me, ThreadParam param)
    {
        volatile nuint &theirInt=*(nuint*)param.ptr;
        
        //main test
        for (volatile int i=0; i<70000; ++i)
        {
            if (badness) return;

            //waste a few cycles
            ++i;++i;++i;++i;++i;
            --i;--i;--i;--i;--i;

            //take the lock
            TLockTaker takeLock(*lock);

            //check initial value
            nuint initVal=theirInt;

            //waste a few cycles
            ++i;++i;++i;++i;++i;
            --i;--i;--i;--i;--i;

            //inc the int
            ++theirInt;

            //waste a few cycles
            ++i;++i;++i;++i;++i;
            --i;--i;--i;--i;--i;

            //dec the int
            --theirInt;

            //check that the value is still the same
            if (theirInt!=initVal)
            {
                badness=true;
                std::cout<<"int value changed... but that should never happen since we held the lock.\n";
                return;
            }
        }
    }
};
template <typename TLock, typename TLockTaker, bool doRentrantTest>
bool BasicLockTest<TLock,TLockTaker,doRentrantTest>::badness;
template <typename TLock, typename TLockTaker, bool doRentrantTest>
TLock *BasicLockTest<TLock,TLockTaker,doRentrantTest>::lock;
typedef BasicLockTest<MutexLock,TakeMutexLock,true> MutexLockTest;
typedef BasicLockTest<SpinLock,TakeSpinLock,false> SpinLockTest;
#endif
DECLARE_TEST_CASE(MutexLockTest);
DECLARE_TEST_CASE(SpinLockTest);

// --

#ifdef DECLARE_TESTS_CODE
class LockRW: public UnitTest
{
private:
    static volatile bool badness;
    static RWSleepLock *lock;

public:
    bool Run()
    {
        badness=false;

        lock=new2(RWSleepLock,RWSleepLock);
        AutoDelete<RWSleepLock> delLock(lock);
        
        //same thread
        std::cout<<"three writers from same thread... ";
        std::cout.flush();
        {
            TakeRWSleepLock takeLock0(*lock,true);
            {
                TakeRWSleepLock takeLock1(*lock,true);
                {
                    TakeRWSleepLock takeLock2(*lock,true);
                }
            }
        }
        std::cout<<"ok.\n";
        
        std::cout<<"three readers from same thread... ";
        std::cout.flush();
        {
            TakeRWSleepLock takeLock0(*lock,false);
            {
                TakeRWSleepLock takeLock1(*lock,false);
                {
                    TakeRWSleepLock takeLock2(*lock,false);
                }
            }
        }
        std::cout<<"ok.\n";
        
        std::cout<<"writer then readers from same thread... ";
        std::cout.flush();
        {
            TakeRWSleepLock takeLock0(*lock,true);
            {
                TakeRWSleepLock takeLock1(*lock,false);
                {
                    TakeRWSleepLock takeLock2(*lock,false);
                }
            }
        }
        std::cout<<"ok.\n";
        
        std::cout<<"reader then writers from same thread... ";
        std::cout.flush();
        {
            TakeRWSleepLock takeLock0(*lock,false);
            {
                TakeRWSleepLock takeLock1(*lock,true);
                {
                    TakeRWSleepLock takeLock2(*lock,true);
                }
            }
        }
        std::cout<<"ok.\n";

        //make 20 test threads that all act on the same int
        std::cout<<"from 20 threads... ";
        std::cout.flush();
        nuint testInt=0;
        std::vector<Thread*> threads;
        for (int i=0; i<20; ++i)
            threads.push_back(new2(Thread(UberThread, ThreadParam((void*)&testInt)),Thread));
        while (!threads.empty())
        {
            delete2(threads.back());
            threads.pop_back();
        }

        //our int should still be 0 after that
        if (testInt!=0)
        {
            badness=true;
            std::cout<<"test int was not 0 after all the ops finished\n";
        }
        else
            std::cout<<"ok.\n";

        //
        return !badness;
    }
    
    //thread that locks and increments and decrements an integer
    static void UberThread(Thread &me, ThreadParam param)
    {
        volatile nuint &theirInt=*(nuint*)param.ptr;
        
        //main test
        for (volatile int i=0; i<70000; ++i)
        {
            if (badness) return;

            //if (i%20000==0) std::cout<<i<<"\n"; //temp

            //waste a few cycles
            ++i;++i;++i;++i;++i;
            --i;--i;--i;--i;--i;

            //take the lock
            bool isWriter=(i%3)==0;
            TakeRWSleepLock takeLock(*lock,isWriter);

            //check initial value
            nuint initVal=theirInt;

            //waste a few cycles
            ++i;++i;++i;++i;++i;
            --i;--i;--i;--i;--i;

            //inc the int
            if (isWriter)
            {
                ++theirInt;

                //waste a few cycles
                ++i;++i;++i;++i;++i;
                --i;--i;--i;--i;--i;
            }

            //dec the int
            if (isWriter)
                --theirInt;

            //check that the value is still the same
            if (theirInt!=initVal)
            {
                badness=true;
                std::cout<<"int value changed... but that should never happen since we held the "<<(isWriter?"writer":"reader")<<" lock.\n";
                return;
            }
            
            //every once in a while test re-entranceness
            if (i%20==3)
            {
                TakeRWSleepLock takeReentry(*lock,isWriter);
                
                //waste a few cycles
                ++i;++i;++i;++i;++i;
                --i;--i;--i;--i;--i;
                
                //check that the value is still the same
                if (theirInt!=initVal)
                {
                    badness=true;
                    std::cout<<"int value changed... but that should never happen since we held the lock re-entrantly even.\n";
                    return;
                }
            }
        }
    }
};
volatile bool LockRW::badness;
RWSleepLock *LockRW::lock;
#endif
DECLARE_TEST_CASE(LockRW);

// --

#ifdef DECLARE_TESTS_CODE
class BlockObjectTest: public UnitTest
{
private:
    static bool badness;
    static BlockingObject *block;

public:
    bool Run()
    {
        badness=false;

        block=new2(BlockingObject,BlockingObject);
        AutoDelete<BlockingObject> delBLock(block);
        
        for (int i=1; i<=5; i+=4)
        {
            std::cout<<"set and block then clear, "<<i<<" other thread(s)... "; std::cout.flush();
            volatile nuint testInt=10;
            block->Set();
            
            Thread **threads=new2_array(Thread*,i,Thread*);
            for (int t=0; t<i; ++t)
                threads[t]=new2(Thread(UberThread1, ThreadParam((void*)&testInt)), Thread);

            Sleep(200);
            if (testInt!=10)
            {
                std::cout<<"int changed from 10 to "<<testInt<<", but threads should be blocked."<<std::endl;
                badness=true;
            }
            
            block->Clear();
            Sleep(200);
            if (!badness && testInt!=10+(nuint)i)
            {
                std::cout<<"int did not change to "<<(10+i)<<" after clear. (was "<<testInt<<")"<<std::endl;
                badness=true;
            }
            
            for (int t=0; t<i; ++t)
                delete2(threads[t]);
            delete2_array(threads);
            
            if (badness) break;

            std::cout<<"ok."<<std::endl;
        }
        
        if (badness) return false;
        
        std::cout<<"timeout... "; std::cout.flush();
        for (int i=2; i>=1; --i)
        {
            block->Set();
            
            volatile nuint testInt=500*i;
            Thread thread4(UberThread4, ThreadParam((void*)&testInt));
            Sleep(100);
            
            if (testInt!=500*(nuint)i)
            {
                std::cout<<"test int should not have changed yet."<<std::endl;
                badness=true;
            }
            
            Sleep(i*500);
            if (!badness && testInt!=500*(nuint)i+100)
            {
                std::cout<<"test int should have been "<<(500*i+100)<<" by now."<<std::endl;
                badness=true;
            }
            
            if (badness) break;
        }
        if (!badness) std::cout<<"ok"<<std::endl;
        
        //
        return !badness;
    }
    
    //for set and block then clear
    static void UberThread1(Thread &me, ThreadParam param)
    {
        nuint &testInt=*(nuint*)param.ptr;
        bool ret=block->WaitUntilClear();
        AtomicIntInc(&testInt);
        
        if (ret!=true)
        {
            badness=true;
            std::cout<<"return from block should have been true."<<std::endl;
        }
    }
    
    //for autoclear
    static void UberThread2(Thread &me, ThreadParam param)
    {
        nuint &testInt=*(nuint*)param.ptr;
        block->WaitUntilClear();
        testInt=150;
    }
    
    //for autoclear
    static void UberThread3(Thread &me, ThreadParam param)
    {
        Sleep(100);
        nuint &testInt=*(nuint*)param.ptr;
        block->WaitUntilClear();
        testInt=200;
    }
    
    //for timeout
    static void UberThread4(Thread &me, ThreadParam param)
    {
        nuint &testInt=*(nuint*)param.ptr;
        nuint blockMs=testInt;
        double blockS=blockMs/1000.0;
        
        Timer timer;
        bool ret=block->WaitUntilClear(false, blockMs);
        double timePassed=timer.Step();
        testInt+=100;

        if (ret!=false)
        {
            badness=true;
            std::cout<<"return from block should have been false."<<std::endl;
        }
        
        if (std::fabs(timePassed-blockS)>0.05) //50ms leeway
        {
            badness=true;
            std::cout<<"time passed waiting on timeout wasn't close enough to expected timeout of "<<blockMs<<"ms: "<<(timePassed*1000)<<"ms"<<std::endl;
        }
    }
};
bool BlockObjectTest::badness;
BlockingObject *BlockObjectTest::block=0;
#endif
DECLARE_TEST_CASE(BlockObjectTest);

