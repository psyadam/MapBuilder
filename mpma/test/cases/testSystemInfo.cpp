//tests that system info is sane
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <string>
#include <iostream>
#include "base/Info.h"

#ifdef DECLARE_TESTS_CODE
class SystemInfoSanityCheck: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        std::cout<<"ProcessorName="<<MPMA::SystemInfo::ProcessorName<<"\n";
        std::cout<<"ProcessorCount="<<MPMA::SystemInfo::ProcessorCount<<"\n";
        std::cout<<"ProcessorHyperthreading="<<MPMA::SystemInfo::ProcessorHyperthreading<<"\n";
        std::cout<<"ProcessorBogomips="<<MPMA::SystemInfo::ProcessorBogomips<<"\n";
        std::cout<<"MemoryPhysicalTotal="<<MPMA::SystemInfo::MemoryPhysicalTotal<<"\n";
        std::cout<<"MemorySwapTotal="<<MPMA::SystemInfo::MemorySwapTotal<<"\n";
        std::cout<<"OperatingSystemName="<<MPMA::SystemInfo::OperatingSystemName<<"\n";
        std::cout<<"SuggestSleepInSpinlock="<<MPMA::SystemInfo::SuggestSleepInSpinlock<<"\n";
        
        if (MPMA::SystemInfo::ProcessorName=="NotPopulated") passed=false;
        if (MPMA::SystemInfo::ProcessorCount==2 && MPMA::SystemInfo::ProcessorBogomips==3456) passed=false;
        if (MPMA::SystemInfo::OperatingSystemName=="NotPopulated") passed=false;
        if (MPMA::SystemInfo::MemoryPhysicalTotal==789 && MPMA::SystemInfo::MemorySwapTotal==345) passed=false;

        return passed;
    }

private:
};
#endif
DECLARE_TEST_CASE(SystemInfoSanityCheck)
