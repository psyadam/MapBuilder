//Luke Lenhart, 2001-2008
//See /docs/License.txt for details on how this code may be used.

#include "../Setup.h"
#include "Memory.h"
#include "Debug.h"
#include "DebugRouter.h"
#include "MiscStuff.h"
#include "Vary.h"

using MPMA::VaryString;

#ifdef MEM_TRACK_OLD_FREE
    #include <vector>
    namespace
    {
        std::vector<MPMAMemoryManager::SAlloc> oldFreedAllocs;
    }
#endif

MPMAMemoryManager mpmaMemoryManager;

#ifdef MEMMAN_TRACING //if management is enabled
namespace
{
    class AutoInitMemMan
    {
    public:
        //hookup init callbacks
        AutoInitMemMan()
        {
            MPMA::Internal_AddInitCallback(MemManInitialize,-9500);
            MPMA::Internal_AddShutdownCallback(MemManShutdown,-9500);
        }
    private:
        static void MemManInitialize()
        {
            mpmaMemoryManager.InternalInitialize();
        }
        static void MemManShutdown()
        {
            mpmaMemoryManager.InternalShutdown();
        }
    } autoInitMemMan;

    const uint8 PAD_BYTE=0xA5; // 1010 0101

    //sets the padding around an allocation
    void SetAllocPadding(uint8 *mem, nuint objSize)
    {
        memset(mem, PAD_BYTE, dbgN2_padding);
        memset(mem+dbgN2_padding+objSize, PAD_BYTE, dbgN2_padding);
    }

    //this is a simple spin-lock exclusive to the memory manager, that does not actually use the memory manager itself.
    //it is otherwise identical to the normal spinlock
    class MemSpinLock
    {
    public:
        volatile nuint taken;
        bool collision;
        MemSpinLock(): taken(0), collision(false)
            {}
    } allocDiffLock, syncLock;

    class MemTakeSpinLock
    {
    public:
        MemTakeSpinLock(MemSpinLock &slock): locker(slock)
        {
            nuint yawn;
            while (!MPMA::AtomicCompareExchange(&locker.taken, 0, 1, yawn))
            {
                locker.collision=true;
                if (MPMA::SystemInfo::SuggestSleepInSpinlock)
                    MPMA::Sleep(0);
            }
        }

        ~MemTakeSpinLock()
        {
            bool wasCollision=locker.collision;
            locker.collision=false;
            locker.taken=0;
            if (wasCollision && MPMA::SystemInfo::SuggestSleepInSpinlock)
                MPMA::Sleep(0);
        }

    private:
        MemSpinLock &locker;

        void operator=(const MemTakeSpinLock &notallowed);
    };
};

//checks that an allocations padding has not been corrupted
bool MPMAMemoryManager::VerifyAllocPadding(uint8 *mem, nuint objSize)
{
    bool bad=false;

    //check mem before
    try
    {
        for (int i=0;i<dbgN2_padding;++i)
        {
            if (*(mem+i) != PAD_BYTE)
            {
                ReportLeak("WARNING: Damage to memory BEFORE an allocation.\n");
                bad=true;
                break;
            }
        }
    }
    catch(...)
    {
        ReportLeak("WARNING: CRASH trying to verify the padding BEFORE an allocation.\n");
        bad=true;
    }

    //check mem after
    try
    {
        for (int i=0;i<dbgN2_padding;++i)
        {
            if (*(mem+dbgN2_padding+objSize+i) != PAD_BYTE)
            {
                ReportLeak("WARNING: Damage to memory AFTER an allocation.\n");
                bad=true;
                break;
            }
        }
    }
    catch(...)
    {
        ReportLeak("WARNING: CRASH trying to verify the padding AFTER an allocation.\n");
        bad=true;
    }

    return bad;
}
#endif //tracing


//
MPMAMemoryManager::MPMAMemoryManager()
{
    m_ready=false;
#ifdef MEMMAN_TRACING //if management is enabled
    m_critErrors=false;
    m_head=0;
#endif
}

MPMAMemoryManager::~MPMAMemoryManager()
{
    m_ready=false;
}

//setup initial memman state (start tracking)
void MPMAMemoryManager::InternalInitialize()
{
    m_ready=true;
}

