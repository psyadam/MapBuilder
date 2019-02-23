//Retrieve information about the system
//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

#include "Info.h"
#include "../Setup.h"

namespace MPMA
{
    //the static vars
    nuint SystemInfo::ProcessorCount=2;
    nuint SystemInfo::ProcessorBogomips=3456;
    std::string SystemInfo::ProcessorName="NotPopulated";
    bool SystemInfo::ProcessorHyperthreading=false;
    nuint SystemInfo::MemoryPhysicalTotal=789;
    nuint SystemInfo::MemorySwapTotal=345;
    std::string SystemInfo::OperatingSystemName="NotPopulated";
    bool SystemInfo::SuggestSleepInSpinlock=false;
    
    //platform specific func
    void Internal_InitInfo();
    
    //
    namespace
    {
        class AutoInitInfo
        {
        public:
            //hookup init callbacks
            AutoInitInfo()
            {
                MPMA::Internal_AddInitCallback(Internal_InitInfo,-2000);
            }
        } autoInitInfo;
    }
}

bool mpmaForceReferenceToInfoCPP=false; //work around a problem using MPMA as a static library
