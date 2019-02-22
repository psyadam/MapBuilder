//! \file Profiler.h \brief Profile the speed of sections of code.
//!
//!The names of a profile are "not" case sensitive.  The results of the profile will be written to the profile file (default _profile.txt) after the progam ends.

/*
An example of profiling a section of code:
  MPMAProfileStart("Code Section 1");
  DoSomething();
  DoSomethingElse();
  MPMAProfileStop("Code Section 1");

An example of automatically profiling the scope of a function:
  void SomeFunction()
  {
      MPMAProfileScope("name of a profile");
      //...code...
  }
*/
/*
Written by Luke Lenhart (2002-2008)
 Updated in late 2004 to be thread-safe, and to use stl
 Rewritten in 2007 for multiple platforms
See /docs/License.txt for details on how this code may be used.
*/

#pragma once
#include "../Config.h"

//macro replacement
#ifdef TIMEPROFILE_ENABLED
    //!Set the filename to save profile data to, after the program ends.
    #define MPMAProfileSetOutputFile(filename) do { if (_instProfile) _instProfile->_SetOutputFile(filename); } while (false)
    //!Begin profiling a section of code.
    #define MPMAProfileStart(name)  do { if (_instProfile) _instProfile->_ProfileStart(name,__FILE__); } while (false)
    //!End profiling a section of code.
    #define MPMAProfileStop(name)  do { if (_instProfile) _instProfile->_ProfileStop(name,__FILE__); } while (false)

    #define MPMAProfileScopeIter0(name, iter) MPMA::Internal_AutoProfileHelper _auto_scope_profiler##iter(name); MPMAProfileStart(name)
    #define MPMAProfileScopeIter1(name, iter) MPMAProfileScopeIter0(name, iter)
    //!Profile a scope of code.
    #define MPMAProfileScope(name) MPMAProfileScopeIter1(name, __COUNTER__)
#else
    #define MPMAProfileSetOutputFile(filename)
    #define MPMAProfileStart(name)
    #define MPMAProfileStop(name)
    #define MPMAProfileScope(name)
#endif


//
#ifdef TIMEPROFILE_ENABLED

#include <string>
#include <list>
#include "Locks.h"
#include "Types.h"
#include "Timer.h"

namespace MPMA
{
    //main profiler
    class Internal_Profiler
    {
    public:
        ~Internal_Profiler();

        void _ProfileStart(const std::string &pName, const std::string &fName);
        void _ProfileStop(const std::string &pName, const std::string &fName);
        void _SetOutputFile(const std::string &filename);

    private:
        struct SProfile
        {
            std::string name; //name for this profile
            std::string file; //filename for this profile

            double timeTotal; //total time spent
            double timeMax; //largest time spent
            double timeMin; //smallest time spent

            MPMA::Timer timeStart; //time of last profile start

            int samples; //number of samples taken
            
            bool started;

            inline bool operator <(const SProfile &o) { return timeTotal>o.timeTotal; }
        };

        std::list<SProfile> llProf; //profile list

        MPMA::Timer timeStart; //time class was created

        //

        void Write(FILE* f, double val);
        void Write(FILE* f, const char* str);
        void Write(FILE* f, int num);

        SProfile* FindProfile(const std::string &name);

        void WriteProfilesToFile(FILE *f, double finalTime);

        void Error(const char *lpzMsg);

        //thread safety
        MPMA::MutexLock crit;
    };
}

#endif

//access to profiler object
#ifdef TIMEPROFILE_ENABLED
    extern MPMA::Internal_Profiler *_instProfile;
#endif

//auto-scope profiler (exception-safe and return-safe) -- use macro, not this!!
#ifdef TIMEPROFILE_ENABLED
namespace MPMA
{
    class Internal_AutoProfileHelper
    {
    public:
        inline Internal_AutoProfileHelper(const std::string &profileName) { name=profileName; }
        inline ~Internal_AutoProfileHelper() { MPMAProfileStop(name); }

    private:
        std::string name;
    };
}
#endif
