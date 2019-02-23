//Debugging helpers
//written by Luke Lenhart, 2007-2010
//See /docs/License.txt for details on how this code may be used.

#include "../../Setup.h"
#include "../Debug.h"
#include "../Types.h"
#include "../Locks.h"
#include "../Memory.h"
#include "evil_windows.h"
#include <dbghelp.h>

namespace
{
#ifdef DEBUG_CALLSTACK_ENABLED
    bool symInit=false;
    MPMA::SpinLock *walkLock=0;

    //sets up symbol library for use
    void Win32DbgInitialize()
    {
        if (walkLock==0)
        {
            walkLock=new2(MPMA::SpinLock(),MPMA::SpinLock);
        }

        if (symInit)
            return;

        if (!SymInitialize(GetCurrentProcess(), 0, true))
        {
            OutputDebugString("SymInitialize failed.  Stack walking will be disabled.\n");
            symInit=false;
            return;
        }

        uint32 opt=SymGetOptions();
        opt|=SYMOPT_UNDNAME;
        opt|=SYMOPT_DEFERRED_LOADS;
        SymSetOptions(opt);

        symInit=true;
    }

    //sets up symbol library for use
    void Win32DbgShutdown()
    {
        //TODO: Investigate why SymCleanup corrupts the heap after SymFromAddr is called.  For now we just do not shut this part down, but instead prevent double initialization.
        /*if (symInit)
        {
            symInit=false;

            if (!SymCleanup(GetCurrentProcess()))
                OutputDebugString("SymCleanup failed.\n");
        }*/

        if (walkLock!=0)
        {
            delete2(walkLock);
            walkLock=0;
        }
    }
    
    class AutoWin32DbgInit
    {
    public:
        //hookup init callbacks
        AutoWin32DbgInit()
        {
            MPMA::Internal_AddInitCallback(Win32DbgInitialize,-5000);
            MPMA::Internal_AddShutdownCallback(Win32DbgShutdown,-5000);
        }
    } autoInitDbg;
#endif //DEBUG_CALLSTACK_ENABLED
}

namespace MPMA
{
    //returns a string representing the call stack (with source locations) for the current thread
#ifdef DEBUG_CALLSTACK_ENABLED
    std::string GetCallStack()
    {
        if (!symInit || !walkLock)
        {
            return "Symbols not initialized.";
        }

        TakeSpinLock takeLock(*walkLock);

        HANDLE curProc=GetCurrentProcess();

        //walk the stack
        void *frames[60]={0};
        unsigned short framesCaptured=CaptureStackBackTrace(0, 60, frames, 0);

        //lookup symbols and build the string
        std::string str="";
        for (unsigned short f=1; f<framesCaptured; ++f) //skip first frame, which is this function
        {
            //add what module it's in
            bool wasKernelModule=false;
            IMAGEHLP_MODULE64 module;
            memset(&module, 0, sizeof(module));
            module.SizeOfStruct=sizeof(IMAGEHLP_MODULE64);

            if (SymGetModuleInfo64(curProc, (DWORD64)frames[f], &module))
            {
                str+=module.ModuleName;
                str+="  ";

                if (std::string(module.ModuleName)=="kernel32")
                    wasKernelModule=true;
            }

            if (!wasKernelModule) //we get false information about kernel so don't bother
            {
                //add the function name
                ULONG64 symInfoBuffer[(sizeof(SYMBOL_INFO)+(MAX_SYM_NAME*sizeof(TCHAR))+(sizeof(ULONG64)-1))/sizeof(ULONG64)]={0};
                SYMBOL_INFO *symInfo=(SYMBOL_INFO*)symInfoBuffer;

                symInfo->SizeOfStruct=sizeof(SYMBOL_INFO);
                symInfo->MaxNameLen=MAX_SYM_NAME;

                if (SymFromAddr(curProc, (DWORD64)frames[f], 0, symInfo))
                {
               
                    //add the file and line number
                    IMAGEHLP_LINE64 line;
                    memset(&line, 0, sizeof(IMAGEHLP_LINE64));
                    line.SizeOfStruct=sizeof(IMAGEHLP_LINE64);
                    uint32 disp=0;

                    if (SymGetLineFromAddr64(curProc, (DWORD64)frames[f], (PDWORD)&disp, &line))
                    {
                        str+=symInfo->Name;
                        str+="  ";
                        str+=line.FileName;
                        str+=" line ";

                        char tmp[64];
                        str+=_itoa(line.LineNumber, tmp, 10);
                    }
                }
            }

            str+="\n";
        }

        return str+"\n";
    }
#endif //DEBUG_CALLSTACK_ENABLED
}

bool mpmaForceReferenceToDebugWin32CPP=false; //work around a problem using MPMA as a static library
