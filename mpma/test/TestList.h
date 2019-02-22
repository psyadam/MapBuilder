//list of test cases, and testcase list builder
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

//a test case should:
//  not include this file
//  place a #ifdef DECLARE_TESTS_CODE before your test class
//  place a #endif after your tests code
//  place a DECLARE_TEST_CASE(YourClassName) after that #endif
//  not declare anything outside of a test case block
//  add an #include for your rest case into the list below
//  not prevent multiple includes (via #pragma once or such)

//a consumer of test cases should:
//  for compiling all test cases:
//    #define DECLARE_TESTS_CODE, but not BUILD_TESTS_LIST
//    #include this file at the global scope
//  for building a test list:
//    declare a std::list< std::pair<std::string,UnitTest*> > named autoBuiltTestList
//    #define BUILD_TESTS_LIST, but not DECLARE_TESTS_CODE
//    #include this file within the scope of autoBuildTestList
//    free the built list using delete2 on each element, when finished

#include "base/Memory.h"
#include "../code/Config.h"

//

#ifdef DECLARE_TEST_CASE
    #undef DECLARE_TEST_CASE
#endif

#ifdef BUILD_TESTS_LIST
    #define DECLARE_TEST_CASE(classname) autoBuiltTestList.push_back(std::pair<std::string,UnitTest*>(""#classname,new2(classname, classname)));
#else
    #define DECLARE_TEST_CASE(blah)
#endif

//

// -- Add your test to this list to have it run
#ifdef TESTS_BASE
    #include "cases/testSystemInfo.cpp"
    #include "cases/testTypes.cpp"
    #include "cases/testMemMan.cpp"
    #include "cases/testThread.cpp"
    #include "cases/testTimers.cpp"
    #include "cases/testLocks.cpp"
    #include "cases/testDebugRouter.cpp"
    #include "cases/testFiles.cpp"
    #include "cases/testDebug.cpp"
    #include "cases/testThreadedTask.cpp"
#endif

#ifdef MPMA_COMPILE_GEO
    #ifdef TESTS_GEO
        #include "cases/testGeo.cpp"
    #endif
#endif

#ifdef MPMA_COMPILE_NET
    #ifdef TESTS_NET
        #include "cases/testNet.cpp"
    #endif
#endif

#ifdef MPMA_COMPILE_GFXSETUP
    #ifdef TESTS_GFXSETUP
        #include "cases/testGfxSetup.cpp"
    #endif

    #ifdef MPMA_COMPILE_GFX
        #ifdef TESTS_GFX
            #include "cases/testGfx.cpp"
        #endif
    #endif

    #ifdef MPMA_COMPILE_VR
        #ifdef TESTS_VR
            #include "cases/testVr.cpp"
        #endif
    #endif

    #ifdef MPMA_COMPILE_INPUT
        #ifdef TESTS_INPUT
            #include "cases/testInput.cpp"
        #endif
    #endif
#endif

#ifdef MPMA_COMPILE_AUDIO
    #ifdef TESTS_AUDIO
        #include "cases/testAudio.cpp"
    #endif
#endif