//check for leaks, then shut down memman
void MPMAMemoryManager::InternalShutdown()
{
    //the ErrorReport has already been shut down at this point.. hence we need our own callback
#ifdef MEMMAN_TRACING //if management is enabled
    CheckForPaddingCorruption();
#endif

    m_ready=false;

#ifdef MEMMAN_TRACING //if management is enabled

    //check for critical errors
    if (m_critErrors) //tell them something very bad happened
    {
        ReportLeak("Critical memory management errors occured.\n\n");
    }

    //check if they didn't free memory
    if (m_head)
    {
        SAlloc *cur;

        //lets see how much was leaked total...
        size_t leakedBytes=0;
        int leakedCount=0;
        for (cur=m_head;cur;cur=cur->next)
        {
            if (cur->countsAsLeak)
            {
                leakedBytes+=cur->size;
                leakedCount++;
            }
        }

        if (leakedCount>0)
        {
            ReportLeak("\nTotal memory leaks: "+VaryString((int)leakedBytes)+" bytes in "+VaryString(leakedCount)+" allocation(s).\n");

            //report all leaks
            ReportLeak("\n -- List of memory leaks --\n");
            for (cur=m_head;cur;)
            {
                if (cur->countsAsLeak)
                    ReportLeak(cur->Describe());

                cur=cur->next;
            }
        }
    }

#endif
}

#ifdef MEMMAN_TRACING //if management is enabled

//create a textual description of the allocation
std::string MPMAMemoryManager::SAlloc::Describe()
{
    std::string text;
    if (type==EMAT_One) text+="Object of ";
    else text+="Array of "+VaryString(count)+" objects using ";
    text+=VaryString(size)+" bytes: ";
    text+=name;
    text+="\n";

    text+="  -Allocated in "+VaryString(srcFile)+" line "+VaryString(srcLine)+".\n";
    if (allocStack.length()>0)
        text+="  -call stack of allocation:\n"+allocStack+"\n";

    return text;
}

//Marks an allocation as "intentionally leaked".  It won't be reported on at framework shutdown.
void MPMAMemoryManager::MarkAsIntentionallyLeaked(void *mem)
{
    MemTakeSpinLock autoSync(syncLock);
    for (SAlloc *cur=m_head;cur;cur=cur->next)
    {
        if (cur->objAddr==mem)
        {
            cur->countsAsLeak=false;
            return;
        }
    }

    ReportLeak("MarkAsIntentionallyLeaked: Failed to find the pointer that was passed in.");
}

//
void MPMAMemoryManager::ReportLeak(const std::string &leak)
{
    MPMA::ErrorReport()<<leak;
}

//records allocation of memory
void MPMAMemoryManager::TraceAlloc(EMemAllocType type, void *memory, void *object, nuint objectSize, nsint count, const char *file, int line, const char *name)
{
    if (!memory || !object)
    {
        ReportLeak("Out of memory trying to alloc?  A call to new may have failed...\n");
        ReportLeak("  -call stack:\n");
        ReportLeak(MPMA::GetCallStack());
        return;
    }

    //create allocation data
    SAlloc *newAlloc=new SAlloc;
    if (!newAlloc)
    {
        ReportLeak("Alloc node in MemMan to trace an allocation failed to allocated.  Out of memory...?\n");
        return;
    }

    newAlloc->type=type;
    newAlloc->objAddr=object;
    newAlloc->allocMem=memory;
    newAlloc->size=objectSize;
    newAlloc->count=count;
    newAlloc->srcFile=file;
    newAlloc->srcLine=line;
    newAlloc->name=name;
    newAlloc->countsAsLeak=m_ready;
#ifdef SAVE_ALLOC_CALL_STACK
    newAlloc->allocStack=MISC::StripFirstLine(MPMA::GetCallStack());
#endif

    //pad the allocation
    SetAllocPadding((uint8*)memory, objectSize);

    //add it to front list
    MemTakeSpinLock autoSync(syncLock);
    newAlloc->next=m_head;
    m_head=newAlloc;
}

