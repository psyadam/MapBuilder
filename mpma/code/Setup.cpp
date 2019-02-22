//Initialization and configuration options for different parts of the framework.
//See /docs/License.txt for details on how this code may be used.

#include "Config.h"
#include "Setup.h"
#include <list>
#include <algorithm>

//Setup priorities:
//Elements in the base framework should us a value below 0.
//Elements in the extended framework should use a value above 0.
//During init, they are called lowest to highest.
//During shutdown, they are called highest to lowest.

// -- -- -- Framework Setup -- -- --

//dependency check
#if defined(MPMA_COMPILE_GFX) && !defined(MPMA_COMPILE_GFXSETUP)
    #error MPMA_COMPILE_GFX requires MPMA_COMPILE_GFXSETUP
#endif

#if defined(MPMA_COMPILE_INPUT) && !defined(MPMA_COMPILE_GFXSETUP)
    #error MPMA_COMPILE_INPUT requires MPMA_COMPILE_GFXSETUP
#endif

#if defined(MPMA_COMPILE_VR) && !defined(MPMA_COMPILE_GFXSETUP)
    #error MPMA_COMPILE_VR requires MPMA_COMPILE_GFXSETUP
#endif

#if defined(MPMA_COMPILE_VR) && !defined(MPMA_COMPILE_GFX)
    #error MPMA_COMPILE_VR requires MPMA_COMPILE_GFXSETUP
#endif

#if defined(MPMA_COMPILE_VR) && !defined(MPMA_COMPILE_GEO)
    #error MPMA_COMPILE_VR requires MPMA_COMPILE_GEO
#endif

//
namespace
{
    struct PrioritizedCallback
    {
        int priority;
        void (*callback)();

        bool operator<(const PrioritizedCallback &other) const
        {
            return priority<other.priority;
        }
    };

    std::list<PrioritizedCallback> *internalInitCallbacks;
    std::list<PrioritizedCallback> *internalShutdownCallbacks;
    int initCount=0;

    //sets up the internal lists for use (will NOT be freed)
    void CheckPreInit()
    {
        if (internalInitCallbacks==0) internalInitCallbacks=new std::list<PrioritizedCallback>;
        if (internalShutdownCallbacks==0) internalShutdownCallbacks=new std::list<PrioritizedCallback>;
    }
}

//references to variables in various CPP files to work around a static link bug where constructors of globals aren't called
#ifdef MPMA_COMPILE_AUDIO
extern bool mpmaForceReferenceToAudioSetupCPP;
extern bool mpmaForceReferenceToPlayerCPP;
#endif

//#ifdef MPMA_COMPILE_BASE //base is not optional
extern bool mpmaForceReferenceToDebugRouterCPP;
extern bool mpmaForceReferenceToInfoCPP;
extern bool mpmaForceReferenceToMemoryCPP;
extern bool mpmaForceReferenceToProfilerCPP;
extern bool mpmaForceReferenceToReferenceCountCPP;
extern bool mpmaForceReferenceToThreadedTaskCPP;
extern bool mpmaForceReferenceToTextureCPP;
extern bool mpmaForceReferenceToTextWriterCPP;
#if defined(_WIN32) || defined(_WIN64)
extern bool mpmaForceReferenceToDebugWin32CPP;
#endif
//#endif

#ifdef MPMA_COMPILE_GEO
#endif

#ifdef MPMA_COMPILE_GFX
#endif

#ifdef MPMA_COMPILE_GFXSETUP
#endif

#ifdef MPMA_COMPILE_INPUT
extern bool mpmaForceReferenceToKeyboardCPP;
extern bool mpmaForceReferenceToMouseCPP;
extern bool mpmaForceReferenceToUnifiedCPP;
extern bool mpmaForceReferenceToKeyboardPlatformSpecificCPP;
extern bool mpmaForceReferenceToMousePlatformSpecificCPP;

#if (defined(_WIN64) || defined(_WIN32)) && defined(ENABLE_GAME_DEVICE_INPUT)
extern bool mpmaForceReferenceToGameDeviceWinCPP;
#endif
#endif

