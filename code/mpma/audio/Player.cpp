//!\file Player.cpp Plays sound buffers.
//Luke Lenhart, 2008-2011
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include "Player.h"
#include "AL_Include.h"
#include "AL_LockedCalls.h"
#include "../Setup.h"
#include "../base/DebugRouter.h"

//the number of buffer subdivisions to use for each streaming source
#define STREAMING_BUFFER_SUBDIVISION_COUNT 16

namespace AUDIO_INTERNAL
{
    //retrieves an available OpenAL source, or 0 if none available
    ALuint ObtainAvailableSource();

    //returns an OpenAL source
    void ReturnAvailableSource(ALuint src);

    //defaults
    float defaultVolume=1;
    float defaultPitch=1;
    float defaultVLMin=0;
    float defaultVLMax=1;
    float defaultRef=1;
    float defaultMax=10;
    float defaultRolloff=1;

    //stream buffer
    class StreamBuffer
    {
    public:
        StreamBuffer(std::shared_ptr<AUDIO::Player> player, std::shared_ptr<AUDIO::Source> source, float streamTimeInSeconds)
        {
            initialLoad=true;
            playerReference=player;
            sourceReference=source;
            bufferSeconds=streamTimeInSeconds;
            for (int i=0; i<STREAMING_BUFFER_SUBDIVISION_COUNT; ++i)
                buffers[i]=0;
            finished=false;
            sourceRanOut=false;
            buffersStillInQueue=0;
#ifdef AUDIO_INCREASE_LATENCY_ON_UNDERRUN
            underrunSizeIncreasesLeft=7; //gets us close to 2x the original size at 10% growth each time
            secondsUntilNextSizeIncrease=0.1f;
#endif
        }

        ~StreamBuffer()
        {
            for (int i=0; i<STREAMING_BUFFER_SUBDIVISION_COUNT; ++i)
            {
                if (buffers[i])
                    Call_alDeleteBuffers(1, &buffers[i]);
                buffers[i]=0;
            }
        }

