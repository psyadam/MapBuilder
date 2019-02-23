//Debug output router
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "DebugRouter.h"
#include "../Setup.h"
#include "Memory.h"

#include <iostream> //for RouterOutputStdout
#include <algorithm>

namespace MPMA
{
    //the global framework error reporter
    static bool isReportReady=false;

    static RouterInput *errorReportPointer=0;    
    std::list<RouterOutput*> oldErrorOutputs; //held accross instances of the global error reporter

    //Attach outputs to this to recieve error messages from the framework.
    RouterInput& ErrorReport()
    {
        if (errorReportPointer==0)
        {
            errorReportPointer=new3(RouterInput());

            //add back any old ones from before
            for (auto i=oldErrorOutputs.begin(); i!=oldErrorOutputs.end(); ++i)
                errorReportPointer->AddOutputMethod(*i);
        }

        return *errorReportPointer;
    }

    //init stuff
    namespace MPMAInternal
    {
        class AutoInitReports
        {
        private:
            static void ReportsInitialize()
            {
                if (errorReportPointer!=0) //clean up any crusty leftovers
                {
                    //save any old outputs for later re-use
                    oldErrorOutputs.assign(ErrorReport().outputs.begin(), ErrorReport().outputs.end());

                    //free
                    delete3(errorReportPointer);
                    errorReportPointer=0;
                }

                isReportReady=true;
                ErrorReport();
            }

            static void ReportsShutDown()
            {
                isReportReady=false;

                if (errorReportPointer!=0)
                {
                    //save any old outputs for later re-use
                    oldErrorOutputs.assign(ErrorReport().outputs.begin(), ErrorReport().outputs.end());

                    //free
                    delete3(errorReportPointer);
                    errorReportPointer=0;
                }
            }

        public:
            //hookup init callbacks
            AutoInitReports()
            {
                MPMA::Internal_AddInitCallback(ReportsInitialize, -1000);
                MPMA::Internal_AddShutdownCallback(ReportsShutDown, -1000);
            }
        } autoInitReports;
    }

    RouterOutput::~RouterOutput()
    {
        TakeSpinLock takeLock(feedLock);
        for (std::list<RouterInput*>::iterator i=reportFeeds.begin(); i!=reportFeeds.end(); ++i)
            (**i).RemoveOutputMethod(this, false, false);
    }

    void RouterOutput::Output(const uint8 *data, nuint dataLen)
    {
        ErrorReport()<<"Data is being written to a base class version of RouterOutput.  This indicates that either a derived class did not implement Output, or that the derived class has been destructed already, but something tried to write output to it.  Data len="<<dataLen<<"\n";
    }

    void RouterOutput::DetachFeed(RouterInput *feed)
    {
        TakeSpinLock takeLock(feedLock);
        feed->RemoveOutputMethod(this, false, false);
        reportFeeds.remove(feed);
    }

    void RouterOutput::FlushInputSources()
    {
        TakeSpinLock takeLock(feedLock);
        for (std::list<RouterInput*>::iterator i=reportFeeds.begin(); i!=reportFeeds.end(); ++i)
            (**i).FlushInputSources();
    }

    // --
#ifdef DEBUGROUTER_ENABLED
    RouterInput::RouterInput()
    {
        bufferedDataProcessing=false;
        numNonbufferedOutputs=0;

        isThreadedMode=true;
        if (!isReportReady) //fall back if mpma is not set up
            isThreadedMode=false;

        //set us up the buffer
        buffer=0;
        if (isThreadedMode)
            buffer=new3_array(uint8, DEBUGROUTER_BUFFER_SIZE);
        bufferLen=0;

        //create the data marshelling worker thread
        endingThreadedMode=false;
        dataMover=0;
        if (isThreadedMode)
            dataMover=new3(Thread(ThreadProc,this));

        alive=true;
    }