#if (defined(MPMA_COMPILE_VR))
extern bool mpmaForceReferenceToHmdCPP;
#endif

#ifdef MPMA_COMPILE_NET
extern bool mpmaForceReferenceToUPNPCPP;
#endif

//
namespace MPMA
{
    //Internal framework use only: registers init and shutdown callbacks
    void Internal_AddInitCallback(void (*pfunc)(), int priority)
    {
        CheckPreInit();
        PrioritizedCallback pcb;
        pcb.callback=pfunc;
        pcb.priority=priority;
        internalInitCallbacks->push_back(pcb);
    }
    void Internal_AddShutdownCallback(void (*pfunc)(), int priority)
    {
        CheckPreInit();
        PrioritizedCallback pcb;
        pcb.callback=pfunc;
        pcb.priority=priority;
        internalShutdownCallbacks->push_back(pcb);
    }

    //performs the callbacks on the inits and shutdowns
    InitAndShutdown::InitAndShutdown()
    {
        //set the dummy external variables to a value.  The only purpose of this is to work around a static link bug, it doesn't actually do anything meaningful.
        #ifdef MPMA_COMPILE_AUDIO
        mpmaForceReferenceToAudioSetupCPP=true;
        mpmaForceReferenceToPlayerCPP=true;
        #endif

        //#ifdef MPMA_COMPILE_BASE //base is not optional
        mpmaForceReferenceToDebugRouterCPP=true;
        mpmaForceReferenceToInfoCPP=true;
        mpmaForceReferenceToMemoryCPP=true;
        mpmaForceReferenceToProfilerCPP=true;
        mpmaForceReferenceToReferenceCountCPP=true;
        mpmaForceReferenceToThreadedTaskCPP=true;
        mpmaForceReferenceToTextureCPP=true;
        mpmaForceReferenceToTextWriterCPP=true;
        #if defined(_WIN32) || defined(_WIN64)
        mpmaForceReferenceToDebugWin32CPP=true;
        #endif
        //#endif

        #ifdef MPMA_COMPILE_GEO
        #endif

        #ifdef MPMA_COMPILE_GFX
        #endif

        #ifdef MPMA_COMPILE_GFXSETUP
        #endif

        #ifdef MPMA_COMPILE_INPUT
        mpmaForceReferenceToKeyboardCPP=true;
        mpmaForceReferenceToMouseCPP=true;
        mpmaForceReferenceToUnifiedCPP=true;
        mpmaForceReferenceToKeyboardPlatformSpecificCPP=true;
        mpmaForceReferenceToMousePlatformSpecificCPP=true;

        #if (defined(_WIN64) || defined(_WIN32)) && defined(ENABLE_GAME_DEVICE_INPUT)
        mpmaForceReferenceToGameDeviceWinCPP=true;
        #endif
        #endif

        #if (defined(MPMA_COMPILE_VR))
        mpmaForceReferenceToHmdCPP=true;
        #endif

        #ifdef MPMA_COMPILE_NET
        mpmaForceReferenceToUPNPCPP=true;
        #endif
        
        //now do the real work
        CheckPreInit();
        if (initCount>0)
        {
            ++initCount;
            return;
        }

        ++initCount;

        //sort the setup list by priority, then call them all
        internalInitCallbacks->sort();

        for (std::list<PrioritizedCallback>::iterator i=(*internalInitCallbacks).begin(); i!=(*internalInitCallbacks).end(); ++i)
            (*i).callback();
    }
    InitAndShutdown::~InitAndShutdown()
    {
        if (initCount<=0)
        {
            return;
        }

        if (initCount==1) //this is the last reference going away
        {
            //sort the shutdown list by priority, then call them all
            internalShutdownCallbacks->sort();

            for (std::list<PrioritizedCallback>::reverse_iterator i=(*internalShutdownCallbacks).rbegin(); i!=(*internalShutdownCallbacks).rend(); ++i)
                (*i).callback();
        }

        //
        --initCount;
    }
}