//records freeing of memory
//returns the actual type of allocation (single or array, or none if invalid)
EMemAllocType MPMAMemoryManager::TraceFree(EMemAllocType type, volatile void *object, char *&outAllocMem, nsint *outObjectSize, nsint *outCount, const char *file, int line)
{
    outAllocMem=0;
    if (outCount) *outCount=0;
    if (outObjectSize) *outObjectSize=0;

    EMemAllocType realType=type;

    //check if they're deleting null memory first
    if (object==0)
    {
        m_critErrors=true;

        ReportLeak("Error: delete being used on a null pointer.\n");
        if (file!=0)
        {
            ReportLeak("  -delete call made in "+VaryString(file)+" line "+VaryString(line)+".\n");
        }
        ReportLeak("  -call stack:\n");
        ReportLeak(MPMA::GetCallStack());

        return EMAT_None;
    }

    //map what they gave us using the differences map
    object=(volatile void*)DeallocDifferenceMap((void*)object, true);

    //
    MemTakeSpinLock autoSync(syncLock);

    SAlloc *cur;
    SAlloc *prev=m_head;

    bool found=false;

    //see if theres even anything allocated
    if (m_head)
    {
        //find it in our list
        for (cur=m_head;cur;cur=cur->next)
        {
            if (cur->objAddr==object) //found it    
            {
                found=true;

                //check that correct free method was used
                if (cur->type!=type)
                {
                    m_critErrors=true;

                    ReportLeak("Error: Wrong delete operator used when freeing memory: ");
                    ReportLeak(cur->Describe());

                    if (cur->type==EMAT_One) ReportLeak("  -Delete array used on memory allocated with new (non-array).\n");
                    else ReportLeak("  -Delete (non-array) used on memory allocated with new array.\n");

                    if (file!=0)
                    {
                        ReportLeak("  -delete call made in "+VaryString(file)+" line "+VaryString(line)+".\n");
                    }

                    ReportLeak("  -call stack:\n");
                    ReportLeak(MPMA::GetCallStack());

                    realType=cur->type;
                }

                //check padding around the allocation
                if (VerifyAllocPadding((uint8*)cur->allocMem, cur->size))
                {
                    m_critErrors=true;
                    ReportLeak(cur->Describe());
                    ReportLeak("  -call stack:\n");
                    ReportLeak(MPMA::GetCallStack());
                }

                //output params
                if (outCount) *outCount=cur->count;
                if (outObjectSize) *outObjectSize=cur->size;
                outAllocMem=(char*)cur->allocMem;

                //if saving old ones, save a copy
                #ifdef MEM_TRACK_OLD_FREE
                    oldFreedAllocs.push_back(*cur);
                    oldFreedAllocs.back().next=0;
                    oldFreedAllocs.back().deleteStack=MISC::StripFirstLine(MPMA::GetCallStack());
                #endif

                //remove from list
                if (cur==m_head)
                {
                    m_head=m_head->next;
                    delete cur;
                    cur=0;
                }
                else
                {
                    prev->next=cur->next;
                    delete cur;
                    cur=0;
                }

                //
                break;
            }

            prev=cur;
        }
    }

    //if it wasn't found, they are deleting bad memory
    if (!found)
    {
        m_critErrors=true;

        ReportLeak("Error: delete used on memory that doesn't exist.\n");
        if (file!=0)
        {
            ReportLeak("  -delete call made in "+VaryString(file)+" line "+VaryString(line)+"\n");
        }

        ReportLeak("  -call stack:\n");
        ReportLeak(MPMA::GetCallStack());

        //check old records
        #ifdef MEM_TRACK_OLD_FREE
        bool oldFound=false;
        for (std::vector<MPMAMemoryManager::SAlloc>::iterator i=oldFreedAllocs.begin(); i!=oldFreedAllocs.end(); ++i)
        {
            if (i->objAddr==object)
            {
                oldFound=true;
                ReportLeak(VaryString("  -matching historic allocation found:\n")+"   >object name: "+i->name+"\n");
                ReportLeak(VaryString("   >object size: ")+VaryString(i->size)+"\n");
                ReportLeak(VaryString("   >object's allocation stack:\n")+i->allocStack+"   >object's delete stack:\n"+i->deleteStack+"\n");
            }
        }
        if (!oldFound)
        {
            ReportLeak("  -No historic allocations found for the same address.  Likely a bad pointer passed.\n");
        }
        #endif

        return EMAT_None;
    }

    return realType;
}

//checks for memory corruption around all allocated memory
void MPMAMemoryManager::CheckForPaddingCorruption()
{
    MemTakeSpinLock autoSync(syncLock);

    for (SAlloc *cur=m_head; cur; cur=cur->next)
    {
        if (VerifyAllocPadding((uint8*)cur->allocMem, cur->size))
        {
            m_critErrors=true;
            std::string error="Memory corruption detected.\n";

            error+=cur->Describe();
            ReportLeak(error);
        }
    }
}

