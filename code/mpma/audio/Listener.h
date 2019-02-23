//!\file Listener.h Controls the sound listener, which all sounds are relative to.  Also controls global sound settings.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

//!Sound playback and streaming.
namespace AUDIO
{
    //!Sets the global sound volume level that affects all sounds.  Value can be from 0 being silence to 1 being normal.  Default is 1.
    void SetGlobalVolume(float value);

    //!Sets the position the listener is at.
    void SetListenerPosition(float x, float y, float z);
    //!Sets the position the listener is at.
    template <typename VectorType>
    inline void SetListenerPosition(const VectorType &pos)
        { SetListenerPosition(pos[0], pos[1], pos[2]); }

    //!Sets the direction the listener is facing.
    void SetListenerFacing(float x, float y, float z);
    //!Sets the direction the listener is facing.
    template <typename VectorType>
    inline void SetListenerFacing(const VectorType &dir)
        { SetListenerFacing(dir[0], dir[1], dir[2]); }

    //!Sets the direction that is up from the listener's head.
    void SetListenerUp(float x, float y, float z);
    //!Sets the direction that is up from the listener's head.
    template <typename VectorType>
    inline void SetListenerUp(const VectorType &up)
        { SetListenerUp(up[0], up[1], up[2]); }

    //!Sets the speed and direction that listener is moving. (used for doppler)
    void SetListenerVelocity(float x, float y, float z);
    //!Sets the speed and direction that listener is moving. (used for doppler)
    template <typename VectorType>
    inline void SetListenerVelocity(const VectorType &vel)
        { SetListenerVelocity(vel[0], vel[1], vel[2]); }

    //!A scalar(>=0) that is can be used to emphasize or reduce the doppler effect globally.  Default is 1.  Setting to 0 disables doppler.
    void SetDopplerFactor(float value);

    //!Sets the speed of sound used for doppler calculations.  Must be >0.  Default is 343.3.
    void SetSpeedOfSound(float value);

    //!Determines how distance affects the volume of 3D sounds.  Most of the parameters used here can be found in the Player class (See Player.h).
    enum DistanceModel
    {
        //!Distance does not affect sound volume.
        DISTANCE_NONE=0x300,
        //!Works like 1/distance. (default)  The exact volume scale is: ReferenceDistance / (ReferenceDistance + RolloffFactor * (distance - ReferenceDistance)).
        DISTANCE_INVERSE,
        //!Same as DISTANCE_INVERSE, except the distance used in the calculation is clamped between ReferenceDistance and MaxDistance.
        DISTANCE_INVERSE_CLAMPED,
        //!Works like 1-distance.  The exact volume scale is: 1 - RolloffFactor * (distance - ReferenceDistance) / (MaxDistance - ReferenceDistance).  The distance is clamped between 0 and MaxDistance.
        DISTANCE_LINEAR,
        //!Same as DISTANCE_LINEAR, except the distance is clamped between ReferenceDistance and MaxDistance.
        DISTANCE_LINEAR_CLAMPED,
        //!Works like distance^(-n).  The exact volume scale is (distance / ReferenceDistance) ^ (-RolloffFactor).
        DISTANCE_EXPONENTIAL,
        //!Same as DISTANCE_EXPONENTIAL, except the distance is clamped between ReferenceDistance and MaxDistance.
        DISTANCE_EXPONENTIAL_CLAMPED
    };

    //!Sets the formula used to calculate how distance affects the volume of sounds in 3D space.
    void SetDistanceModel(DistanceModel distanceModel);

}; //namespace AUDIO

#endif //#ifdef MPMA_COMPILE_AUDIO
