//!\file Memory.h Handles memory management and tracking/debugging.

//!new3(t) - allocates an object of type t
//!new3_array(t,c) - allocates an array of t's, of length c.
//!new3(t(param1,param2,etc)) - allocates a new t using specific constructor parameters
//!
//!delete3(mem) - frees memory allocated by new3
//!delete3_array(mem) - frees an array allocated by new3_array
//!
//!new, delete, malloc, and free must not be mixed with new3 and delete3.
//!if you have a private destructor, you must declare your class as a friend to (global namespace) MPMAMemoryManager.
//!
//!Leaks will be reported to MPMA's ErrorReport() object.
//!Only memory allocated after init and before shutdown are used for leak tracking purposes, but you can use the memory functions at any time.

//Luke Lenhart, 2001-2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

//cut some harmless noise warnings out from msvc on warning level 4
#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4127) //conditional expression is constant.  example: do { ...etc... } while (false)
#pragma warning(disable:4100) //unreferenced format parameter.  This seems to be a compiler bug... calling a destructor doesn't count as a "reference to" apparently.
#endif


// --

#include <list>
#include <string>
#include <string.h> //for memset
#include "Types.h"

//internal use
enum EMemAllocType
{
    EMAT_None,
    EMAT_One,
    EMAT_Array
};

//!The memory manager.  This is a global singleton: mpmaMemoryManager.  Do not instantiate this yourself.
class MPMAMemoryManager
{
public:
    //returns whether the memory manager is ready to handle requests
    inline bool IsReady() const
        { return m_ready; }

    //Marks an allocation as "intentionally leaked".  It won't be reported on at framework shutdown.
    void MarkAsIntentionallyLeaked(void *mem);

    // -- internal use below

    //used by the allocation scheme, do not call directly - traces an alloc or free
    void TraceAlloc(EMemAllocType type, void *memory, void *object, nuint objectSize, nsint count, const char *file, int line, const char *name);
    EMemAllocType TraceFree(EMemAllocType type, volatile void *object, char *&outAllocMem, nsint *outObjectSize, nsint *outCount, const char *file, int line);

    //used by the allocation scheme, do not call directly
    void InternalInitialize();
    void InternalShutdown();

#ifndef MEMMAN_TRACING // -- if NOT using management --
    //Returns if a specific pointer is an allocated object (always returns true if management is disabled)
    inline bool IsPointerAnObject(void *p)
        { return true; }

    //checks for memory corruption around all allocated memory
    inline void CheckForPaddingCorruption() {}
#endif

#ifdef MEMMAN_TRACING // -- if using management --

    //Returns if a specific pointer is an allocated object (always returns true if management is disabled)
    bool IsPointerAnObject(void *p);

    //checks for memory corruption around all allocated memory
    void CheckForPaddingCorruption();

    struct SAlloc
    {
        EMemAllocType type;
        void *objAddr; //object address
        void *allocMem; //container memory
        nuint size;
        std::string srcFile;
        int srcLine;
        nsint count; //number in array
        std::string allocStack;
        std::string deleteStack;
        std::string name;
        bool countsAsLeak; //true for anything allocated between init and shutdown

        SAlloc *next;

        std::string Describe();
    };

private:
    bool m_critErrors; //did any critical memory errors occur?

    SAlloc *m_head; //first item in linked list of allocations

    //used internally
    bool VerifyAllocPadding(uint8 *mem, nuint objSize);

    //used as part of mapping the difference between the memory we provide for an object and the memory new return to the caller
    struct SMapAllocDifference
    {
        void *actual;
        void *returned;
    };
    std::list<SMapAllocDifference> allocDifferences;

#endif // -- end if using management --

    //calls back to the leak report callbacks
    void ReportLeak(const std::string &leak);

    bool m_ready; //class ready to handle requests

public: //internal use, do not call directly
#ifdef MEMMAN_TRACING // -- if using management --
    //calls an objects destructor
    template <typename T> static void dbgN2_Destruct(T *obj)
    {
        obj->~T();
    }
    template <typename T> static void dbgN2_Destruct_array(T *obj, nsint count)
    {
        for (nsint i=0; i<count; ++i)
        {
            obj->~T();
            ++obj;
        }
    }

    //used as part of tracking a allocation differences
    template <typename T> static T dbgN3_ReturnNew(T obj)
    {
        AllocDifferenceMapEnd(obj);
        return obj;
    }

    static void AllocDifferenceMapStart(void *mem);
    static void AllocDifferenceMapEnd(void *mem);

