//!\file AudioSetup.cpp OpenAL setup
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"
#include "../Setup.h"

#ifdef MPMA_COMPILE_AUDIO

#include "../base/DebugRouter.h"
#include "../base/Locks.h"
#include "../base/Memory.h"
#include "AL_Include.h"
#include "Listener.h"

#include <vector>

//
namespace AUDIO_INTERNAL
{
    bool initSuccess=false;

    //openal setup data
    ALCdevice *audioDevice=0;
    ALCcontext *audioContext=0;

    //pre-created openal sources
    std::vector<ALuint> availableSources;
    nuint origSourceCount=0;
    MPMA::SpinLock *sourceCacheLock=0;

    //used for locking around calls
    MPMA::SpinLock *lockedCallLock=0;

    //setup OpenAL
    void InitOpenAL()
    {
        //set up the device
        audioDevice=alcOpenDevice(0); //for now, lets always choose the default device
        if (!audioDevice)
        {
            MPMA::ErrorReport()<<"OpenAL: alcOpenDevice failed.\n";
            return;
        }

        //set up the context
        audioContext=alcCreateContext(audioDevice, 0);
        if (!audioContext)
        {
            MPMA::ErrorReport()<<"OpenAL: alcCreateContext failed.\n";
            return;
        }

        alcMakeContextCurrent(audioContext);

        //set some defaults
        AUDIO::SetDistanceModel(AUDIO::DISTANCE_INVERSE);

        //some odd bugginess with alGenSources is causing random OpenAL calls to fail on other threads, so we pre-generate all our sources("players") here
        sourceCacheLock=new3(MPMA::SpinLock);
        for (nuint i=0; i<32; ++i)
        {
            alGetError();
            ALuint src=0;
            alGenSources(1, &src);
            if (alGetError()!=AL_NO_ERROR || src==0)
                break;
            availableSources.push_back(src);
        }

        origSourceCount=availableSources.size();
        if (origSourceCount==0)
            MPMA::ErrorReport()<<"OpenAL: alGenSources failed and we have 0 sources created.  Sound will not be playable.\n";

        //
        lockedCallLock=new3(MPMA::SpinLock);
        initSuccess=true;
    }

    //cleanup OpenAL
    void ShutdownOpenAL()
    {
        //
        if (lockedCallLock)
        {
            delete3(lockedCallLock);
            lockedCallLock=0;
        }

        //free cached sources
        if (sourceCacheLock)
        {
            delete3(sourceCacheLock);
            sourceCacheLock=0;
        }

        if (origSourceCount!=availableSources.size())
            MPMA::ErrorReport()<<"OpenAL: Cached availableSources contains fewer sources than at startup.  Some sources have been leaked!\n";
        alDeleteSources((ALsizei)availableSources.size(), &availableSources[0]);
        availableSources.clear();

        //free context
        if (audioContext)
        {
            alcMakeContextCurrent(0);
            alcDestroyContext(audioContext);
            audioContext=0;
        }

        //free device
        if (audioDevice)
        {
            alcCloseDevice(audioDevice);
            audioDevice=0;
        }

        initSuccess=false;
    }

    //hookup init callbacks
    class AutoSetup
    {
    public:
        AutoSetup()
        {
            MPMA::Internal_AddInitCallback(InitOpenAL, 2000);
            MPMA::Internal_AddShutdownCallback(ShutdownOpenAL, 2000);
        }
    } autoSetup;

    //retrieves an available OpenAL source, or 0 if none available
    ALuint ObtainAvailableSource()
    {
        if (!initSuccess)
            return 0;

        MPMA::TakeSpinLock autoLock(*sourceCacheLock);
        if (availableSources.empty())
            return 0;
        ALuint src=availableSources.back();
        availableSources.pop_back();
        return src;
    }

    //returns an OpenAL source
    void ReturnAvailableSource(ALuint src)
    {
        if (!initSuccess)
            return;

        if (src==0)
        {
            MPMA::ErrorReport()<<"ReturnAvailableSource: 0 was passed to us... this should never happen!\n";
            return;
        }

        MPMA::TakeSpinLock autoLock(*sourceCacheLock);
        availableSources.push_back(src);
    }
}

bool mpmaForceReferenceToAudioSetupCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_AUDIO