//checks if a pointer is an allocated object
bool MPMAMemoryManager::IsPointerAnObject(void *p)
{
    p=(void*)DeallocDifferenceMap((void*)p, false);

    MemTakeSpinLock autoSync(syncLock);

    for (SAlloc *cur=m_head; cur; cur=cur->next)
    {
        if (cur->objAddr==p)
        {
            return true;
        }
    }

    return false;
}

// -- operator overloads for new and delete

void* operator new(size_t objLen, MPMAMemoryManager *theMan, const char *file, int line, const char *name)
{
    char *mem=new char[objLen+2*dbgN2_padding];
    memset(mem, 0x7f, objLen+2*dbgN2_padding); //fill with crud
    char *obj=mem+dbgN2_padding;
    theMan->TraceAlloc(EMAT_One, mem, obj, objLen, 1, file, line, name);
    MPMAMemoryManager::AllocDifferenceMapStart(obj);
    return obj;
}

void* operator new[](size_t objLen, MPMAMemoryManager *theMan, size_t count, const char *file, int line, const char *name)
{
    char *mem=new char[objLen+2*dbgN2_padding];
    memset(mem, 0x7f, objLen+2*dbgN2_padding); //fill with crud
    char *obj=mem+dbgN2_padding;
    theMan->TraceAlloc(EMAT_Array, mem, obj, objLen, (nuint)count, file, line, name);
    MPMAMemoryManager::AllocDifferenceMapStart(obj);
    return obj;
}

void operator delete(void *obj, MPMAMemoryManager *theMan, const char *file, int line, const char *name)
{
    char *memory=0;
    nsint size=0;
    theMan->TraceFree(EMAT_One, obj, memory, &size, 0, file, line);
    memset(obj, 0x7e, size); //fill with crud
    delete[] memory;
}

void operator delete[](void *obj, MPMAMemoryManager *theMan, size_t count, const char *file, int line, const char *name)
{
    char *memory=0;
    nsint size=0;
    theMan->TraceFree(EMAT_Array, obj, memory, &size, 0, file, line);
    memset(obj, 0x7e, size); //fill with crud
    delete[] memory;
}

// -- allocation difference mapping

namespace
{
    //THREAD_LOCAL void *allocDiffStartValue=0;
    THREAD_LOCAL void *allocDiffStartValues[100];
    THREAD_LOCAL int allocDiffStartValuesInd=0;
};

void MPMAMemoryManager::AllocDifferenceMapStart(void *mem)
{
    if (allocDiffStartValuesInd==100)
    {
        mpmaMemoryManager.ReportLeak("An insane badness just happened in memory difference mapping: more than 100 recursed new's.  Tracking may be wrong now.\n");
        ++allocDiffStartValuesInd;
        return;
    }
    allocDiffStartValues[allocDiffStartValuesInd]=mem;
    ++allocDiffStartValuesInd;
}

void MPMAMemoryManager::AllocDifferenceMapEnd(void *mem)
{
    if (allocDiffStartValuesInd<1)
    {
        mpmaMemoryManager.ReportLeak("An insane badness just happened in memory difference mapping.  Bad index?\n");
    }
    else if (allocDiffStartValuesInd>=100)
    {
        return;
    }

    --allocDiffStartValuesInd;
    void *allocDiffStartValue=allocDiffStartValues[allocDiffStartValuesInd];

    //only record it if it deviates
    if (allocDiffStartValue==mem)
    {
        return;
    }

    SMapAllocDifference diff;
    diff.returned=mem;
    diff.actual=allocDiffStartValue;
    allocDiffStartValue=0;

    MemTakeSpinLock takeLock(allocDiffLock);
    mpmaMemoryManager.allocDifferences.emplace_back(std::move(diff));
}

void* MPMAMemoryManager::DeallocDifferenceMap(void *mem, bool pop)
{
    //if it's in the list, return that item and remove it from the list... else it wasn't a deviation so just return it.
    MemTakeSpinLock takeLock(allocDiffLock);
    for (std::list<SMapAllocDifference>::iterator i=mpmaMemoryManager.allocDifferences.begin(); i!=mpmaMemoryManager.allocDifferences.end(); ++i)
    {
        if (i->returned==mem)
        {
            void *actual=i->actual;
            if (pop)
                mpmaMemoryManager.allocDifferences.erase(i);
            return actual;
        }
    }

    return mem;
}

#endif //MEMMAN_TRACING

bool mpmaForceReferenceToMemoryCPP=false; //work around a problem using MPMA as a static library
