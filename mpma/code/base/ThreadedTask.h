//!\file ThreadedTask.h Paralellize a task using threads.
//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

#include "Types.h"
#include "Thread.h"

#ifndef THREADEDTASK_H_INCLUDED
#define THREADEDTASK_H_INCLUDED

namespace MPMA
{
    /*!Executes a function in parallel using multiple threads (1 per cpu core).  The function should take 2 parameters: a nuint for the unique 0-based count index passed to it, and the user-supplied parameter.
    Example: \n
    If you have a function: \n
    inline void Foo(int i, float *param) \n
    { \n
        param[i]+=123.456f; \n
    } \n
    And you wanted to call that function 1000 times total, with i ranging from 0 to 999, you would write: \n
    ExecuteThreadedTask<void(*)(int,float*), Foo)>(1000, myArrayOfFloats);
    */
    template <typename FuncType, FuncType userFunc, typename UserParamType>
    void ExecuteThreadedTask(nuint count, UserParamType userParam);
}

#ifndef THREADEDTASK_INCLUDE_INLINE
    #define THREADEDTASK_INCLUDE_INLINE
    #include "ThreadedTask.cpp"
#endif

#endif //THREADEDTASK_H_INCLUDED
