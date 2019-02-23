//Thread creation and management.
//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

#include "Thread.h"
#include "Memory.h"

namespace
{
    nuint threadIdGlobalCounter=1;
    THREAD_LOCAL nuint currentThreadId=0;
}

namespace MPMA
{
    // -- Threadpool class

    //ctor
    ThreadPool::ThreadPool(nuint initialThreads, nuint maxThreads)
    {
        threadLimit=maxThreads;
        threadCount=0;

        //make initial threads
        for (nuint i=0; i<initialThreads; ++i)
            GrowPool();
    }

    //dtor
    ThreadPool::~ThreadPool()
    {
        //tell all threads to die
        {
            TakeSpinLock takeLock(listLock);

            if (allThreads.size()!=availableThreads.size())
                threadReturnBlock.Set();
            else
                threadReturnBlock.Clear();

            for (std::list<ThreadPoolThread*>::iterator i=allThreads.begin(); i!=allThreads.end(); ++i)
            {
                (**i).thread->SetEnding();
                (**i).isEnding=true;
                (**i).block.Clear();
            }
        }

        //wait until all threads are in the available bucket
        do
        {
            threadReturnBlock.WaitUntilClear(true);

            TakeSpinLock takeLock(listLock);
            if (allThreads.size()==availableThreads.size())
                break;
        } while (true);

        //free all threads
        {
            TakeSpinLock takeLock(listLock);
            for (std::list<ThreadPoolThread*>::iterator i=allThreads.begin(); i!=allThreads.end(); ++i)
            {
                delete2((**i).thread);
                (**i).thread=0;
                delete2(*i);
            }
        }
    }

    //attempts to increase the size of the thread pool
    void ThreadPool::GrowPool()
    {
        TakeSpinLock takeLock(listLock);
        if (threadCount>=threadLimit)
            return;

        //make the new one
        ThreadPoolThread *tpt=new2(ThreadPoolThread(),ThreadPoolThread);
        tpt->pool=this;
        tpt->isEnding=false;
        tpt->isAssigned=false;
        tpt->isReadyToRun=false;
        tpt->runFunc=0;
        tpt->thread=new2(Thread(ThreadProc,tpt),Thread);
        availableThreads.push_back(tpt);
        allThreads.push_back(tpt);
        ++threadCount;
    }

    //removes a thread from the pool, growing or blocking if needed until one is available
    ThreadPool::ThreadPoolThread* ThreadPool::GetThreadFromPool()
    {
        TakeSpinLock takeLock(listLock);
        threadReturnBlock.Set();
        while (availableThreads.size()==0)
        {
            //grow it if possible
            if (threadCount<threadLimit)
            {
                takeLock.Leave();
                GrowPool();
                takeLock.Take();
                continue;
            }

            //block and wait for a thread to return to the pool
            takeLock.Leave();
            threadReturnBlock.WaitUntilClear(true);
            takeLock.Take();
        }

        ThreadPoolThread *useThread=availableThreads.front();
        availableThreads.pop_front();
        useThread->isAssigned=true;
        return useThread;
    }

    //returns a thread to the pool
    void ThreadPool::ReturnThreadToPool(ThreadPoolThread *thread)
    {
        TakeSpinLock takeLock(listLock);
        thread->isAssigned=false;
        thread->isReadyToRun=false;
        availableThreads.push_front(thread);
        threadReturnBlock.Clear();
    }

    //static thread proc
    void ThreadPool::ThreadProc(Thread &myThread, ThreadParam param)
    {
        ThreadPoolThread *tpt=(ThreadPoolThread*)param.ptr;
        tpt->block.Set();
        while (!myThread.IsEnding() && !tpt->isEnding)
        {
            //if we are assigned a task, run it
            if (tpt->isAssigned)
            {
                while (!tpt->isReadyToRun)
                    Sleep(0);

                tpt->runFunc(*tpt->thread, tpt->runParam);
                tpt->pool->ReturnThreadToPool(tpt);
            }

            //block until woken
            tpt->block.WaitUntilClear(true);
        }
    }

    //Exectutes a function on a thread in the pool.  The thread returns to the pool when the function returns.  Blocks if needed until there is a thread available.
    void ThreadPool::RunThread(ThreadFunc proc, ThreadParam param)
    {
        ThreadPoolThread *tpt=GetThreadFromPool();
        tpt->runFunc=proc;
        tpt->runParam=param;
        tpt->isReadyToRun=true;
        tpt->block.Clear();
    }

    // --

    //Gets a value that is unique to the calling thread.  Any given thread will always return the same value.
    nuint GetThreadUniqueIdentifier()
    {
        if (currentThreadId==0)
            currentThreadId=AtomicIntAdd((nsint*)&threadIdGlobalCounter, 1);

        return currentThreadId;
    }

}