        void UpdateStream()
        {
            std::shared_ptr<AUDIO::Player> player=playerReference.lock();
            if (!player.get())
            {
                finished=true;
                return;
            }

            //unqueue or allocate initial buffers
            ALuint tempBuffers[STREAMING_BUFFER_SUBDIVISION_COUNT]={0};
            ALuint *curBuffers=tempBuffers;

            int processedCount=0;
            Call_alGetSourcei(player->player, AL_BUFFERS_PROCESSED, &processedCount);

            bool isInitialLoad=initialLoad;
            if (initialLoad)
            {
                initialLoad=false;
                Call_alGenBuffers(STREAMING_BUFFER_SUBDIVISION_COUNT, buffers);
                curBuffers=buffers;
                processedCount=STREAMING_BUFFER_SUBDIVISION_COUNT;
                buffersStillInQueue=0;
            }
            else if (processedCount>0) //existing one(s) needs unqueued and reused
            {
                Call_alSourceUnqueueBuffers(player->player, processedCount, curBuffers);
                buffersStillInQueue-=processedCount;
            }

            if (sourceRanOut)
            {
                if (buffersStillInQueue==0)
                    finished=true;

                return;
            }

            if (!isInitialLoad && !sourceRanOut)
            {
#ifdef REPORT_AUDIO_STREAMING_PROBLEMS
                if (buffersStillInQueue==0)
                {
                    MPMA::ErrorReport()<<"Buffer underrun occured in audio streaming.  Buffer size is "<<bufferSeconds<<" seconds\n";
                }
#endif

#ifdef AUDIO_INCREASE_LATENCY_ON_UNDERRUN
                if (buffersStillInQueue==0)
                {
                    if (underrunSizeIncreasesLeft>0 && secondsUntilNextSizeIncrease==0)
                    {
                        --underrunSizeIncreasesLeft;
                        bufferSeconds=bufferSeconds*1.1f;
                        secondsUntilNextSizeIncrease=0.15f;
                    }
                }

                secondsUntilNextSizeIncrease-=bufferSeconds/STREAMING_BUFFER_SUBDIVISION_COUNT*processedCount;
                if (secondsUntilNextSizeIncrease<0)
                    secondsUntilNextSizeIncrease=0;
#endif
            }

            //fill buffers from source
            if (processedCount>0)
            {
                std::shared_ptr<AUDIO::Source> source=sourceReference.lock();
                if (!source.get())
                {
                    finished=true;
                    return;
                }

                nuint sampleSize=source->GetSampleSize();
                nuint sampleRate=source->GetSampleRate();
                nuint samplesPerBuffer=(nuint)(source->GetSampleRate()*bufferSeconds/STREAMING_BUFFER_SUBDIVISION_COUNT);
                if (samplesPerBuffer<1)
                    samplesPerBuffer=1;
                if (samplesPerBuffer*sampleSize>tempBufferData.size())
                    tempBufferData.resize(samplesPerBuffer*sampleSize);

                nuint samplesToAskFor=samplesPerBuffer;
                nuint sourceSamplesRemaining=source->GetRemainingSamples();
                if (samplesToAskFor>=sourceSamplesRemaining)
                {
                    samplesToAskFor=sourceSamplesRemaining;
                    processedCount=1;
                    sourceRanOut=true;
                }

                int buffersFilled=0;
                for (int bufferInd=0; bufferInd<processedCount; ++bufferInd)
                {
                    nuint samplesOffset=0;
                    nuint samplesLeftForBuffer=samplesToAskFor;
                    while (samplesLeftForBuffer>0)
                    {
                        nuint samplesReturned=source->FillData(&tempBufferData[samplesOffset*sampleSize], samplesLeftForBuffer);
                        samplesOffset+=samplesReturned;
                        samplesLeftForBuffer-=samplesReturned;

                        if (samplesReturned==0)
                        {
                            sourceRanOut=true;
                            samplesToAskFor=0;
                            samplesLeftForBuffer=0;
                            break;
                        }
                    }

                    if (samplesOffset>0)
                    {
                        ++buffersFilled;
                        Call_alBufferData(curBuffers[bufferInd],
                            source->IsStereo()?AL_FORMAT_STEREO16:AL_FORMAT_MONO16,
                            &tempBufferData[0],
                            (ALsizei)(samplesOffset*sampleSize),
                            (ALsizei)sampleRate);
                    }

                    if (samplesToAskFor==0)
                        break;
                }

                Call_alSourceQueueBuffers(player->player, (ALsizei)buffersFilled, curBuffers);
                buffersStillInQueue+=buffersFilled;
            }

            //if it stopped (due to underrun or whatever) restart it
            if (buffersStillInQueue>0)
            {
                int state=0;
                Call_alGetSourcei(player->player, AL_SOURCE_STATE, &state);
                if (state!=AL_PLAYING)
                {
                    Call_alSourcePlay(player->player);
                }
            }
        }

        bool FinishedPlaying()
        {
            return finished;
        }

        float GetBufferSizeInSeconds()
        {
            return bufferSeconds;
        }

        //may return null if reference is gone
        std::shared_ptr<AUDIO::Player> GetPlayer()
        {
            return playerReference.lock();
        }

    private:
        ALuint buffers[STREAMING_BUFFER_SUBDIVISION_COUNT];
        std::weak_ptr<AUDIO::Player> playerReference;
        std::weak_ptr<AUDIO::Source> sourceReference;
        bool initialLoad;
        float bufferSeconds;
        bool finished;
        bool sourceRanOut;
        int buffersStillInQueue;

        std::vector<uint8> tempBufferData;

#ifdef AUDIO_INCREASE_LATENCY_ON_UNDERRUN
        nuint underrunSizeIncreasesLeft;
        float secondsUntilNextSizeIncrease;
#endif

        friend class AUDIO::Player;
    };

