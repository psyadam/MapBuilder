//smart rw thread synchronizator.  read-only accesses don't block each other out.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

// -- normal code section
#ifndef LOCKS_INCLUDE_INLINE

#include "Locks.h"
#include "DebugRouter.h"
#include "Debug.h"
#include "Thread.h"
#include "Types.h"
#include <list>
#include <algorithm>

namespace MPMA
{
    // -- RWSleepLock

    //Optionally takes the lock.
    TakeRWSleepLock::TakeRWSleepLock(RWSleepLock &rwsLock, bool writeAccessIfTakenNow, bool takeNow): lock(rwsLock)
    {
        hasRead=false;
        hasWrite=false;

        if (takeNow)
        {
            if (writeAccessIfTakenNow)
                TakeWrite();
            else
                TakeRead();
        }
    }

    //Releases the reader and writer lock, if they are currently taken.
    TakeRWSleepLock::~TakeRWSleepLock()
    {
        LeaveWrite();
        LeaveRead();
    }

    //Takes the writer lock. (If a writer is already taken by this TakeRWSleepLock instance, the call is ignored)
    void TakeRWSleepLock::TakeWrite()
    {
        if (hasWrite)
            return;
        hasWrite=true;

        nuint threadId=GetThreadUniqueIdentifier();

        //see how many writers this thread already has, and add us to the list
        TakeSpinLock takeSpin(lock.Data().threadListLock);
        nuint threadWriterCount=std::count(lock.Data().writerThreadList.begin(), lock.Data().writerThreadList.end(), threadId);
        lock.Data().writerThreadList.insert(threadId);

        //if this thread already has a writer, it's safe to just bump us in too
        if (threadWriterCount>0)
        {
            AtomicIntInc(&lock.Data().writerCount);
            return;
        }

        //see how many readers this thread has
        nuint threadReaderCount=std::count(lock.Data().readerThreadList.begin(), lock.Data().readerThreadList.end(), threadId);
        takeSpin.Leave();

        //try to acquire the writer
        nuint junk=0;
        while (!AtomicCompareExchange(&lock.Data().writerCount, 0, 1, junk))
            Sleep(0);

        //wait for other thread readers to go away
        while (lock.Data().readerCount>threadReaderCount)
            Sleep(0);
    }

    //Releases a write lock. (If a writer is not taken by this TakeRWSleepLock instance, the call is ignored)
    void TakeRWSleepLock::LeaveWrite()
    {
        if (!hasWrite)
            return;
        hasWrite=false;

        nuint threadId=GetThreadUniqueIdentifier();

        //remove us from the list and drop the writer count
        TakeSpinLock takeSpin(lock.Data().threadListLock);
        std::multiset<nuint>::iterator firstInList=lock.Data().writerThreadList.find(threadId);
        lock.Data().writerThreadList.erase(firstInList);
        AtomicIntDec(&lock.Data().writerCount);

    }

    //Takes a reader lock. (If a reader is already taken by this TakeRWSleepLock instance, the call is ignored)
    void TakeRWSleepLock::TakeRead()
    {
        if (hasRead)
            return;
        hasRead=true;

        nuint threadId=GetThreadUniqueIdentifier();

        //see how many readers and writers this thread has, and add us to the reader list
        TakeSpinLock takeSpin(lock.Data().threadListLock);
        nuint threadReaderCount=std::count(lock.Data().readerThreadList.begin(), lock.Data().readerThreadList.end(), threadId);
        nuint threadWriterCount=std::count(lock.Data().writerThreadList.begin(), lock.Data().writerThreadList.end(), threadId);
        lock.Data().readerThreadList.insert(threadId);
        takeSpin.Leave();

        //if we already have a reader or a writer, it's safe to add another
        if (threadReaderCount>0 || threadWriterCount>0)
        {
            AtomicIntInc(&lock.Data().readerCount);
            return;
        }

        //temporarily grab the writer, to prevent a writer lock from grabbing it at the same time we add our reader
        nuint junk=0;
        while (!AtomicCompareExchange(&lock.Data().writerCount, 0, 1, junk))
            Sleep(0);

        //now it's safe to add our reader, then release the writer
        AtomicIntInc(&lock.Data().readerCount);
        AtomicIntDec(&lock.Data().writerCount);
    }

    //Releases a reader lock. (If a reader is not taken by this TakeRWSleepLock instance, the call is ignored)
    void TakeRWSleepLock::LeaveRead()
    {
        if (!hasRead)
            return;
        hasRead=false;

        nuint threadId=GetThreadUniqueIdentifier();

        //remove us from the list and drop the reader count
        TakeSpinLock takeSpin(lock.Data().threadListLock);
        std::multiset<nuint>::iterator firstInList=lock.Data().readerThreadList.find(threadId);
        lock.Data().readerThreadList.erase(firstInList);
        AtomicIntDec(&lock.Data().readerCount);
    }

} //namespace MPMA

//-- end normal code sectino
#else //-- template and inline section

#include "Thread.h"
#include "Info.h"

namespace MPMA
{
    extern void Sleep(nuint time);

// -- Spinlock

inline void TakeSpinLock::Take()
{
    if (taken)
        return;

    nuint yawn;
    while (!AtomicCompareExchange(&locker.Data().taken, 0, 1, yawn))
    {
        locker.Data().collision=true;
        if (SystemInfo::SuggestSleepInSpinlock)
            Sleep(0);
    }

    taken=true;
}

inline void TakeSpinLock::Leave()
{
    if (!taken)
        return;

    bool wasCollision=locker.Data().collision;
    locker.Data().collision=false;
    locker.Data().taken=0;
    if (wasCollision && SystemInfo::SuggestSleepInSpinlock)
        Sleep(0);

    taken=false;
}

} //namespace MPMA

#endif

