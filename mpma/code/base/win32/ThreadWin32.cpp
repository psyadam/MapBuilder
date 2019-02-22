//win32 implementation of threads.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Thread.h"
#include "../Debug.h"
#include "../Memory.h"
#include "../DebugRouter.h"
#include "evil_windows.h"

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
    
            HANDLE threadHandle;
            volatile bool isDieing;
            volatile bool alive;
        };
    
        //thread procedure wrapper
        int WINAPI ThreadProcWrapper(void *ptrParam)
        {
            ThreadInfo &info=*(ThreadInfo*)ptrParam;
    
            //enter main procedure
            info.func(*info.ownerThread,*info.param);
    
            //mark that we're done and end
            info.alive=false;
            CloseHandle(info.threadHandle);
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
        data->threadHandle=0;
        data->isDieing=false;
        data->alive=true;
    
        //make thread
        DWORD yawn;
        internal=data;
        data->threadHandle=CreateThread(0,0,(LPTHREAD_START_ROUTINE)ThreadProcWrapper,data,0,&yawn);
        if (!data->threadHandle)
        {
            ErrorReport()<<"Win32 CreateThread failed... grah.\n";
            delete2(data->param);
            delete2(data);
            data=0;
            internal=0;
            return;
        }
    }
    
    //when this object destructs, we block until the thread terminates (if it has not already done so)
    Thread::~Thread()
    {
        ThreadInfo *data=(ThreadInfo*)internal;
    
        if (data)
        {
            //TODO: a blocking object here would be better than sleep
            data->isDieing=true;
    
            while (data->alive)
            {
                Sleep(0);
            }
    
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

        if (newPriority==THREAD_LOW)
            SetThreadPriority(data->threadHandle, THREAD_PRIORITY_BELOW_NORMAL);
        else if (newPriority==THREAD_HIGH)
            SetThreadPriority(data->threadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
        else //THREAD_NORMAL
            SetThreadPriority(data->threadHandle, THREAD_PRIORITY_NORMAL);
    }
    
    // -- Thread-related helpers
    
    //Causes the current thread to give block for at least time ms
    void Sleep(nuint time)
    {
        ::Sleep((DWORD)time);
    }

}