    std::list<std::shared_ptr<StreamBuffer>> activeStreams;
    MPMA::MutexLock *activeStreamsLock;

    //streaming worker
    class StreamProcessing
    {
    public:
        StreamProcessing()
        {
            mainThread=new3(MPMA::Thread(ThreadMain, this));
#ifdef HIGHER_PRIORITY_SOUND_THREAD
            mainThread->SetPriority(MPMA::THREAD_HIGH);
#endif
        }

        ~StreamProcessing()
        {
            delete3(mainThread);
        }

        //the main sound thread worker thread
        static void ThreadMain(MPMA::Thread &myThread, MPMA::ThreadParam param)
        {
            StreamProcessing *me=(StreamProcessing*)param.ptr;

            while (!myThread.IsEnding())
            {
                nuint msToSleep=UpdateAllStreams();

                //wait until a buffer might need refilled next
                me->block.WaitUntilClear(true, msToSleep);
            }
        }

        //wakes the worker thread so it will run immediately
        void WakeThread()
        {
            block.Clear();
        }

        static nuint UpdateAllStreams()
        {
            MPMA::TakeMutexLock autoLock(*AUDIO_INTERNAL::activeStreamsLock);

            //we need to hold a reference to all players for the streams to prevent them from cleaning up and deleting themself from activeStreams while we're walking the list
            std::list<std::shared_ptr<AUDIO::Player>> tempPlayers;
            for (auto &s:AUDIO_INTERNAL::activeStreams)
            {
                tempPlayers.emplace_back(s.get()->GetPlayer());
            }

            //
            float minBufferSize=1.0f;
            for (auto i=AUDIO_INTERNAL::activeStreams.begin(); i!=AUDIO_INTERNAL::activeStreams.end(); )
            {
                if (i->get()->FinishedPlaying())
                {
                    i=activeStreams.erase(i);
                }
                else
                {
                    i->get()->UpdateStream();

                    if (i->get()->GetBufferSizeInSeconds()<minBufferSize)
                        minBufferSize=i->get()->GetBufferSizeInSeconds();

                    ++i;
                }
            }

            return (nuint)(minBufferSize*1000.0f/STREAMING_BUFFER_SUBDIVISION_COUNT);
        }

    private:
        MPMA::Thread *mainThread;
        MPMA::BlockingObject block;
    };

    StreamProcessing *bg=0;

    //setup/shutdown
    void InitAudioSystem()
    {
        activeStreamsLock=new3(MPMA::MutexLock);
        AUDIO::SetBackgroundStreaming(true);
    }

    void ShutdownAudioSystem()
    {
        AUDIO::SetBackgroundStreaming(false);
        delete3(activeStreamsLock);
        activeStreams.clear();
    }

    class AutoSetupAudio
    {
    public:
        AutoSetupAudio()
        {
            MPMA::Internal_AddInitCallback(InitAudioSystem, 2100);
            MPMA::Internal_AddShutdownCallback(ShutdownAudioSystem, 2100);
        }
    } autoSetupAudio;
}

namespace AUDIO
{
    //If enabled (default), streams will be refilled by a worker thread as needed.  If disabled, the application must explicitely call UpdateStreams frequently enough that streaming buffers do not run out of data.
    void SetBackgroundStreaming(bool enable)
    {
        if (enable)
        {
            if (!AUDIO_INTERNAL::bg)
                AUDIO_INTERNAL::bg=new3(AUDIO_INTERNAL::StreamProcessing);
        }
        else //disable
        {
            if (AUDIO_INTERNAL::bg)
            {
                delete3(AUDIO_INTERNAL::bg);
                AUDIO_INTERNAL::bg=0;
            }
        }
    }

    //Refills buffers
    uint32 UpdateStreams()
    {
        if (AUDIO_INTERNAL::bg)
        {
            AUDIO_INTERNAL::bg->WakeThread();
            return 0;
        }
        else
            return (uint32)AUDIO_INTERNAL::StreamProcessing::UpdateAllStreams();
    }

