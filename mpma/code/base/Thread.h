//!\file Thread.h Thread creation and management.
//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "Types.h"
#include "Locks.h"
#include <list>

namespace MPMA
{
    // -- Thread Class

    //!Thread priorities.
    enum ThreadPriority
    {
        THREAD_LOW, //!<Lower than normal thread priority
        THREAD_NORMAL, //!<Normal thread priority
        THREAD_HIGH //!<Higher than normal thread priority
    };

    //!A user-defined parameter passed to the thread procedure.
    union ThreadParam
    {
        //!param as a pointer
        void *ptr;
        //!param as an int
        int val;

        //handy contructors
        inline ThreadParam(): val(0) //!<ctor
            {}
        inline ThreadParam(void *somePtr): val(0) //!<ctor
            { ptr=somePtr; }
        inline ThreadParam(int someInt): ptr(0) //!<ctor
            { val=someInt; }
        inline ThreadParam(const ThreadParam &p)
            { ptr=p.ptr; val=p.val; }
    };

    //!\brief A callback function for the thread's procedure.
    //!This function should monitor the IsEnding() method of the provided Thread class, and terminate if it ever returns true.
    typedef void (*ThreadFunc)(class Thread&, ThreadParam);

    //!Represents a thread.
    class Thread
    {
    public:
        //!Construct a thread and start it.
        Thread(ThreadFunc proc, ThreadParam param);

        //!When this object destructs, we block until the thread terminates (if it has not already done so).
        ~Thread();

        //!Sets the thread to run at a specific priority.
        void SetPriority(ThreadPriority newPriority);

        //!Returns true if the thread is still running
        bool IsRunning() const;

        //!Returns true if the thread has been asked to terminate.  This is meant for use by the thread proc itself.
        bool IsEnding() const;

    protected:
        void SetEnding();

    private:
        //you cannot duplicate a thread
        Thread(const Thread&);
        const Thread& operator=(const Thread&);

        //internal use to store thread state
        void *internal;

        friend class ThreadPool;
    };


    // -- Threadpool class

    //!A pool of pre-created threads.
    class ThreadPool
    {
    public:
        //!ctor
        ThreadPool(nuint initialThreads=1, nuint maxThreads=100);
      //!When this object destructs, we block until all threads are returned to the pool.
        ~ThreadPool();

        //!Exectutes a function on a thread in the pool.  The thread returns to the pool when the function returns.  Blocks if needed until there is a thread available.
        void RunThread(ThreadFunc proc, ThreadParam param);

    private:
        class ThreadPoolThread
        {
        public:
            ThreadPool *pool;
            Thread *thread;
            ThreadFunc runFunc;
            ThreadParam runParam;
            BlockingObject block;
            bool isEnding;
            bool isAssigned;
            bool isReadyToRun;
        };

        void GrowPool();
        ThreadPoolThread* GetThreadFromPool();
        void ReturnThreadToPool(ThreadPoolThread *thread);

        static void ThreadProc(Thread &myThread, ThreadParam param);

        std::list<ThreadPoolThread*> availableThreads;
        std::list<ThreadPoolThread*> allThreads;
        nuint threadLimit;
        nuint threadCount;
        SpinLock listLock;
        BlockingObject threadReturnBlock;

        //you cannot duplicate a threadpool
        ThreadPool(const ThreadPool&);
        const ThreadPool& operator=(const ThreadPool&);
    };


    // -- Thread-related helpers

    //!Causes the current thread to block for at least time (in milliseconds).  0 just forces a yield.
    void Sleep(nuint time);

    //!Gets a value that is unique to the calling thread.  Any given thread will always return the same value.
    nuint GetThreadUniqueIdentifier();
}
