//tests debugging functions
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "base/Debug.h"
#include <string>

#ifdef DECLARE_TESTS_CODE
class CheckStackTrace: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;
        
        std::string trace=MPMA::GetCallStack();
        std::cout<<trace;
        if (trace.length()<5) passed=false;
        
        
        if (trace.find("Run")==std::string::npos) passed=false;
        
        return passed;
    }
};
#endif
DECLARE_TEST_CASE(CheckStackTrace)