    // -- StaticBuffer

    StaticBuffer::StaticBuffer(std::shared_ptr<Source> source)
    {
        buffer=0;

        //read the data from the source
        std::vector<uint8> data;

        //read all data from the source
        source->Seek(0);
        nuint sampleSize=source->GetSampleSize();
        nuint sampleRate=source->GetSampleRate();
        nuint sampleCount=source->GetRemainingSamples();

        if (sampleCount==INFINITE_DATA)
        {
            MPMA::ErrorReport()<<"Cannot construct a StaticBuffer from an INFINITE_DATA source\n";
            return;
        }
        else if (sampleCount==FINITE_DATA) //unknown size
        {
            data.reserve(sampleRate*sampleSize); //initial guess of 1 second

            sampleCount=0;
            nuint samplesLeft;
            do
            {
                char buff[1024*50];
                nuint ret=source->FillData(buff, sizeof(buff)/sampleSize);
                sampleCount+=ret;

                data.insert(data.end(), buff, buff+ret*sampleSize);

                samplesLeft=source->GetRemainingSamples();
            } while (samplesLeft!=0);
        }
        else //known size
        {
            data.resize(sampleSize*sampleCount);
            nuint pos=0;
            nuint samplesLeft=sampleCount;
            while (pos<data.size())
            {
                nuint ret=source->FillData(&data[pos], samplesLeft);
                pos+=ret*sampleSize;
                samplesLeft-=ret;
            }
        }

        //create the openal buffer
        if (!Call_alGenBuffers(1, &buffer))
            return;

        //TODO: try explicitely specifying a hardware buffer first

        //fill the openal buffer
        if(!Call_alBufferData(buffer,
                            source->IsStereo()?AL_FORMAT_STEREO16:AL_FORMAT_MONO16,
                            &data[0],
                            (ALsizei)data.size(),
                            (ALsizei)source->GetSampleRate()))
        {
            Call_alDeleteBuffers(1, &buffer);
            buffer=0;
        }
    }

    StaticBuffer::~StaticBuffer()
    {
        if (buffer)
            Call_alDeleteBuffers(1, &buffer);
        buffer=0;
    }

    // -- Player

    Player::Player()
    {
        player=0;
        Reset();
    }

    std::shared_ptr<Player> Player::Create(bool allocateNow)
    {
        std::shared_ptr<Player> newPlayer=std::shared_ptr<Player>(new3(Player), new3_delete<Player>());
        newPlayer->self=newPlayer;

        if (allocateNow)
        {
            newPlayer->player=AUDIO_INTERNAL::ObtainAvailableSource();
        }

        return newPlayer;
    }

    void Player::Reset()
    {
        isStatic=true;

        staticBuffer.reset();
        streamSource.reset();
        streamBuffer.reset();
    }

    Player::~Player()
    {
        Stop();

        if (player)
        {
            Call_alSourceStop(player);
            Call_alSourcei(player, AL_BUFFER, 0);
            AUDIO_INTERNAL::ReturnAvailableSource(player);
            player=0;
        }
    }

    //Starts playing a buffer on this player.
    bool Player::PlayStreaming(std::shared_ptr<Source> source, RenderMode mode, float streamSizeInSeconds)
    {
        Stop();

        //verify that this source isn't already being used for streaming
        {
            MPMA::TakeMutexLock autoLock(*AUDIO_INTERNAL::activeStreamsLock);
            for (auto i=AUDIO_INTERNAL::activeStreams.begin(); i!=AUDIO_INTERNAL::activeStreams.end(); ++i)
            {
                if (i->get()->sourceReference.lock().get()==source.get())
                    return false;
            }
        }

        //grab an openal source, and configure it to match this player
        if (!player)
        {
            player=AUDIO_INTERNAL::ObtainAvailableSource();
            if (!player)
                return false;

            SetDefaults();
        }

        //setup initial streaming buffers and play
        isStatic=false;
        streamBuffer=std::make_shared<AUDIO_INTERNAL::StreamBuffer>(self.lock(), source, streamSizeInSeconds);
        streamBuffer->UpdateStream();

        {
            MPMA::TakeMutexLock autoLock(*AUDIO_INTERNAL::activeStreamsLock);
            AUDIO_INTERNAL::activeStreams.emplace_back(streamBuffer);
        }

        if (!Call_alSourcePlay(player))
        {
            Stop();
            return false;
        }

        //wake the bg thread so that it won't oversleep
        if (AUDIO_INTERNAL::bg)
            AUDIO_INTERNAL::bg->WakeThread();

        return true;
    }

