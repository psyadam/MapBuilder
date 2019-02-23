//Thread Locking constructs
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

namespace MPMA
{
    //Atomically increments an integer
    inline void AtomicIntInc(volatile nuint *pint)
    {
#ifdef __i386__
        asm( "lock incl (%0);"
            :
            :"r"(pint)
            :"%0", "memory");
#else
        __sync_fetch_and_add(pint, 1);
#endif
    }

    //Atomically decrements an integer
    inline void AtomicIntDec(volatile nuint *pint)
    {
#ifdef __i386__
        asm( "lock decl (%0);"
            :
            :"r"(pint)
            :"%0", "memory");
#else
        __sync_fetch_and_add(pint, -1);
#endif
    }

    //Atomically adds one integer to another and returns the value of the original
    inline nsint AtomicIntAdd(volatile nsint *pint, nsint addValue)
    {
#ifdef __i386__
        int rval;
        asm volatile( "lock xaddl %2, (%1);"
                      "movl %2, %0"
                     :"=g"(rval)
                     :"r"(pint), "r"(addValue)
                     :"%0", "%1", "%2", "memory");
        return rval;
#else
        return __sync_fetch_and_add(pint, addValue);
#endif
    }

    //Compares expectedValue with the value at pInt, and if they are the same, sets pInt to newValue and returns true with outResultValue set to newValue.  If they are different then pInt is unaffected, and returns false with outResultValue set to the value that was found at pInt.
    inline bool AtomicCompareExchange(volatile nuint *pInt, nuint expectedValue, nuint newValue, volatile nuint &outResultValue)
    {
#ifdef __i386__
        volatile nuint changed=0; //why does this need volatile? it fixes the return bug...
        nuint rval;
        asm(
             "lock cmpxchgl %4, (%2);"
             "jnz AtomCmpExch_Diff%=;"
             "movl %4, %%eax;"
             "incl %1;"
             "AtomCmpExch_Diff%=:;"
             "movl %%eax, %0;"
            :"=g"(rval), "=g"(changed)
            :"r"(pInt), "a"(expectedValue), "r"(newValue)
            :"memory", "cc");

        outResultValue=rval;
        return changed!=0;
#else
        nuint ret=__sync_val_compare_and_swap(pInt, expectedValue, newValue);
        if (ret==expectedValue)
        {
            outResultValue=newValue;
            return true;
        }
        else
        {
            outResultValue=ret;
            return false;
        }
#endif
    }

}; //namespace MPMA
