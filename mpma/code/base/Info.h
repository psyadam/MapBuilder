//!\file Info.h Retrieves information about the system.
//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

#pragma once
#include <string>
#include "Types.h"

namespace MPMA
{
    //!\brief Information about the system.  These are readable anytime after init.
    //!Values which are not possible to obtain will be populated with sane defaults.
    struct SystemInfo
    {
        //overall cpu facts
        static nuint ProcessorCount; //!<Number of logical CPUs
        static std::string ProcessorName; //!<Name of the CPU
        static bool ProcessorHyperthreading; //!<Whether the CPU has hyperthreading enabled

        //per-cpu facts
        static nuint ProcessorBogomips; //!<Very rough speed approximation of one CPU.
        
        //mem info (in MB)
        static nuint MemoryPhysicalTotal; //!<The amount of physical memory the machine has.
        static nuint MemorySwapTotal; //!<The amount of swap memory the machine has.

        //os info
        static std::string OperatingSystemName; //!<Name of the operating system.

        //heuristacal information
        static bool SuggestSleepInSpinlock; //!<Heuristic: Suggestion of whether to yield in a spinlock.
    };
}
