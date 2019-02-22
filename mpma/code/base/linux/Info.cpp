//Luke Lenhart (2007)
//See /docs/License.txt for details on how this code may be used.

#include "../MiscStuff.h"
#include "../Info.h"
#include "../Vary.h"

#include <map>

namespace MPMA
{
    void Internal_InitInfo()
    {
        // -- Populate cpu info
        std::string cpuinfo=MISC::ReadFile("/proc/cpuinfo");
        if (cpuinfo.empty()) //proc not mounted...?
        {
            SystemInfo::ProcessorName="Could not read /proc/cpuinfo";
        }
        else
        {
            nuint cpuCount=0;
            nuint siblingCount=0;
            std::string cpuName="";
            unsigned cpuBogomips=0;
            nuint curPhysicalId=0;

            std::map<nuint, std::map<std::string, bool>> collectedFeature;

            //break it up into lines
            std::vector<std::string> cpulines;
            MISC::ExplodeString(cpuinfo,cpulines,"\n");

            //parse each line
            for (std::vector<std::string>::iterator line=cpulines.begin(); line!=cpulines.end(); ++line)
            {
                //split it over the colon
                std::vector<std::string> lineparts;
                MISC::ExplodeString(*line,lineparts,":");
                if (lineparts.size()==2)
                {
                    std::string key=MISC::StripPadding(lineparts[0]);
                    std::string val=MISC::StripPadding(lineparts[1]);

                    if (key=="physical id")
                        curPhysicalId=(int)Vary(val);
                    else
                    {
                        if (collectedFeature[curPhysicalId].find(key)==collectedFeature[curPhysicalId].end())
                        {
                            collectedFeature[curPhysicalId][key]=true;

                            if (key=="model name")
                                cpuName=val;
                            else if (key=="cpu cores")
                                cpuCount+=(int)Vary(val);
                            else if (key=="siblings")
                                siblingCount+=(int)Vary(val);
                            else if (key=="bogomips")
                                cpuBogomips=(int)Vary(val);
                        }
                    }
                }
            }

            if (!cpuName.empty() || cpuCount>0 || cpuBogomips>0)
            {
                if (!cpuName.empty()) SystemInfo::ProcessorName=cpuName;
                else SystemInfo::ProcessorName="Unparsed";

                if (cpuCount==0 || cpuBogomips==0) SystemInfo::ProcessorName+=" (partially parsed /proc/cpuinfo)";

                if (cpuCount>0)
                {
                    SystemInfo::ProcessorCount=cpuCount;
                    SystemInfo::ProcessorHyperthreading=(cpuCount!=siblingCount);
                }
                if (cpuBogomips>0) SystemInfo::ProcessorBogomips=cpuBogomips;
            }
            else
                SystemInfo::ProcessorName="Could not parse /proc/cpuinfo";
        }

        // -- Populate memory information
        std::string meminfo=MISC::ReadFile("/proc/meminfo");
        if (!meminfo.empty())
        {
            //break it up into lines
            std::vector<std::string> memlines;
            MISC::ExplodeString(meminfo,memlines,"\n");

            //parse each line
            for (std::vector<std::string>::iterator line=memlines.begin(); line!=memlines.end(); ++line)
            {
                //split it over the colon
                std::vector<std::string> lineparts;
                MISC::ExplodeString(*line,lineparts,":");
                if (lineparts.size()==2)
                {
                    std::string key=MISC::StripPadding(lineparts[0]);
                    std::string val=MISC::StripPadding(lineparts[1]);

                    if (key=="MemTotal") SystemInfo::MemoryPhysicalTotal=(int)Vary(val)/1024;
                    else if (key=="SwapTotal") SystemInfo::MemorySwapTotal=(int)Vary(val)/1024;
                }
            }
        }

        // -- Populate os information
        std::string osname=MISC::ReadFile("/proc/version");
        if (!osname.empty()) SystemInfo::OperatingSystemName=osname;

        // -- Populate heuristic suggestions
        if (SystemInfo::ProcessorCount>1) SystemInfo::SuggestSleepInSpinlock=false;
        else SystemInfo::SuggestSleepInSpinlock=true;
    }
}
