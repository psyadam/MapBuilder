//Paralellize a task using threads.
//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

// -- templated and inline code section
#ifdef THREADEDTASK_INCLUDE_INLINE

#include "ThreadedTask.h"
#include "Info.h"
#include "Memory.h"
#include "Locks.h"
#include "Thread.h"

extern MPMA::ThreadPool *internalTaskPool;
extern std::list<MPMA::BlockingObject*> *internalTaskBlocks;
extern MPMA::SpinLock *internalTaskLock;

namespace
{
    //param to worker thread
    template <typename FuncType, FuncType func, typename UserParamType>
    struct TaskWorkerThreadParam
    {
        UserParamType *param;

        volatile nuint &currentCount;
        nuint &maxCount;

        MPMA::BlockingObject *block;
        volatile nuint *threadsAlive;

        inline TaskWorkerThreadParam(UserParamType *userParam, volatile nuint &curCount, nuint &maxCount, MPMA::BlockingObject *blocker, volatile nuint *threadCount):
            param(userParam), currentCount(curCount), maxCount(maxCount), block(blocker), threadsAlive(threadCount)
        {}

    private:
        void operator=(const TaskWorkerThreadParam<FuncType, func, UserParamType> &notAllowed); //disable
    };

    //each worker thread runs this
    template <typename FuncType, FuncType func, typename UserParamType>
    void ExecuteThreadedTask_ThreadProc(MPMA::Thread &thread, MPMA::ThreadParam threadParam)
    {
        TaskWorkerThreadParam<FuncType, func, UserParamType> &runInfo=*(TaskWorkerThreadParam<FuncType, func, UserParamType>*)threadParam.ptr;

        //calc initial chunk size and big-chunk limit
        nuint bigChunkLimit=(runInfo.maxCount*4)/5; //80% in
        uint32 chunkSize=(uint32)(runInfo.maxCount/MPMA::SystemInfo::ProcessorCount)/4; //TODO: use nuint once atomic stuff supports 64-bit
        if (chunkSize==0) chunkSize=1;

        nuint startRange;
        do
        {
            //grab a range of space to work on
            startRange=MPMA::AtomicIntAdd((volatile nsint*)&runInfo.currentCount, chunkSize);
            if (startRange>runInfo.maxCount) break;
            nuint endRange=startRange+chunkSize;
            if (endRange>runInfo.maxCount) endRange=runInfo.maxCount;

            if (startRange>bigChunkLimit) //time to switch to smaller chunks
            {
                chunkSize>>=1;
                chunkSize+=1;
            }

            //run them
            for (nuint i=startRange; i<endRange; ++i)
            {
                func(i, *runInfo.param);
            }

        } while (startRange<runInfo.maxCount);

        MPMA::AtomicIntDec(runInfo.threadsAlive);
        //runInfo.block->Clear(); //TEMP TODO: race condition of ExecuteThreadedTask returning before this but after the dec, causing accessing bad memory
    }
}

namespace MPMA
{
    //implementation of the function
    template <typename FuncType, FuncType userFunc, typename UserParamType>
    void ExecuteThreadedTask(nuint count, UserParamType userParam)
    {
        if (count==0) return;

        //grab a blocking object to use, or add one if needed
        MPMA::BlockingObject *block;
        {
            MPMA::TakeSpinLock takeLock(*internalTaskLock);
            if (internalTaskBlocks->size()>0)
            {
                block=internalTaskBlocks->front();
                internalTaskBlocks->pop_front();
            }
            else
                block=new2(MPMA::BlockingObject(), MPMA::BlockingObject);
        }

        block->Set();

        //divide the tasks among threads
        volatile nuint sharedCount=0;
        nuint threadCount=SystemInfo::ProcessorCount;
        if (threadCount>count) threadCount=count;
        volatile nuint threadsAlive=threadCount;
        TaskWorkerThreadParam<FuncType, userFunc, UserParamType> runInfo(&userParam, sharedCount, count, block, &threadsAlive);
        for (nuint t=0; t<threadCount; ++t)
            internalTaskPool->RunThread(ExecuteThreadedTask_ThreadProc<FuncType, userFunc, UserParamType>, MPMA::ThreadParam(&runInfo));

        //wait until they're done
        while (threadsAlive>0)
        {
            //block->WaitUntilClear(true); //TEMP TODO: race condition of this returning early and causing thread to access bad memory from this stack
            MPMA::Sleep(0); //temporary hack around... TODO! fix this!
        }

        //put the block back
        MPMA::TakeSpinLock takeLock(*internalTaskLock);
        internalTaskBlocks->push_back(block);
    }
}

// -- end templated and inline code section
#else // -- start normal compiled section

#include "../Setup.h"
#include "Info.h"
#include "Memory.h"
#include "Thread.h"

//threadpool to execute tasks on
MPMA::ThreadPool *internalTaskPool=0;
std::list<MPMA::BlockingObject*> *internalTaskBlocks=0;
MPMA::SpinLock *internalTaskLock=0;

//init stuff
namespace
{
    void ThreadedTaskInitialize()
    {
        //start with 0 threads in the pool and allow up to 2x the cpu count in it
        internalTaskPool=new2(MPMA::ThreadPool(0, MPMA::SystemInfo::ProcessorCount*2), MPMA::ThreadPool);
        internalTaskBlocks=new2(std::list<MPMA::BlockingObject*>(), std::list<MPMA::BlockingObject*>);
        internalTaskLock=new2(MPMA::SpinLock(), MPMA::SpinLock);
    }
    void ThreadedTaskShutdown()
    {
        delete2(internalTaskPool);
        internalTaskPool=0;

        for (std::list<MPMA::BlockingObject*>::iterator i=internalTaskBlocks->begin(); i!=internalTaskBlocks->end(); ++i)
            delete2(*i);
        delete2(internalTaskBlocks);
        internalTaskBlocks=0;

        delete2(internalTaskLock);
        internalTaskLock=0;
    }
    
    class AutoInitThreadedTask
    {
    public:
        //hookup init callbacks
        AutoInitThreadedTask()
        {
            MPMA::Internal_AddInitCallback(ThreadedTaskInitialize,-200);
            MPMA::Internal_AddShutdownCallback(ThreadedTaskShutdown,-200);
        }
    } autoInitReports;
}

bool mpmaForceReferenceToThreadedTaskCPP=false; //work around a problem using MPMA as a static library

#endif
