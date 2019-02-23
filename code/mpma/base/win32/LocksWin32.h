//Thread Locking Constructs
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include <intrin.h>

namespace MPMA
{
    //Atomically increments an integer
    inline void AtomicIntInc(volatile nuint *pint)
    {
#ifdef _WIN64
		_InterlockedIncrement64((volatile __int64*)pint);
#else //WIN32
		__asm
        {
            mov eax, pint;
            lock inc dword ptr [eax];
        }
#endif
    }

    //Atomically decrements an integer
    inline void AtomicIntDec(volatile nuint *pint)
    {
#ifdef _WIN64
		_InterlockedDecrement64((volatile __int64*)pint);
#else //WIN32
        __asm
        {
            mov eax, pint;
            lock dec dword ptr [eax];
        }
#endif
    }
    
    //Atomically adds one integer to another and returns the value of the original
    inline nsint AtomicIntAdd(volatile nsint *pint, nsint addValue)
    {
#ifdef _WIN64
        return _InterlockedExchangeAdd64((volatile __int64*)pint, addValue);
#else //WIN32
		int rval;
        __asm
        {
            mov eax, addValue;
            mov ebx, pint;
            lock xadd dword ptr [ebx], eax;
            mov rval, eax;
        }
        return rval;
#endif
    }

    //Compares expectedValue with the value at pInt, and if they are the same, sets pInt to newValue and returns true with outResultValue set to newValue.  If they are different then pInt is unaffected, and returns false with outResultValue set to the value that was found at pInt.
    inline bool AtomicCompareExchange(volatile nuint *pInt, nuint expectedValue, nuint newValue, volatile nuint &outResultValue)
    {
#ifdef _WIN64
		nuint rval;
		rval=_InterlockedCompareExchange64((volatile __int64*)pInt, newValue, expectedValue);
		if (rval==expectedValue)
		{
			outResultValue=newValue;
			return true;
		}
		else
		{
			outResultValue=rval;
			return false;
		}
			
#else //WIN32
        nuint rval;
        nuint changed=0;
        __asm
        {
            mov eax, expectedValue;
            mov ebx, pInt;
            mov ecx, newValue;
            lock cmpxchg [ebx], ecx;
            jnz AtomCmpExch_Diff;
            mov eax,ecx;
            inc changed;
            AtomCmpExch_Diff:
            mov rval, eax;
        }
        outResultValue=rval;
        return changed!=0;
#endif
    }

}; //namespace Platform
