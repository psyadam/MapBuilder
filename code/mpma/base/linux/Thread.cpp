//linux implementation of threads.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Thread.h"
#include "../Memory.h"
#include <pthread.h>
#include <unistd.h>

namespace MPMA
{
    // -- Thread Class

    //
    namespace
    {
        //info given to the thread procedure wrapper
        struct ThreadInfo
        {
            Thread *ownerThread;
            ThreadFunc func;
            ThreadParam *param;

            pthread_t thread;
            bool isDieing;
            bool alive;
            int defaultPri;
        };

        //thread procedure wrapper
        void* ThreadProcWrapper(void *ptrParam)
        {
            ThreadInfo &info=*(ThreadInfo*)ptrParam;

            //enter main procedure
            info.func(*info.ownerThread,*info.param);

            info.alive=false;
            return 0;
        }
    }

    //

    //construct a thread and optionally start it now
    Thread::Thread(ThreadFunc proc, ThreadParam param)
    {
        internal=0;

        //make info about thread
        ThreadInfo *data=new2(ThreadInfo, ThreadInfo);
        data->ownerThread=this;
        data->param=new2(ThreadParam(param),ThreadParam);
        data->func=proc;
        data->thread=0;
        data->isDieing=false;
        data->alive=true;
        data->defaultPri=0;
        internal=data;

        //make it
        pthread_create(&data->thread, 0, ThreadProcWrapper, data);

        //store pri for later
        int policy=0;
        sched_param threadParam;
        if (pthread_getschedparam(data->thread, &policy, &threadParam))
        {
            data->defaultPri=threadParam.sched_priority;
        }
    }

    //when this object destructs, we block until the thread terminates (if it has not already done so)
    Thread::~Thread()
    {
        ThreadInfo *data=(ThreadInfo*)internal;

        if (data)
        {
            data->isDieing=true;

            pthread_join(data->thread,0);

            delete2(data->param);
            delete2(data);
            data=0;
        }
    }

    //returns true if the thread is still running
    bool Thread::IsRunning() const
    {
        ThreadInfo *data=(ThreadInfo*)internal;
        if (!data) return false;
        return data->alive;
    }

    //returns true if the thread has been asked to terminate
    bool Thread::IsEnding() const
    {
        ThreadInfo *data=(ThreadInfo*)internal;
        if (!data) return true;
        return data->isDieing;
    }

    //mark the thread as ending
    void Thread::SetEnding()
    {
        ThreadInfo *data=(ThreadInfo*)internal;
        if (!data) return;
        data->isDieing=true;
    }

    //Sets the thread to run at a specific priority.
    void Thread::SetPriority(ThreadPriority newPriority)
    {
        ThreadInfo *data=(ThreadInfo*)internal;
        if (!data) return;

        int policy=0;
        sched_param param;
        if(pthread_getschedparam(data->thread, &policy, &param))
        {
            int newPri;
            if (newPriority==THREAD_LOW)
            {
                int lowPri=sched_get_priority_min(policy);
                newPri=(lowPri+data->defaultPri)/2;
            }
            else if (newPriority==THREAD_HIGH)
            {
                int highPri=sched_get_priority_max(policy);
                newPri=(1+highPri+data->defaultPri)/2;
            }
            else //THREAD_NORMAL
                newPri=data->defaultPri;

            param.sched_priority=newPri;
            pthread_setschedparam(data->thread, policy, &param);
        }
    }

    // -- Thread-related helpers

    //Causes the current thread to give block for at least time (in ms)
    void Sleep(nuint time)
    {
        if (time==0)
            sched_yield();
        else
        {
            nuint seconds=time/1000;
            nuint ms=time%1000;

            for (nuint i=0; i<seconds*2; ++i)
                usleep(500*1000);
            usleep(ms*1000);
        }
    }

}