    static void* DeallocDifferenceMap(void *mem, bool pop);
#endif // -- end if using management --

    MPMAMemoryManager();
    ~MPMAMemoryManager();

};

extern MPMAMemoryManager mpmaMemoryManager;

#ifndef MEMMAN_TRACING // -- if NOT using management --
inline void MPMAMemoryManager::MarkAsIntentionallyLeaked(void *mem) {}
#endif

#ifdef MEMMAN_TRACING // -- if using management --

//bytes on each side of an allocation that are used to check for damage
#define dbgN2_padding 8

//allocators that store tracing information
void* operator new(size_t objLen, MPMAMemoryManager *theMan, const char *file, int line, const char *name);

#define new3(obj)       MPMAMemoryManager::dbgN3_ReturnNew(new (&mpmaMemoryManager, __FILE__, __LINE__, ""#obj) obj)
#define new2(obj, junk) new3(obj)

void* operator new[](size_t objLen, MPMAMemoryManager *theMan, size_t count, const char *file, int line, const char *name);

#define new3_array(obj, count)       MPMAMemoryManager::dbgN3_ReturnNew(new (&mpmaMemoryManager, count, __FILE__, __LINE__, ""#obj) obj[count])
#define new2_array(obj, count, junk) new3_array(obj, count)

//freers that match the above allocators

void operator delete(void *obj, MPMAMemoryManager *theMan, const char *file, int line, const char *name);

void operator delete[](void *obj, MPMAMemoryManager *theMan, size_t count, const char *file, int line, const char *name);

#define delete3(obj) \
do { \
    void *pObj=(void*)(obj); \
    nsint count=0; \
    nsint objSize=0; \
    char *realMem=0; \
    EMemAllocType type=mpmaMemoryManager.TraceFree(EMAT_One, pObj, realMem, &objSize, &count, __FILE__, __LINE__); \
    if (type==EMAT_One) MPMAMemoryManager::dbgN2_Destruct(obj); \
    else if (type==EMAT_Array) MPMAMemoryManager::dbgN2_Destruct_array(obj, count); \
    memset(pObj, 0x7e, objSize); \
    delete[] realMem; \
    *((nuint**)&obj)=(nuint*)-1; \
} while(false)

#define delete2(obj) delete3(obj) 

#define delete3_array(obj) \
do { \
    void *pObj=(void*)(obj); \
    nsint count=0; \
    nsint objSize=0; \
    char *realMem=0; \
    EMemAllocType type=mpmaMemoryManager.TraceFree(EMAT_Array, pObj, realMem, &objSize, &count, __FILE__, __LINE__); \
    if (type==EMAT_One) MPMAMemoryManager::dbgN2_Destruct(obj); \
    else if (type==EMAT_Array) MPMAMemoryManager::dbgN2_Destruct_array(obj, count); \
    memset(pObj, 0x7e, objSize); \
    delete[] realMem; \
    *((nuint**)&obj)=(nuint*)-1; \
} while(false)

#define delete2_array(obj) delete3_array(obj)

#else // -- if NOT using management --

    #define new3(obj) new obj
    #define new2(obj,junk) new obj
    #define new3_array(obj,count) new obj[count]
    #define new2_array(obj,count,junk) new obj[count]

    #define delete3(obj) delete obj
    #define delete2(obj) delete obj
    #define delete3_array(obj) delete[] obj
    #define delete2_array(obj) delete[] obj

#endif // -- end if using management --

// -- custom deleter for shared_ptr and similar use
template <typename T>
class new3_delete
{
public:
    void operator()(T *p)
    {
        delete3(p);
    }
};

template <typename T>
class new3_array_delete
{
public:
    void operator()(T *p)
    {
        delete3_array(p);
    }
};

// -- auto scope pointer freer --

namespace MPMA
{
    //!Automatically free a single object when this loses scope.
    template <typename T>
    class AutoDelete
    {
    public:
        inline AutoDelete(T *inPtr): ptr(inPtr) {}
        inline ~AutoDelete() {if (ptr) delete2(ptr);}
    private:
        T *ptr;
    };

    //!Automatically free an array of objects when this loses scope.
    template <typename T>
    class AutoDeleteArray
    {
    public:
        inline AutoDeleteArray(T *inPtr): ptr(inPtr) {}
        inline ~AutoDeleteArray() {if (ptr) delete2_array(ptr);}
    private:
        T *ptr;
    };
}