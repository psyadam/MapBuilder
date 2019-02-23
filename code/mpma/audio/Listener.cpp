//!\file Listener.cpp Controls the sound listener, which all sounds are relative to.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include "Listener.h"
#include "AL_Include.h"
#include "../base/DebugRouter.h"

//
namespace AUDIO
{
    //Sets the global sound volume control that affects all sounds.
    void SetGlobalVolume(float value)
    {
        alListenerf(AL_GAIN, value);
    }

    //Sets the position the listener is at.
    void SetListenerPosition(float x, float y, float z)
    {
        alListener3f(AL_POSITION, x, y, z);
    }

    //Sets the direction the listener is facing.
    void SetListenerFacing(float x, float y, float z)
    {
        float orientation[6];
        alGetListenerfv(AL_ORIENTATION, orientation);
        orientation[0]=x;
        orientation[1]=y;
        orientation[2]=z;
        alListenerfv(AL_ORIENTATION, orientation);
    }

    //Sets the direction that is up from the listener's head.
    void SetListenerUp(float x, float y, float z)
    {
        float orientation[6];
        alGetListenerfv(AL_ORIENTATION, orientation);
        orientation[3]=x;
        orientation[4]=y;
        orientation[5]=z;
        alListenerfv(AL_ORIENTATION, orientation);
    }

    //Sets the speed and direction that listener is moving.
    void SetListenerVelocity(float x, float y, float z)
    {
        alListener3f(AL_VELOCITY, x, y, z);
    }

    //A scalar used to emphasize or reduce the doppler effect globally.
    void SetDopplerFactor(float value)
    {
        alDopplerFactor(value);
    }

    //Sets the speed of sound used for doppler calculations.
    void SetSpeedOfSound(float value)
    {
        alSpeedOfSound(value);
    }

    //Sets the formula used to calculate how distance affects the volume of sounds in 3D space.
    void SetDistanceModel(DistanceModel distanceModel)
    {
        if (distanceModel==DISTANCE_NONE) alDistanceModel(AL_NONE);
        else if (distanceModel==DISTANCE_INVERSE) alDistanceModel(AL_INVERSE_DISTANCE);
        else if (distanceModel==DISTANCE_INVERSE_CLAMPED) alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        else if (distanceModel==DISTANCE_LINEAR) alDistanceModel(AL_LINEAR_DISTANCE);
        else if (distanceModel==DISTANCE_LINEAR_CLAMPED) alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
        else if (distanceModel==DISTANCE_EXPONENTIAL) alDistanceModel(AL_EXPONENT_DISTANCE);
        else if (distanceModel==DISTANCE_EXPONENTIAL_CLAMPED) alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
        else MPMA::ErrorReport()<<"Bad value passed to SetDistanceModel.\n";
    }
}

#endif //#ifdef MPMA_COMPILE_AUDIO