    RouterInput::~RouterInput()
    {
        alive=false;

        //stop/free the thread
        DemoteToUnthreaded();

        //remove ourself from all outputs's lists
        nsint outputCount=outputs.size();
        while (!outputs.empty())
        {
            outputs.front()->DetachFeed(this);
        }

        //
        if (numNonbufferedOutputs!=0)
        {
            ErrorReport()<<"numNonbufferedOutputs is not 0 on destruction of a RouterInput object it was "<<numNonbufferedOutputs<<", had "<<outputCount<<" outputs before destruction and "<<outputs.size()<<" after.\n";
        }

        outputs.clear();
    }

    void RouterInput::DemoteToUnthreaded()
    {
        if (isThreadedMode)
        {
            endingThreadedMode=true;
            workerWaiter.Clear();
            delete3(dataMover);

            delete3_array(buffer);
            buffer=0;
            bufferLen=0;

            isThreadedMode=false;

            //make sure that our data actually all made it out
            FlushOutput();
        }
    }

    //writes all pending data to all destinations
    void RouterInput::FlushOutput()
    {
        if (bufferLen==0 || !isThreadedMode)
			return;

        TakeMutexLock *takelock=0;
        if (isThreadedMode)
            takelock=new3(TakeMutexLock(outputLock));
        MPMA::AutoDelete<TakeMutexLock> autoDeleteLockTaker(takelock);

        for (std::list<RouterOutput*>::iterator i=outputs.begin(); i!=outputs.end(); ++i)
        {
            if ((**i).IsBufferProcessing())
                (**i).Output((uint8*)buffer, bufferLen);
        }

        bufferLen=0;
    }

    //
    void RouterInput::FlushInputSources()
    {
        TakeSpinLock takethreadworklock(threadWorkingLock);
        RouterOutput::FlushInputSources();
        FlushOutput();
    }

    //adds an output destination to this input
    void RouterInput::AddOutputMethod(RouterOutput *outputter)
    {
        if (!alive)
			return;

        TakeMutexLock *takelock=0;
        if (isThreadedMode)
            takelock=new3(TakeMutexLock(outputLock));
        MPMA::AutoDelete<TakeMutexLock> autoDeleteLockTaker(takelock);

        outputs.push_front(outputter);

        if (!outputter->IsBufferProcessing())
			++numNonbufferedOutputs;

        TakeSpinLock takeLock(outputter->feedLock);
        outputter->reportFeeds.push_back(this);
    }

    //removes an output destination from this input
    void RouterInput::RemoveOutputMethod(RouterOutput *outputter, bool flushOutputFirst, bool removeChildLink)
    {
        TakeMutexLock takelock(outputLock);

        //make sure all output is flushed first so nothing is lost
        if (flushOutputFirst)
            FlushOutput();

        //do remove
        TakeSpinLock takethreadworklock(threadWorkingLock);
        if (std::find(outputs.begin(), outputs.end(), outputter)!=outputs.end())
        {
            outputs.remove(outputter);

            if (!outputter->IsBufferProcessing())
				--numNonbufferedOutputs;

            if (removeChildLink)
            {
                TakeSpinLock takeLock(outputter->feedLock);
                outputter->reportFeeds.remove(this);
            }
        }
    }

    //handles input to the router and moves it to the output queue
    void RouterInput::Output(const uint8 *data, nuint dataLen)
    {
        if (!alive)
			return;
        if (dataLen==0)
			return;
        if (outputs.size()==0)
			return;

        TakeMutexLock *takeDataLock=0;
        if (isThreadedMode)
            takeDataLock=new3(TakeMutexLock(addDataLock));
        MPMA::AutoDelete<TakeMutexLock> autoDeleteLockTakerData(takeDataLock);

        //do nonbuffered ones (or if we aren't inited, we never buffer)
        if (numNonbufferedOutputs>0 || !isThreadedMode)
        {
            TakeMutexLock takelock(outputLock);

            TakeMutexLock *takeOutputLock=0;
            if (isThreadedMode)
                takeOutputLock=new3(TakeMutexLock(outputLock));
            MPMA::AutoDelete<TakeMutexLock> autoDeleteLockTakerOutput(takeOutputLock);

            for (std::list<RouterOutput*>::iterator i=outputs.begin(); i!=outputs.end(); ++i)
            {
                if (!(**i).IsBufferProcessing() || !isThreadedMode)
                    (**i).Output(data, dataLen);
            }
        }

        if (outputs.size()==numNonbufferedOutputs || !isThreadedMode)
			return; //no more to do

        //now do buffered ones
        TakeSpinLock takelock(dataLock);

        nuint dataOffset=0;
        nuint dataLeft=dataLen;
        while (dataLeft>0)
        {
            nsint amt=DEBUGROUTER_BUFFER_SIZE-bufferLen;
            if (amt>(nsint)dataLeft)
				amt=dataLeft;

            memcpy((void*)(buffer+bufferLen), data+dataOffset, amt);
            bufferLen+=amt;
            dataOffset+=amt;
            dataLeft-=amt;

            if (dataLeft>0) //need to wait on thread
            {
                routerWaiter.Set();
                takelock.Leave();
                workerWaiter.Clear();
                routerWaiter.WaitUntilClear();
                takelock.Take();
            }
            else //else just wake it up
                workerWaiter.Clear();
        }
    }

