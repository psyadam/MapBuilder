//Debugging helpers
//written by Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Debug.h"
#include "../MiscStuff.h"
#include <execinfo.h>
#include <stdlib.h>
#include <cxxabi.h>

namespace MPMA
{
    //returns a string representing the call stack (with source locations) for the current thread
#ifdef DEBUG_CALLSTACK_ENABLED
    std::string GetCallStack()
    {
        //do the trace
        void *traceList[100];
        int listCount=backtrace(traceList, 100);
        if (listCount==0) return std::string();

        //got all strings
        std::string str;
        char **strBuff=backtrace_symbols(traceList, listCount);
        for (int i=1; i<listCount; ++i)
        {
            //parse out the mangled portion from the filename
            int ind=MISC::IndexOf(strBuff[i], "(");
            if (ind==-1) //odd, just use the string then..
            {
                str+=strBuff[i];
                str+=" (failed to demangle stack entry, parse error)";
            }
            else
            {
                std::string file=strBuff[i];
                std::string symbol=file.substr(ind+1);
                file.resize(ind);
                int closeInd=MISC::IndexOf(symbol, "+");
                if (closeInd!=-1) symbol.resize(closeInd);

                //now demangle it
                int status=0;
                char *orig=(char*)malloc(100);
                size_t origSize=100;
                orig[0]=0;
                char *ret=abi::__cxa_demangle(symbol.c_str(), orig, &origSize, &status);
                if (!ret) //error demangling
                {
                    str+=file;
                    str+=": ";
                    str+=symbol;
                    free(orig);
                }
                else
                {
                    str+=file;
                    str+=": ";
                    str+=ret;
                    free(ret);
                }
            }

            str+="\n";
        }

        free(strBuff);
        return str;
    }
#endif
}
