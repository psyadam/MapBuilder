//internal use: Wrappers around OpenAL calls to avoid multithreading bugs in OpenAL.  As much as it claims to be thread safe, I see intermitant failures returned when more than one thread calls OpenAL at the same time.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include "AL_Include.h"
#include "../base/Locks.h"
#include "../base/DebugRouter.h"

namespace AUDIO_INTERNAL
{
    //used for locking around calls
    extern MPMA::SpinLock *lockedCallLock;
}

//use this one to try it with locks
#define TAKE_AL_CALL_LOCK MPMA::TakeSpinLock autoTakeSpinLockForScope(*AUDIO_INTERNAL::lockedCallLock)
//use this one to try it without locks:
//#define TAKE_AL_CALL_LOCK  

//only declare them inline in non-debug
//#ifndef _DEBUG
    #define CALL_LOCK_INLINE inline
//#else
//    #define CALL_LOCK_INLINE  
//#endif

//TODO: Make error output optional in some way, to avoid negatively impacting streaming

//namespace
//{
    CALL_LOCK_INLINE bool Call_alSourcePlay(ALuint source)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSourcePlay(source);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSourcePlay failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alSourceStop(ALuint source)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSourceStop(source);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSourceStop failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alGetSourcei(ALuint source, ALenum param, int *state)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alGetSourcei(source, param, state);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alGetSourcei failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alSourcei(ALuint source, ALenum param, int value)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSourcei(source, param, value);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSourcei failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alSourcef(ALuint source, ALenum param, float value)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSourcef(source, param, value);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSourcef failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alSource3f(ALuint source, ALenum param, float v1, float v2, float v3)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSource3f(source, param, v1, v2, v3);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSource3f failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alSourceQueueBuffers(ALuint source, ALsizei size, ALuint *buffers)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSourceQueueBuffers(source, size, buffers);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSourceQueueBuffers failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alSourceUnqueueBuffers(ALuint source, ALsizei size, ALuint *buffers)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alSourceUnqueueBuffers(source, size, buffers);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alSourceUnqueueBuffers failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alGenBuffers(ALsizei size, ALuint *buffers)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alGenBuffers(size, buffers);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alGenBuffers failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alDeleteBuffers(ALsizei size, ALuint *buffers)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alDeleteBuffers(size, buffers);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alDeleteBuffers failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }

    CALL_LOCK_INLINE bool Call_alBufferData(ALuint buffer, ALenum format, const void *data, ALsizei size, ALsizei freq)
    {
        TAKE_AL_CALL_LOCK;
        alGetError();
        alBufferData(buffer, format, data, size, freq);
        nuint err=alGetError();
        if (err!=AL_NO_ERROR)
        {
            MPMA::ErrorReport()<<"alBufferData failed with error "<<err<<"\n";
            return false;
        }
        return true;
    }
//}

#endif //#ifdef MPMA_COMPILE_AUDIO
