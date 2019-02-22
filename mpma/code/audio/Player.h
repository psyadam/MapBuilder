//!\file Player.h Plays sound buffers.
//Luke Lenhart, 2008-2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include <list>
#include <vector>
#include <memory>
#include "Source.h"
#include "AL_Include.h"

namespace AUDIO_INTERNAL
{
    class StreamBuffer;
}

namespace AUDIO
{
    //!Determines how the sound should be played back.
    enum RenderMode
    {
        //!The sound has a 3D position and orientation in space.  Only mono sound sources may be used with this (stereo sources are automatically collapsed to mono).
        POSITIONAL=0x200,
        //!The sound is played without any 3D positioning applied.  This must be used to play stereo sources correctly.
        FIXED
    };

    //!If enabled (default), streams will be refilled by a worker thread as needed.  If disabled, the application must explicitely call UpdateStreams frequently enough that streaming buffers do not run out of data.
    void SetBackgroundStreaming(bool enable);

    //!If background streaming is enabled, this forces the background thread to wake up and run immediately (but does not block to wait for it to do anything) then returns 0.  If background streaming is not enabled, this refills all streaming sound buffers and then returns the approximate number of milliseconds until any more buffer data can be queued.
    uint32 UpdateStreams();

    //!Used to store static buffer data that can be re-used without having to re-read it from a source every time it is played.
    class StaticBuffer
    {
    public:
        //!Creates a static buffer and fills it from a source.  The data in source is completely read during this call, so it must be of finite length.
        StaticBuffer(std::shared_ptr<Source> source);

        virtual ~StaticBuffer();

    private:
        ALuint buffer;

        friend class Player;
    };

    //!Used to play sound buffers sources.
    class Player
    {
    public:
        //!Creates a new instance of a Player.  If allocateNow is true, openAl resources will be allocated for it immediately so that effects may be set prior to calling Play.
        static std::shared_ptr<Player> Create(bool allocateNow=false);

        virtual ~Player();

        //!Starts playing a source on this player.  If the player was already playing something, the previous sound is stopped.  A source may not be played by more than one player at a time.  Returns true if the sound could be started.
        bool PlayStreaming(std::shared_ptr<Source> source, RenderMode mode, float streamSizeInSeconds=0.2f);

        //!Starts playing a precreated static source on this player.  If the player was already playing something, the previous sound is stopped.  A StaticBuffer may be played by any number of players at the same time.  Returns true if the sound could be started.
        bool PlayStatic(std::shared_ptr<StaticBuffer> buffer, RenderMode mode);

        //!Stops the currently playing sound.
        void Stop();

        //!Returns whether a sound is still being played by this player.
        bool IsPlaying() const;

        //!Sets the global sound volume level for this player.  Value can be from 0 being silence to 1 being normal.  Default is 1.
        void SetVolume(float value);
        //!Sets the default value for SetVolume, which is applied to newly created Players.
        static void SetDefaultVolume(float value);

        //!Sets a pitch multiplier for the sound (must be >0)
        void SetPitchMultiplier(float value);
        //!Sets the default value for SetPitchMultiplier, which is applied to newly created Players.
        static void SetDefaultPitchMultiplier(float value);

        //!Sets the 3D position that the sound is located at.
        void SetPosition(float x, float y, float z);
        //!Sets the 3D position that the sound is located at.
        template <typename VectorType>
        inline void SetPosition(const VectorType &pos)
            { SetPosition(pos[0], pos[1], pos[2]); }

        //!Sets the speed and direction that sound is moving. (used for doppler)
        void SetVelocity(float x, float y, float z);
        //!Sets the speed and direction that sound is moving. (used for doppler)
        template <typename VectorType>
        inline void SetVelocity(const VectorType &vel)
            { SetVelocity(vel[0], vel[1], vel[2]); }

        //!Sets the sound's output to be restricted to a cone pointing in a specific direction.  Sounds within the inner angle are normal volume, while sounds outside the outter angle are coneOutterAngleVolume volume(from 0 to 1).  Angles are in radians.
        void SetCone(float dirX, float dirY, float dirZ, float coneInnerAngle, float coneOutterAngle, float coneOutterAngleVolume);
        //!Sets the sound's output to be restricted to a cone pointing in a specific direction.  Sounds within the inner angle are normal volume, while sounds outside the outter angle are coneOutterAngleVolume volume(from 0 to 1).  Angles are in radians.
        template <typename VectorType>
        inline void SetCone(const VectorType &dir, float coneInnerAngle, float coneOutterAngle, float coneOutterAngleVolume)
            { SetCone(dir[0], dir[1], dir[2], coneInnerAngle, coneOutterAngle, coneOutterAngleVolume); }

        //!This sets the minimum and maximum volume the sound can be scaled.  This is applied after the current distance model's effect is applied.
        void SetDistanceVolumeLimits(float minVolume, float maxVolume);
        //!Sets the default value for SetDistanceVolumeLimits, which is applied to newly created Players.
        static void SetDefaultDistanceVolumeLimits(float minVolume, float maxVolume);

        //!Sets this player's ReferenceDistance for the current distance model (see Listener.h).
        void SetReferenceDistance(float value);
        //!Sets the default value for SetReferenceDistance, which is applied to newly created Players.
        static void SetDefaultReferenceDistance(float value);

        //!Sets this player's MaxDistance for the current distance model (see Listener.h).
        void SetMaxDistance(float value);
        //!Sets the default value for SetMaxDistance, which is applied to newly created Players.
        static void SetDefaultMaxDistance(float value);

        //!Sets this player's RolloffFactor for the current distance model (see Listener.h).
        void SetRolloffFactor(float value);
        //!Sets the default value for SetRolloffFactor, which is applied to newly created Players.
        static void SetDefaultRolloffFactor(float value);

        //TODO: effects (reverb etc)

        //!Applies the default settings to the current player
        void SetDefaults();

    private:
        Player();
        void Reset();

        ALuint player; //(openal source) - this is lazy initialized but kept around until the class is freed after that
        bool isStatic; //whether the data is static or being streamed

        //used by for playing static buffers
        std::shared_ptr<StaticBuffer> staticBuffer;

        //used for playing streaming sources
        std::shared_ptr<Source> streamSource;
        std::shared_ptr<AUDIO_INTERNAL::StreamBuffer> streamBuffer;
        std::weak_ptr<Player> self;

        friend class AUDIO_INTERNAL::StreamBuffer;
    };
}

#endif //#ifdef MPMA_COMPILE_AUDIO