    //data marshelling thread
    void RouterInput::ThreadProc(Thread& thread, ThreadParam param)
    {
        RouterInput *me=(RouterInput*)param.ptr;

        uint8 *localData=new2_array(uint8, DEBUGROUTER_BUFFER_SIZE, uint8);
        nuint localDataLen=0;

        while (!thread.IsEnding() && !me->endingThreadedMode)
        {
            me->workerWaiter.Set();
            {
                TakeSpinLock takethreadworklock(me->threadWorkingLock);
                //swap the local buffer with the main one
                {
                    TakeSpinLock takelock(me->dataLock);

                    uint8 *tmp0=localData;
                    localData=(uint8*)me->buffer;
                    me->buffer=tmp0;

                    nuint tmp1=localDataLen;
                    localDataLen=me->bufferLen;
                    me->bufferLen=tmp1;

                    me->routerWaiter.Clear();
                }

                //send to all outputs
                if (localDataLen>0)
                {
                    TakeMutexLock takelock(me->outputLock);

                    for (std::list<RouterOutput*>::iterator i=me->outputs.begin(); i!=me->outputs.end(); ++i)
                    {
                        if ((**i).IsBufferProcessing())
                            (**i).Output(localData, localDataLen);
                    }

                    localDataLen=0;
                }
            }
            me->workerWaiter.WaitUntilClear();
        }

        delete2_array(localData);
    }
#endif

    // -- useful outputs

    //stdout
    RouterOutputStdout::~RouterOutputStdout()
    {
        FlushInputSources();
    }

    void RouterOutputStdout::Output(const uint8 *data, nuint dataLen)
    {
#ifdef DEBUGROUTER_ENABLED
        std::cout.write((const char*)data, dataLen);
#endif
    }

    //file
    RouterOutputFile::RouterOutputFile(const Filename &fileName, bool textMode)
    {
#ifdef DEBUGROUTER_ENABLED
        f=fopen(fileName.c_str(), textMode?"wt":"wb");
#endif
    }

    RouterOutputFile::~RouterOutputFile()
    {
#ifdef DEBUGROUTER_ENABLED
        FlushInputSources();

        TakeMutexLock *takelock=0;
        if (isReportReady) //we assume single threaded in this case, since we're shutting down
            takelock=new3(TakeMutexLock(lock));
        MPMA::AutoDelete<TakeMutexLock> autoDeleteLockTaker(takelock);

        if (f)
			fclose(f);
        f=0;
#endif
    }

    void RouterOutputFile::Output(const uint8 *data, nuint dataLen)
    {
#ifdef DEBUGROUTER_ENABLED
        if (!f)
			return;

        TakeMutexLock *takelock=0;
        if (isReportReady) //we assume single threaded in this case, since we're shutting down
            takelock=new3(TakeMutexLock(lock));
        MPMA::AutoDelete<TakeMutexLock> autoDeleteLockTaker(takelock);

        if (!f)
			return;
        fwrite(data,dataLen,1,f);
        fflush(f);
#endif
    }
}

bool mpmaForceReferenceToDebugRouterCPP=false; //work around a problem using MPMA as a static library
