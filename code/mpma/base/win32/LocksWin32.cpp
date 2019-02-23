//Luke Lenhart, 2007
//(filename is different to work around a msvc ide bug that prevented compilation)
//See /docs/License.txt for details on how this code may be used.

#include "../Locks.h"
#include "../Memory.h"
#include "evil_windows.h"

// -- mutex setup

namespace MPMA
{

MutexLock::MutexLock()
{
    Data().crit=new2(CRITICAL_SECTION,CRITICAL_SECTION);
    InitializeCriticalSection((CRITICAL_SECTION*)Data().crit);
}

MutexLock::~MutexLock()
{
    if (IsOnlyReference())
    {
        DeleteCriticalSection((CRITICAL_SECTION*)Data().crit);
        CRITICAL_SECTION* theCrit=(CRITICAL_SECTION*)Data().crit;

        delete2(theCrit);
        Data().crit=0;
    }
}


// -- mutex use

TakeMutexLock::TakeMutexLock(MutexLock &critSection, bool takeNow):
    crit(critSection), taken(false)
{
    if (takeNow)
        Take();
}

TakeMutexLock::~TakeMutexLock()
{
    Leave();
}

//Manually takes the lock.
void TakeMutexLock::Take()
{
    if (!taken)
        EnterCriticalSection((CRITICAL_SECTION*)crit.Data().crit);
    taken=true;
}

//Manually releases the lock.
void TakeMutexLock::Leave()
{
    if (taken)
        LeaveCriticalSection((CRITICAL_SECTION*)crit.Data().crit);
    taken=false;
}


// -- BlockingObject

struct BlockObjData
{
    SpinLock lock;
    HANDLE handEvent;

    void SignalAll()
    {
        SetEvent(handEvent);
    }
};

InternalBlockingObjectData::InternalBlockingObjectData()
{
    isSet=false;
    clearCount=0;

    pdata=new2(BlockObjData,BlockObjData);
    BlockObjData *bodata=(BlockObjData*)pdata;

    bodata->handEvent=CreateEvent(0, true, true, 0);
}

InternalBlockingObjectData::~InternalBlockingObjectData()
{
    BlockObjData *bodata=(BlockObjData*)pdata;

    CloseHandle(bodata->handEvent);

    delete2(bodata);
}

//Marks the object as cleared, so nothing is blocking on it.
void BlockingObject::Clear()
{
    BlockObjData *bodata=(BlockObjData*)Data().pdata;

    TakeSpinLock takeLock(bodata->lock);
    ++Data().clearCount;
    Data().isSet=false;
    bodata->SignalAll();
}

//Marks the object as set, so any all Waits will block on it.
void BlockingObject::Set()
{
    BlockObjData *bodata=(BlockObjData*)Data().pdata;

    TakeSpinLock takeLock(bodata->lock);
    Data().isSet=true;

    ResetEvent(bodata->handEvent);
}

//Blocks and waits for as long as the object is set (or if time is non-max, then until that many milliseconds pass).  Returns false only on timeout.
bool BlockingObject::WaitUntilClear(bool setOnReturn, nuint timeToWait)
{
    BlockObjData *bodata=(BlockObjData*)Data().pdata;

    TakeSpinLock takeLock(bodata->lock);
    if (!Data().isSet)
    {
        Data().clearCount=0;
        if (setOnReturn)
        {
            Data().isSet=true;
            ResetEvent(bodata->handEvent);
        }
        return true;
    }
    else if (Data().clearCount>0)
    {
        --Data().clearCount;
        if (setOnReturn)
        {
            Data().isSet=true;
            ResetEvent(bodata->handEvent);
        }
        return true;
    }

    bool retval=true;

    takeLock.Leave();
    if (timeToWait==0xffffffff)
    {
        WaitForSingleObject(bodata->handEvent, INFINITE);
    }
    else
    {
        DWORD ret=WaitForSingleObject(bodata->handEvent, (DWORD)timeToWait);
        retval=ret!=WAIT_TIMEOUT;
    }

    takeLock.Take();
    if (setOnReturn)
    {
        Data().isSet=true;
        ResetEvent(bodata->handEvent);
    }
    --Data().clearCount;

    return retval;
}

};
