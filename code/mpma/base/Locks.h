//!\file Locks.h Thread locking constructs and atomic functions.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "Types.h"
#include "ReferenceCount.h"

#include <set>

#ifndef LOCKS_H_INCLUDED
#define LOCKS_H_INCLUDED

namespace MPMA
{

    //  -- Atomic Operations safe for multiprocessor operations

    //!Atomically increments an integer.
    void AtomicIntInc(volatile nuint *pint);

    //!Atomically decrements an integer.
    void AtomicIntDec(volatile nuint *pint);

    //!Atomically adds one integer to another and returns the value of the original.
    nsint AtomicIntAdd(volatile nsint *pint, nsint addValue);

    //!Compares expectedValue with the value at pInt, and if they are the same, sets pInt to newValue and returns true with outResultValue set to newValue.  If they are different then pInt is unaffected, and returns false with outResultValue set to the value that was found at pInt.
    bool AtomicCompareExchange(volatile nuint *pInt, nuint expectedValue, nuint newValue, volatile nuint &outResultValue);

    //!Compares ptrExpected with the value at ptrToReplace, and if they are the same, sets ptrToReplace to ptrToSet and returns true with outResultValue set to ptrToSet.  If they are different then ptrToReplace is unaffected, and returns false with outResultValue set to the value that was found at ptrToReplace.
    template <typename T> inline bool AtomicCompareExchange(T **ptrToReplace, T *ptrExpected, T *ptrToSet, T *&outResultValue) { return AtomicCompareExchange((nuint*)ptrToReplace, (nuint)ptrExpected, (nuint)ptrToSet, (nuint&)outResultValue); }

    // -- Locking constructs

    //used by MutexLock
    struct InternalMutexLockData
    {
        void *crit;
    };

    //!A (re-entrant safe) lock based on the operating system's mutexes.
    class MutexLock: public ReferenceCountedData<InternalMutexLockData>
    {
    public:
        MutexLock();
        ~MutexLock();

        friend class TakeMutexLock;
    };

    //!Used to lock/unlock a MutexLock automatically using scope.  The lock is released on dustruction.
    class TakeMutexLock
    {
    public:
        //!Optionally takes the lock.
        TakeMutexLock(MutexLock &critSection, bool takeNow=true);
        //!Releases the lock if it is currently taken.
        ~TakeMutexLock();

        //!Manually takes the lock. (If already taken by this TakeMutexLock instance, the call is ignored)
        void Take();
        //!Manually releases the lock. (If not taken by this TakeMutexLock instance, the call is ignored)
        void Leave();

    private:
        MutexLock crit;
        bool taken;

        //you cannot duplicate this
        TakeMutexLock(const TakeMutexLock&);
        const TakeMutexLock& operator=(const TakeMutexLock&);
    };


    //used by SpinLock
    struct InternalSpinLockData
    {
        volatile nuint taken;
        bool collision;

        inline InternalSpinLockData(): taken(0), collision(false)
            {}
    };

    //!A light-weight spin-lock (NOT re-entrant safe from the same thread) that reverts to a sleeplock on single cpu systems.  This is a reference counted object, so all copies of the object still refer to the same lock.
    class SpinLock: public ReferenceCountedData<InternalSpinLockData>
    {
        friend class TakeSpinLock;
    };

    //!Used to lock/unlock a spinlock automatically using scope.  The lock is released on destruction.
    class TakeSpinLock
    {
    public:
        //!Optionally takes the lock.
        inline TakeSpinLock(SpinLock &slock, bool takeNow=true): locker(slock), taken(false)
            { if (takeNow) Take(); }
        //!Releases the lock if it is currently taken.
        inline ~TakeSpinLock()
            { Leave(); }

        //!Manually takes the lock. (If already taken by this TakeSpinLock instance, the call is ignored)
        inline void Take();
        //!Manually releases the lock. (If not taken by this TakeSpinLock instance, the call is ignored)
        inline void Leave();

    private:
        SpinLock locker;
        bool taken;

        //you cannot duplicate this
        TakeSpinLock(const TakeSpinLock&);
        const TakeSpinLock& operator=(const TakeSpinLock&);
    };

    //used by RWSleepLock
    struct InternalRWSleepLockData
    {
        volatile nuint readerCount;
        volatile nuint writerCount;

        std::multiset<nuint> readerThreadList;
        std::multiset<nuint> writerThreadList;
        SpinLock threadListLock;

        inline InternalRWSleepLockData(): readerCount(0), writerCount(0)
            {}
    };

    //!\brief ReaderWriter sleep-lock (re-entrant safe).
    //!Read locks don't block each other, but Write locks are exclusive to all other locks.  This is a reference counted object, so all copies of the object still refer to the same lock.
    class RWSleepLock: public ReferenceCountedData<InternalRWSleepLockData>
    {
    public:
        friend class TakeRWSleepLock;
    };

    //!Used to lock/unlock a RWSleepLock automatically using scope.  The lock is released on dustruction.
    class TakeRWSleepLock
    {
    public:
        //!Optionally takes the lock.
        TakeRWSleepLock(RWSleepLock &rwsLock, bool writeAccessIfTakenNow=true, bool takeNow=true);
        //!Releases the reader and writer lock, if they are currently taken.
        ~TakeRWSleepLock();

        //!Takes the writer lock. (If a writer is already taken by this TakeRWSleepLock instance, the call is ignored)
        void TakeWrite();
        //!Releases a write lock. (If a writer is not taken by this TakeRWSleepLock instance, the call is ignored)
        void LeaveWrite();

        //!Takes a reader lock. (If a reader is already taken by this TakeRWSleepLock instance, the call is ignored)
        void TakeRead();
        //!Releases a reader lock. (If a reader is not taken by this TakeRWSleepLock instance, the call is ignored)
        void LeaveRead();

    private:
        RWSleepLock lock;

        bool hasRead, hasWrite;

        //you cannot duplicate this
        TakeRWSleepLock(const TakeRWSleepLock&);
        const TakeRWSleepLock& operator=(const TakeRWSleepLock&);
    };

    // -- Signal-based blocking constructs

    struct InternalBlockingObjectData
    {
        InternalBlockingObjectData();
        ~InternalBlockingObjectData();

        volatile int clearCount;
        volatile bool isSet;
        void *pdata; //platform-specific container
    };

    //!An object that can be used to block a thread until another thread signals it to resume.  This is a reference counted object, so all copies of the object still refer to the same lock.
    class BlockingObject: public ReferenceCountedData<InternalBlockingObjectData>
    {
    public:
        //!Marks the object as cleared, so nothing is blocking on it.
        void Clear();

        //!Marks the object as set, so BlockUntilClear will block on it.
        void Set();

        //!Blocks and waits for as long as the object is set (or if time is non-max, then until that many milliseconds pass).  Returns false only on timeout.  If setOnReturn is true, the object will be Set() when the function returns.  Only 1 thread should block on this, though any number may set or clear it.
        bool WaitUntilClear(bool setOnReturn=false, nuint timeToWait=0xffffffff);
    };

} //namespace MPMA

//include the templated and inline code
#ifndef LOCKS_INCLUDE_INLINE
    #define LOCKS_INCLUDE_INLINE
    #include "Locks.cpp"
#endif

//include the platform-specific headers
#if defined(_WIN32) || defined(_WIN64)
    #include "win32/LocksWin32.h"
#else
    #include "linux/LocksLin32.h"
#endif

#endif //LOCKS_H_INCLUDED
