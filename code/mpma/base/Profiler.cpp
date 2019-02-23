//Luke Lenhart, 2002-2007
//See /docs/License.txt for details on how this code may be used.

#include "../Setup.h"
#include "Profiler.h"
#include "DebugRouter.h"
#include "Vary.h"
#include <string.h>
#include <stdlib.h>

#ifdef TIMEPROFILE_ENABLED

MPMA::Internal_Profiler *_instProfile=0; //instance of the profiler

namespace
{
    std::string profileFilename="_profile.txt";

    void InitProfiler()
    {
        _instProfile=new3(MPMA::Internal_Profiler);
    }

    void CleanupProfiler()
    {
        delete3(_instProfile);
        _instProfile=0;
    }

    //hookup init callbacks
    class AutoSetup
    {
    public:
        AutoSetup()
        {
            MPMA::Internal_AddInitCallback(InitProfiler, -9000);
            MPMA::Internal_AddShutdownCallback(CleanupProfiler, -9000);
        }
    } autoSetup;
}

namespace MPMA
{
Internal_Profiler::~Internal_Profiler()
{
    //set up
    FILE *f=fopen(profileFilename.c_str(), "wt"); //open file and kill old contents
    if (!f)
    {
        MPMA::ErrorReport()<<"Unable to open file for Profiler(start).\n";
        return;
    }

    const char startMsg[]="Profile File Started.\n\n";
    fwrite(startMsg, strlen(startMsg), 1, f);

    double printTime = timeStart.Step();
    Write(f, "Total time: ");
    Write(f, printTime);
    Write(f, " seconds.\n\n");

    WriteProfilesToFile(f, printTime);

    fclose(f);
}

//writes a value to a file
void Internal_Profiler::Write(FILE* f, double val)
{
    char buf[64];
	sprintf(buf, "%f", val);
    fwrite(buf, strlen(buf), 1, f);
}

void Internal_Profiler::Write(FILE* f, const char* str)
{
    fwrite(str,strlen(str),1,f);
}

void Internal_Profiler::Write(FILE* f, int num)
{
    std::string buf=(MPMA::Vary)num;
    fwrite(buf.c_str(), buf.size(), 1, f);
}

//starts profiling
void Internal_Profiler::_ProfileStart(const std::string &pName, const std::string &fName)
{
    MPMA::TakeMutexLock autoCrit(crit);

    //find profile
    SProfile* pro=FindProfile(pName);
    if (!pro) //profile doesn't exist
    {
        //allocate it
        SProfile newPro;

        //initialize it
        newPro.file=fName;
        newPro.name=pName;
        newPro.samples=0;
        newPro.timeMax=0;
        newPro.timeMin=0;
        newPro.timeTotal=0;
        newPro.started=false;

        //add it to list
        llProf.emplace_back(std::move(newPro));

        pro=&*--llProf.end();
    }
    
    if (pro->started)
    {
        Error("Warning: ProfileStart called without a matching ProfileStop\n");
        Error("  Name: ");
        Error(pName.c_str());
        Error("\n  File: ");
        Error(fName.c_str());
        Error("\n\n");
    }

    //set start
    pro->timeStart.Step();
    pro->started=true;
}

//stops profiling
void Internal_Profiler::_ProfileStop(const std::string &pName, const std::string &fName)
{
    MPMA::Timer searchTimer;

    MPMA::TakeMutexLock autoCrit(crit);

    //find the profile
    SProfile* pro=FindProfile(pName);
    if (!pro || !pro->started) //not found
    {
        Error("Warning: ProfileStop called without a matching ProfileStart\n");
        Error("  Name: ");
        Error(pName.c_str());
        Error("\n  File: ");
        Error(fName.c_str());
        Error("\n\n");
        return;
    }

    //get difference in time
    double timeDif=pro->timeStart.Step();
    pro->started=false;
    
    double searchTime=searchTimer.Step();
    timeDif-=searchTime;

    //update profile appropriatly
    if (timeDif>pro->timeMax) pro->timeMax=timeDif;
    if (timeDif<pro->timeMin || pro->samples==0) pro->timeMin=timeDif;
    pro->timeTotal+=timeDif;
    pro->samples++;
}

void Internal_Profiler::_SetOutputFile(const std::string &filename)
{
    profileFilename=filename;
}

//finds a profile, NULL if not found
Internal_Profiler::SProfile* Internal_Profiler::FindProfile(const std::string &name)
{
    std::list<SProfile>::iterator cur=llProf.begin();

    while (cur!=llProf.end())
    {
        if (name==cur->name)
            return &*cur;

        ++cur;
    }

    return 0;
}

//writes all profiles taken to file
void Internal_Profiler::WriteProfilesToFile(FILE *f, double finalTime)
{
    //write header
    const char lpzProMsg[]=" -- Profiles --\n\n";
    fwrite(lpzProMsg, strlen(lpzProMsg), 1, f);

    //sort by time total
    llProf.sort();

    std::list<SProfile>::iterator cur=llProf.begin();

    //write profiles
    while (cur!=llProf.end())
    {
        Write(f, cur->name.c_str());
        Write(f, " - ");
        Write(f, cur->file.c_str());
        Write(f, "\n");

        Write(f, " Average time: ");
        Write(f, cur->timeTotal/cur->samples*1000);
        Write(f, " ms\n");

        Write(f, " Max time: ");
        Write(f, cur->timeMax*1000);
        Write(f, " ms\n");

        Write(f, " Min time: ");
        Write(f, cur->timeMin*1000);
        Write(f, " ms\n");

        double printTime = cur->timeTotal;
        Write(f, " Total time: ");
        Write(f, printTime);
        Write(f, " seconds\n");

        Write(f, " Samples taken: ");
        Write(f, cur->samples);

        Write(f, "\n Percentage of total time: ");
        Write(f, (int)((printTime/finalTime)*100.0));

        Write(f, "%\n\n");

        ++cur;
    }
}

//opens profile file and writes error message
void Internal_Profiler::Error(const char *lpzMsg)
{
    MPMA::ErrorReport()<<lpzMsg;
}
} //namespace MPMA

#endif

bool mpmaForceReferenceToProfilerCPP=false; //work around a problem using MPMA as a static library