    //!Starts playing a precreated static source on this player.  If the player was already playing something, the previous sound is stopped.  Returns true if the sound could be started.
    bool Player::PlayStatic(std::shared_ptr<StaticBuffer> buffer, RenderMode mode)
    {
        Stop();

        //grab an openal source, and configure it to match this player
        if (!player)
        {
            player=AUDIO_INTERNAL::ObtainAvailableSource();
            if (!player)
                return false;

            SetDefaults();
        }

        //set the buffer to this player then start playing
        isStatic=true;

        staticBuffer=buffer;

        if (!Call_alSourcei(player, AL_BUFFER, staticBuffer->buffer))
        {
            Stop();
            return false;
        }

        if (!Call_alSourcePlay(player))
        {
            Stop();
            return false;
        }

        return true;
    }

    //Stops the currently playing sound.
    void Player::Stop()
    {
        if (!IsPlaying())
            return;

        //remove from stream list
        if (streamBuffer.get())
        {
            MPMA::TakeMutexLock autoLock(*AUDIO_INTERNAL::activeStreamsLock);
            for (auto i=AUDIO_INTERNAL::activeStreams.begin(); i!=AUDIO_INTERNAL::activeStreams.end(); ++i)
            {
                if (i->get()==streamBuffer.get())
                {
                    AUDIO_INTERNAL::activeStreams.erase(i);
                    break;
                }
            }
        }

        if (player!=0)
        {
            //stop playing
            Call_alSourceStop(player);

            //remove all buffers from it
            Call_alSourcei(player, AL_BUFFER, 0);
        }

        Reset();
    }

    //!Returns whether a sound is still being played by this player.
    bool Player::IsPlaying() const
    {
        if (isStatic)
        {
            if (!staticBuffer.get())
                return false;

            int state=0;
            Call_alGetSourcei(player, AL_SOURCE_STATE, &state);
            return state==AL_PLAYING;
        }
        else
        {
            if (!streamBuffer.get())
                return false;

            return !streamBuffer->FinishedPlaying();
        }
    }

    //Sets the global sound volume level for this player.  Value can be from 0 being silence to 1 being normal.  Default is 1.
    void Player::SetVolume(float value)
    {
        Call_alSourcef(player, AL_GAIN, value);
    }

    //Sets a pitch multiplier for the sound (must be >0)
    void Player::SetPitchMultiplier(float value)
    {
        Call_alSourcef(player, AL_PITCH, value);
    }

    //Sets the 3D position that the sound is located at.
    void Player::SetPosition(float x, float y, float z)
    {
        Call_alSource3f(player, AL_POSITION, x, y, z);
    }

    //Sets the speed and direction that sound is moving. (used for doppler)
    void Player::SetVelocity(float x, float y, float z)
    {
        Call_alSource3f(player, AL_VELOCITY, x, y, z);
    }

