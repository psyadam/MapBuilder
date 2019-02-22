//Console Unit Test runner
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "UnitTest.h"
#include <list>
#include "base/Memory.h"
#include "base/Types.h"
#include "base/Profiler.h" //we'll test the profiler on this console itself
#include "Setup.h"
#include "base/DebugRouter.h"
#include <iostream>
#include <exception>

#define DECLARE_TESTS_CODE
#include "TestList.h"
#undef DECLARE_TESTS_CODE

// --

//test runner
int main()
{
    //route all framework errors to stdout
    MPMA::RouterOutputStdout stdReportOutput;
    MPMA::ErrorReport().AddOutputMethod(&stdReportOutput);

    //TEST: verify error report works before init
    MPMA::ErrorReport()<<"Pre-init check.\n";

    {
        //init framework
        MPMA::InitAndShutdown scopeAutoInitAndShutdown;

        //get list of tests to run
        std::list< std::pair<std::string,UnitTest*> > autoBuiltTestList;

        MPMAProfileStart("Build Test List");
#define BUILD_TESTS_LIST
        #include "TestList.h"
#undef BUILD_TESTS_LIST
        std::list<std::pair<std::string,UnitTest*> > &testList=autoBuiltTestList;
        MPMAProfileStop("Build Test List");

        //lets keep some counts
        nuint passed=0;
        nuint failed=0;

        //run them!
        for (std::list<std::pair<std::string,UnitTest*> >::iterator i=testList.begin(); i!=testList.end(); ++i)
        {
            MPMAProfileScope("Every Test Run");

            //
            bool didPass=false;
            std::string testName="Unknown Test";
            bool boom=false;
            try
            {
                testName=(*i).first;
                std::cout<<"* Running "<<testName<<"..."<<std::endl;

                MPMAProfileScope(testName.c_str());

                didPass=(*i).second->Run();
            }
            catch (const std::exception &exc)
            {
                boom=true;
                std::cout<<"\n* !! Test threw: "<<testName<<"\n"<<exc.what()<<"\n"<<std::endl;
            }
            catch(...)
            {
                boom=true;
                std::cout<<"\n* !! Test crashed: "<<testName<<"\n"<<std::endl;
            }

            //since each test run has it's own little lifespan, lets flush the error output so that it doesn't mix with the output of a different test
            stdReportOutput.FlushInputSources();

            //
            if (didPass)
            {
                std::cout<<"* "<<testName<<" Passed.\n\n";
                ++passed;
            }
            else
            {
                if (!boom) std::cout<<"* ! "<<testName<<" Failed.\n\n";
                ++failed;
            }
        }

        //cleanup
        MPMAProfileStart("Free Test List");
        while (!testList.empty())
        {
            delete2(testList.front().second);
            testList.front().second=0;
            testList.pop_front();
        }
        MPMAProfileStop("Free Test List");

        //spit out some stats
        std::cout<<"\n* Summary for ";
#ifdef _DEBUG
        std::cout<<"Debug";
#else
        std::cout<<"Release";
#endif
        std::cout<<" build:\n "<<passed<<"   Passed\n "<<failed<<"   Failed\n";
    } //framework shuts down when scope ends

    //TEST: verify error report works after shutdown
    MPMA::ErrorReport()<<"Post-shutdown check.\n";

    //remove our output from the error report
    MPMA::ErrorReport().RemoveOutputMethod(&stdReportOutput);

    return 0;
}
