//!\file DebugRouter.h Debug output router.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.
/*
This system allows you to hook up chains of outputs systems.
The actual output normally occurs in a different thread, minimizing overhead on the caller thread.
If used before or after the framework is inited, all output is syncronous.

For example:
One might create a DebugRouter for errors and a DebugRouter for informational messages.
The errors router could output everything to an error file.
The informational router could output everything to an informational file.
They both could also output to a third router instance, which in turns outputs everything to a debug window.
[Output Error File]<-----------------+--------[error Input]<-----------
                                    /                                  \
[Output Debug Window]<-------[merged Input]                        [Program]
                                    \                                  /
[Output Info File]<------------------+--------[info Input]<------------

*/

#pragma once

#include "Locks.h"
#include "Vary.h"
#include "Thread.h"
#include "File.h" //for Filename
#include <string>
#include <list>
#include "../Config.h"

namespace MPMA
{
    namespace MPMAInternal
    {
        class AutoInitReports;
    }

    // -- Router system

    //!Implementations of this are output targets which can accept debug data from a RouterInput and output it to anything it likes.
    class RouterOutput
    {
    public:
        inline RouterOutput(): bufferedDataProcessing(true) {}
        virtual ~RouterOutput();

        //!Implement this to recieve data that should be outputted.
        virtual void Output(const uint8 *data, nuint dataLen);

        //!If this returns false, the full output is passed on immediately rather than buffered through a thread.  By default data is marshelled on a thread.
        inline bool IsBufferProcessing() {return bufferedDataProcessing;}

        //Flushes all output from all sources that connect to this output.
        //It is recemmonded that your derived classes's destructor call this first thing, to ensure that all buffered output makes it through.
        virtual void FlushInputSources();

    protected:
        //your derived class can choose to change this from the default (true) during construction to disable buffered output
        bool bufferedDataProcessing;

    private:
        SpinLock feedLock;
        std::list<class RouterInput*> reportFeeds;
        void DetachFeed(RouterInput *feed);

        friend class RouterInput;

        //you cannot duplicate this
        RouterOutput(const RouterOutput&);
        const RouterOutput& operator=(const RouterOutput&);
    };

#ifdef DEBUGROUTER_ENABLED
    //!The program will send it's debug data to instances of this.  This will take data and marshell it to all attached outputs.
    class RouterInput: public RouterOutput
    {
    public:
        //
        RouterInput();
        ~RouterInput();

        //!Allows you to send normal textuals with the << operator, similar to ostream.
        inline RouterInput& operator<<(const Vary &v) {const std::string &s=v; Output((const uint8*)&s[0], s.size()); return *this;}
        inline friend RouterInput& operator<<(RouterInput &db, RouterInput &db2) {return db;}

        //!Sends a buffer of data.
        void Output(const uint8 *data, nuint dataLen);
        inline void Output(const Vary &v) {*this<<v;}

        //!Adds an output destination to this input.  You can also add another RouterInput as a destination.
        void AddOutputMethod(RouterOutput *outputter);

        //!Removes an output destination from this input (it is up to you to free it still) (leave the last 2 params alone, they are for internal use).
        void RemoveOutputMethod(RouterOutput *outputter, bool flushOutputFirst=true, bool removeChildLink=true);

    private:
        bool IsBufferProcessing();
        void FlushOutput();
        virtual void FlushInputSources();

        std::list<RouterOutput*> outputs;
        MutexLock outputLock;

        //data marshelling
        Thread *dataMover;
        static void ThreadProc(Thread &thread, ThreadParam param);
        volatile bool endingThreadedMode;
        BlockingObject workerWaiter;
        BlockingObject routerWaiter;
        volatile uint8 *buffer;
        volatile nuint bufferLen;
        SpinLock dataLock;
        MutexLock addDataLock;
        SpinLock threadWorkingLock;

        nuint numNonbufferedOutputs;
        bool alive;

        bool isThreadedMode;
        void DemoteToUnthreaded();

        friend class RouterOutput;
        friend class MPMAInternal::AutoInitReports;

        //you cannot duplicate this
        RouterInput(const RouterInput&);
        const RouterInput& operator=(const RouterInput&);
    };
#else //dummy do-nothing implementation - when reporder is disabled
    class RouterInput: public RouterOutput
    {
    public:
        inline RouterInput& operator<<(const Vary &v) {return *this;}
        inline friend RouterInput& operator<<(RouterInput &db, RouterInput &db2) {return db;}

        inline void Output(const uint8 *data, nuint dataLen) {}
        inline void Output(const Vary &v) {}
        inline void AddOutputMethod(RouterOutput *outputter) {}
        inline void RemoveOutputMethod(RouterOutput *outputter, bool flushOutputFirst=true, bool removeChildLink=true) {}

        friend class RouterOutput;
    };
#endif


    // -- Some useful output implementations

    //!A simple RouterInputOutput implementation that writes output to a file (overridding the file).
    class RouterOutputFile: public RouterOutput
    {
    public:
        //!ctor
        RouterOutputFile(const Filename &fileName, bool textMode=true);
        ~RouterOutputFile();

        void Output(const uint8 *data, nuint dataLen);
    private:
#ifdef DEBUGROUTER_ENABLED
        FILE *f;
        MutexLock lock;
#endif

        //you cannot duplicate this
        RouterOutputFile(const RouterOutputFile&);
        const RouterOutputFile& operator=(const RouterOutputFile&);
    };

    //!A simple RouterInputOutput implementation that writes output to a stdout.
    class RouterOutputStdout: public RouterOutput
    {
    public:
        ~RouterOutputStdout();
        void Output(const uint8 *data, nuint dataLen);
    };


    // -- framework error reporter
    //Use "MPMA::ErrorReport()" for anything that happens that should never happen within the framework.

    //!Attach outputs to this to recieve error messages from the framework.
    RouterInput& ErrorReport();


} //namespace MPMA