    //Sets the sound's output to be restricted to a cone pointing in a specific direction.  Sounds within the inner angle are normal volume, while sounds outside the outter angle are coneOutterAngleVolume volume(from 0 to 1).  Angles are in radians.
    void Player::SetCone(float dirX, float dirY, float dirZ, float coneInnerAngle, float coneOutterAngle, float coneOutterAngleVolume)
    {
        //adjust units to degrees
        coneInnerAngle=360.0f/(2*3.14159265f);
        if (coneInnerAngle<0 || coneInnerAngle>=360) coneInnerAngle=0;

        coneOutterAngle=360.0f/(2*3.14159265f);
        if (coneOutterAngle<0 || coneOutterAngle>=360) coneOutterAngle=0;

        //set
        Call_alSource3f(player, AL_DIRECTION, dirX, dirY, dirZ);
        Call_alSourcef(player, AL_CONE_INNER_ANGLE, coneInnerAngle);
        Call_alSourcef(player, AL_CONE_OUTER_ANGLE, coneOutterAngle);
        Call_alSourcef(player, AL_CONE_OUTER_GAIN, coneOutterAngleVolume);
    }

    //This sets the minimum and maximum volume the sound can be scaled.  This is applied after the current distance model's effect is applied.
    void Player::SetDistanceVolumeLimits(float minVolume, float maxVolume)
    {
        Call_alSourcef(player, AL_MIN_GAIN, minVolume);
        Call_alSourcef(player, AL_MAX_GAIN, maxVolume);
    }

    //Sets this player's ReferenceDistance for the current distance model (see Listener.h).
    void Player::SetReferenceDistance(float value)
    {
        Call_alSourcef(player, AL_REFERENCE_DISTANCE, value);
    }

    //Sets this player's MaxDistance for the current distance model (see Listener.h).
    void Player::SetMaxDistance(float value)
    {
        Call_alSourcef(player, AL_MAX_DISTANCE, value);
    }

    //Sets this player's RolloffFactor for the current distance model (see Listener.h).
    void Player::SetRolloffFactor(float value)
    {
        Call_alSourcef(player, AL_ROLLOFF_FACTOR, value);
    }

    //Sets the default value for SetVolume, which is applied to newly created Players.
    void Player::SetDefaultVolume(float value)
    {
        AUDIO_INTERNAL::defaultVolume=value;
    }

    //Sets the default value for SetPitchMultiplier, which is applied to newly created Players.
    void Player::SetDefaultPitchMultiplier(float value)
    {
        AUDIO_INTERNAL::defaultPitch=value;
    }

    //Sets the default value for SetDistanceVolumeLimits, which is applied to newly created Players.
    void Player::SetDefaultDistanceVolumeLimits(float minVolume, float maxVolume)
    {
        AUDIO_INTERNAL::defaultVLMin=minVolume;
        AUDIO_INTERNAL::defaultVLMax=maxVolume;
    }

    //Sets the default value for SetReferenceDistance, which is applied to newly created Players.
    void Player::SetDefaultReferenceDistance(float value)
    {
        AUDIO_INTERNAL::defaultRef=value;
    }

    //Sets the default value for SetMaxDistance, which is applied to newly created Players.
    void Player::SetDefaultMaxDistance(float value)
    {
        AUDIO_INTERNAL::defaultMax=value;
    }

    //Sets the default value for SetRolloffFactor, which is applied to newly created Players.
    void Player::SetDefaultRolloffFactor(float value)
    {
        AUDIO_INTERNAL::defaultRolloff=value;
    }

    //Sets up a player to match our state (what openal calls a source)
    void Player::SetDefaults()
    {
        if (!player)
            return;

        //setup the defaults
        SetVolume(AUDIO_INTERNAL::defaultVolume);
        SetPitchMultiplier(AUDIO_INTERNAL::defaultPitch);
        SetDistanceVolumeLimits(AUDIO_INTERNAL::defaultVLMin, AUDIO_INTERNAL::defaultVLMax);
        SetReferenceDistance(AUDIO_INTERNAL::defaultRef);
        SetMaxDistance(AUDIO_INTERNAL::defaultMax);
        SetRolloffFactor(AUDIO_INTERNAL::defaultRolloff);
    }
}

bool mpmaForceReferenceToPlayerCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_AUDIO
