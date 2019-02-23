//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Locks.h"
#include <pthread.h>
#include "../Memory.h"
#include <errno.h>
#include <time.h>

#include <iostream> //debug

// -- mutex setup

namespace MPMA
{

MutexLock::MutexLock()
{
    pthread_mutexattr_t mutexattr;
    int ret=pthread_mutexattr_init(&mutexattr);
    if (ret!=0)
        std::cout<<"pthread_mutexattr_init returned "<<ret<<"\n";

    ret=pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    if (ret!=0)
        std::cout<<"pthread_mutexattr_settype returned "<<ret<<"\n";

    pthread_mutex_t *m;
    m=new2(pthread_mutex_t, pthread_mutex_t);

    ret=pthread_mutex_init(m, &mutexattr);
    if (ret!=0)
        std::cout<<"pthread_mutex_init returned "<<ret<<"\n";

    ret=pthread_mutexattr_destroy(&mutexattr);
    if (ret!=0)
        std::cout<<"pthread_mutexattr_destroy returned "<<ret<<"\n";

    Data().crit=m;
}

MutexLock::~MutexLock()
{
    if (IsOnlyReference())
    {
        pthread_mutex_t* theCrit=(pthread_mutex_t*)Data().crit;
        pthread_mutex_destroy(theCrit);

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
    {
        int ret=pthread_mutex_lock((pthread_mutex_t*)crit.Data().crit);
        if (ret!=0)
            std::cout<<"pthread_mutex_lock failed with "<<ret<<"\n";
    }
    taken=true;
}

//Manually releases the lock.
void TakeMutexLock::Leave()
{
    if (taken)
    {
        int ret=pthread_mutex_unlock((pthread_mutex_t*)crit.Data().crit);
        if (ret!=0)
            std::cout<<"pthread_mutex_unlock failed with "<<ret<<"\n";
    }
    taken=false;
}


// -- BlockingObject

struct BlockObjData
{
    pthread_cond_t cond;
    pthread_mutex_t condlock;
};

InternalBlockingObjectData::InternalBlockingObjectData()
{
    isSet=false;
    clearCount=0;
    pdata=new2(BlockObjData(),BlockObjData);
    BlockObjData *bodata=(BlockObjData*)pdata;

    pthread_cond_init(&bodata->cond, 0);
    pthread_mutex_init(&bodata->condlock, 0);
}

InternalBlockingObjectData::~InternalBlockingObjectData()
{
    BlockObjData *bodata=(BlockObjData*)pdata;
    pthread_mutex_destroy(&bodata->condlock);
    pthread_cond_destroy(&bodata->cond);

    delete2(bodata);
}

//Marks the object as cleared, so nothing is blocking on it.
void BlockingObject::Clear()
{
    BlockObjData *bodata=(BlockObjData*)Data().pdata;

    pthread_mutex_lock(&bodata->condlock);
    ++Data().clearCount;
    pthread_cond_broadcast(&bodata->cond);
    Data().isSet=false;
    pthread_mutex_unlock(&bodata->condlock);
}

//Marks the object as set, so any all Waits will block on it.
void BlockingObject::Set()
{
    BlockObjData *bodata=(BlockObjData*)Data().pdata;

    pthread_mutex_lock(&bodata->condlock);
    Data().isSet=true;
    pthread_mutex_unlock(&bodata->condlock);
}

//Blocks and waits for as long as the object is set (or if time is non-max, then until that many milliseconds pass).  Returns false only on timeout.
bool BlockingObject::WaitUntilClear(bool setOnReturn, nuint timeToWait)
{
    BlockObjData *bodata=(BlockObjData*)Data().pdata;

    pthread_mutex_lock(&bodata->condlock);
    if (!Data().isSet)
    {
        Data().clearCount=0;
        if (setOnReturn)
            Data().isSet=true;
        pthread_mutex_unlock(&bodata->condlock);
        return true;
    }
    else if (Data().clearCount>0)
    {
        --Data().clearCount;
        if (setOnReturn)
            Data().isSet=true;
        pthread_mutex_unlock(&bodata->condlock);
        return true;
    }

    if (timeToWait==0xffffffff)
    {
        pthread_cond_wait(&bodata->cond, &bodata->condlock);
        if (setOnReturn) Data().isSet=true;
        --Data().clearCount;
        pthread_mutex_unlock(&bodata->condlock); //it re-takes it on return
        return true;
    }
    else
    {
        timespec timeout;
        clock_gettime(CLOCK_REALTIME,&timeout);
        timeout.tv_sec+=timeToWait/1000;
        timeout.tv_nsec+=(timeToWait%1000)*1000*1000;
        if (timeout.tv_nsec>1000*1000*1000)
        {
            timeout.tv_nsec-=1000*1000*1000;
            ++timeout.tv_sec;
        }
        int ret=pthread_cond_timedwait(&bodata->cond, &bodata->condlock, &timeout);
        if (setOnReturn)
            Data().isSet=true;
        --Data().clearCount;
        pthread_mutex_unlock(&bodata->condlock); //it re-takes it on return
        return ret!=ETIMEDOUT;
    }
}

} //namespace MPMA
