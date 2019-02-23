//Luke Lenhart (2007)
//(filename is different to work around a msvc ide bug that prevented compilation)
//See /docs/License.txt for details on how this code may be used.

#include "evil_windows.h"
#include <intrin.h>
#include "../Info.h"
#include "../MiscStuff.h"

namespace MPMA
{
    void Internal_InitInfo()
    {
        // -- Pull cpu info from the registry
        HKEY key;
        long ret=RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &key);
        if (ret==ERROR_SUCCESS)
        {
            char cpuName[2048];
            nuint cpuNameLen=2048;
            ret=RegQueryValueEx(key, "ProcessorNameString", 0, 0, (uint8*)cpuName, (LPDWORD)&cpuNameLen);
            if (ret==ERROR_SUCCESS)
            {
                SystemInfo::ProcessorName=MISC::StripPadding(cpuName);
            }

            nuint mhz;
            nuint mhzLen=sizeof(nuint);
            ret=RegQueryValueEx(key, "~MHz", 0, 0, (uint8*)&mhz, (LPDWORD)&mhzLen);
            if (ret==ERROR_SUCCESS)
            {
                //unfortunately windows only provides mhz, not bogomips, so we will use an inaccurate and crude estimate based on mhz for now
                SystemInfo::ProcessorBogomips=2*mhz;
            }

            RegCloseKey(key);
        }

        ret=RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor", 0, KEY_READ, &key);
        if (ret==ERROR_SUCCESS)
        {
            nuint numSubKeys=0;
            ret=RegQueryInfoKey(key, 0, 0, 0, (LPDWORD)&numSubKeys, 0, 0, 0, 0, 0, 0, 0);
            if (ret==ERROR_SUCCESS)
            {
                if (numSubKeys>0)
                {
                    SystemInfo::ProcessorCount=numSubKeys;
                }
            }

            RegCloseKey(key);
        }

        //call cpuid to get hyperthreading information
        if (MISC::MakeLower(SystemInfo::ProcessorName).find("intel")!=std::string::npos)
        {
            int regs[4]={0};
            __cpuid(regs, 0);
            int maxCpuIdFunc=regs[0];
            if (maxCpuIdFunc>=1)
            {
                __cpuid(regs, 1);
                if (regs[3]&(1<<28)) //indicates HT is supported, not neccesarily enabled
                {
                    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION plpi=0;
                    int procCount=0;
                    int coreCount=0;
                    DWORD requiredLength=0;
                    GetLogicalProcessorInformation(plpi, &requiredLength);
                    if (requiredLength>0)
                    {
                        std::vector<char> rawBytes;
                        rawBytes.resize(requiredLength);
                        plpi=(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)&rawBytes[0];

                        if (GetLogicalProcessorInformation(plpi, &requiredLength))
                        {
                            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION cur=plpi;
                            while (cur<plpi+requiredLength/sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION))
                            {
                                if (cur->Relationship==RelationProcessorCore)
                                {
                                    ++coreCount;
                                    procCount+=(int)MISC::CountBits(cur->ProcessorMask);
                                }

                                ++cur;
                            }
                        }
                    }

                    if (procCount>coreCount)
                        SystemInfo::ProcessorHyperthreading=true;
                }
            }
        }

        // -- Memory information we'll get from an api call

        MEMORYSTATUSEX meminfo;
        meminfo.dwLength=sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&meminfo))
        {
            SystemInfo::MemoryPhysicalTotal=(nuint)(meminfo.ullTotalPhys/(1024*1024));
            SystemInfo::MemorySwapTotal=(nuint)(meminfo.ullTotalPageFile/(1024*1024));

            if (SystemInfo::MemoryPhysicalTotal<SystemInfo::MemorySwapTotal) //should always be the case.. but just in case
            {
                SystemInfo::MemorySwapTotal-=SystemInfo::MemoryPhysicalTotal;
            }
        }

        // -- Figure out the os version from another api call

        OSVERSIONINFO ver;
        ver.dwOSVersionInfoSize=sizeof(ver);
        if (GetVersionEx(&ver))
        {
            int maj=ver.dwMajorVersion;
            int min=ver.dwMinorVersion;

            if (maj<=3) SystemInfo::OperatingSystemName="Ancient Windows";
            else if (maj==4) //nt, 95, 98 era
            {
                if (min==0)
                {
                    if (ver.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) SystemInfo::OperatingSystemName="Windows 95";
                    else SystemInfo::OperatingSystemName="Windows NT4";
                }
                else if (min==10) SystemInfo::OperatingSystemName="Windows 98";
                else if (min==90) SystemInfo::OperatingSystemName="Windows ME";
                else SystemInfo::OperatingSystemName="Unknown (Windows 95 era)";
            }
            else if (maj==5) //2k, xp, 2k3 era
            {
                if (min==0) SystemInfo::OperatingSystemName="Windows 2000";
                else if (min==1) SystemInfo::OperatingSystemName="Windows XP";
                else if (min==2) SystemInfo::OperatingSystemName="Windows 2003";
                else SystemInfo::OperatingSystemName="Unknown (Windows XP era)";
            }
            else if (maj==6) //vista era
            {
                if (min==0) SystemInfo::OperatingSystemName="Windows Vista";
                else if (min==1) SystemInfo::OperatingSystemName="Windows 7";
                else SystemInfo::OperatingSystemName="Unknown (Windows 7 era)";
            }
            else
                SystemInfo::OperatingSystemName="Unknown (post-windows 7 era)";
        }

        // -- Populate heuristic suggestions
        if (SystemInfo::ProcessorCount>1) SystemInfo::SuggestSleepInSpinlock=false;
        else SystemInfo::SuggestSleepInSpinlock=true;
    }
}
