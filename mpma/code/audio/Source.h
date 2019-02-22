//!\file Source.h Sound sample sources.
//Luke Lenhart, 2008-2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include "../base/Types.h"
#include "../base/ReferenceCount.h"
#include "../base/Locks.h"

#include <string>
#include <memory>
#include <vector>

//forward declarations
struct OggVorbis_File;
struct OggOpusFile;

namespace AUDIO
{
    //!There is a finite amount of data left, but the source doesn't know exactly how much.
    const nuint FINITE_DATA=(nuint)(0-2);
    //!The source will never run out of data.
    const nuint INFINITE_DATA=(nuint)(0-1);

    //!Represents a source of sound sample data.
    class Source
    {
    public:
        Source();
        virtual ~Source();

        //!Instructs the data source to write in the next maxSamples worth of data.  It should return how many samples were actually written.  If less samples than requested were returned and GetRemainingSamples still returns that there is data left, this may be called multiple times consecutively.  All sound samples must be 16-bit signed values.  Stereo samples must be interleaved, with the left channel first.
        virtual nuint FillData(void *data, nuint maxSamples)=0;

        //!This must return the number of samples left to be read, or 0 if all data has been read.  For stereo sources, a single sample includes both left and right channel data.  It may also return FINITE_DATA or INFINITE_DATA.
        virtual nuint GetRemainingSamples() const=0;

        //!Returns whether the sound is stereo.  Stereo sources will have their output collpased to mono if they are used with 3D positioning.  The value returned from this must remain constant for any given instance.
        virtual bool IsStereo() const=0;

        //!Returns the number of samples per second that should be played under normal conditions.  The value returned from this must remain constant for any given instance.
        virtual nuint GetSampleRate() const=0;

        //!Tells the data source to start returning samples from a specific position (not applicable to all sources).
        virtual void Seek(nuint sampleNumber=0);

        //!Returns the size of a single sample.  Currently this will always be either 2 for mono or 4 for stereo, since all samples must be 16-bit.
        inline nuint GetSampleSize() const
            { return IsStereo()?4:2; }
    };


    // -- below are a several convenient implementations of Source

    //!Reads sound from a .wav file.
    class WavFileSource: public Source
    {
    public:
        //!Creates a new source from a .wav file.
        WavFileSource(const std::string &filename);
        virtual ~WavFileSource();

        //!Returns whether the source has a valid file loaded.  If there were problems loading the file, this will return false.
        inline bool IsLoaded() const
            { return fileHandle!=0; }

        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
        virtual void Seek(nuint sampleNumber=0);

    private:
        void LoadFailed();

        void *fileHandle;
        std::string filename;
        nuint rate;
        nuint sourceBits;
        bool isStereo;
        nuint dataStartSeekPos;

        bool hitEof;

        Source *bad; //bad source to use if we failed to load
        MPMA::SpinLock lock;
    };

#ifdef AUDIO_OGG_VORBIS_ENABLED
    //!Reads sound from a .ogg/.ogm file.
    class VorbisFileSource: public Source
    {
    public:
        //!Creates a new source from a .ogg/.ogm file on disk.
        VorbisFileSource(const std::string &filename);
        //!Creates a new source from a .ogg/.ogm file in memory.
        VorbisFileSource(const void *pFileInMemory, uint32 memoryLength);
        virtual ~VorbisFileSource();

        //!Returns whether the source has a valid file loaded.  If there were problems loading the file, this will return false.
        inline bool IsLoaded() const
            { return vorbisFile!=0; }

        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
        virtual void Seek(nuint sampleNumber=0);

    private:
        OggVorbis_File *vorbisFile;
        std::string filename;
        nuint rate;
        nuint sourceBits;
        bool isStereo;

        bool hitEof;

        Source *bad; //bad source to use if we failed to load
        MPMA::SpinLock lock;

        //things for reading directly from memory rather than a file
        struct VorbisMemoryData
        {
            inline VorbisMemoryData(): Cursor(0) {}

            std::vector<unsigned char> Data;
            uint32 Cursor;
        } vorbisInMemory;

        static size_t VorbisMemoryRead(void *ptr, size_t size, size_t nmemb, void *datasource);
        static int VorbisMemorySeek(void *datasource, sint64 offset, int whence);
        static int VorbisMemoryClose(void *datasource);
        static long VorbisMemoryTell(void *datasource);

    };
#endif

#ifdef AUDIO_OGG_FLAC_ENABLED
    class InternalMpmaFlacDecoder;

    //!Reads sound from a .flac file.
    class FlacFileSource: public Source
    {
    public:
        //!Creates a new source from a .flac.
        FlacFileSource(const std::string &filename);
        virtual ~FlacFileSource();

        //!Returns whether the source has a valid file loaded.  If there were problems loading the file, this will return false.
        inline bool IsLoaded() const
            { return flac!=0; }

        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
        virtual void Seek(nuint sampleNumber=0);

    private:
        InternalMpmaFlacDecoder *flac;
        std::string filename;

        bool hitEof;

        Source *bad; //bad source to use if we failed to load
        MPMA::SpinLock lock;
    };
#endif

#ifdef AUDIO_OGG_OPUS_ENABLED
    //!Reads sound from a .opus file.
    class OpusFileSource: public Source
    {
    public:
        //!Creates a new source from a .opus file on disk.
        OpusFileSource(const std::string &filename);
        //!Creates a new source from a .opus file in memory.
        OpusFileSource(const void *pFileInMemory, uint32 memoryLength);
        virtual ~OpusFileSource();

        //!Returns whether the source has a valid file loaded.  If there were problems loading the file, this will return false.
        inline bool IsLoaded() const
            { return opusFile!=nullptr; }

        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
        virtual void Seek(nuint sampleNumber=0);

    private:
        void SetInfo();

        OggOpusFile *opusFile;
        std::string filename;
        sint64 totalSamples;
        bool isStereo;
        bool needStereoDownmix;
        nuint sampleRate;
        bool hitEof;

        Source *bad; //bad source to use if we failed to load
        MPMA::SpinLock lock;
    };
#endif

    //!Source that creates a streaming infinite length loop from another source.  If reading data reaches the end or the loop endpoint, it seeks back to the loop start point.
    class LoopSource: public Source
    {
    public:
        //!Source will initially start at the beginning (you can always Seek() still), and read until loopEndSample or the end is hit, at which point it is Seek()'d back to loopStartSample.  This will loop forever.
        LoopSource(std::shared_ptr<Source> source, nuint loopStartSample=0, nuint loopEndSample=~0);

        virtual nuint FillData(void *data, nuint maxSamples);
        inline nuint GetRemainingSamples() const { return INFINITE_DATA; }
        inline bool IsStereo() const { return source->IsStereo(); }
        inline nuint GetSampleRate() const { return source->GetSampleRate(); }

    private:
        std::shared_ptr<Source> source;
        nuint loopStart;
        nuint loopEnd;
        nuint currentSample;
    };

    //!Combines two mono sources into a stereo source.
    class MonoToStereoSource: public Source
    {
    public:
        //!Constructs a stereo source that combines two mono sources together to form a left and right channel.  The sources passed in will be duplicated, so the originals may be freed after the call.  If the sources are different lengths, silence will fill in the last portion of the shorter source.
        MonoToStereoSource(std::shared_ptr<Source> sourceLeft, std::shared_ptr<Source> sourceRight);

        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
        virtual void Seek(nuint sampleNumber=0);

    private:
        std::shared_ptr<Source> sourceL, sourceR;
    };

    //!Generates nothing.
    class EmptySource: public Source
    {
    public:
        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
    };

    //!White noise generator.
    class WhiteNoiseSource: public Source
    {
    public:
        //!Constructs a mono white noise generator, which will a specific amount of time's worth of white noise.  If seconds is 0, then an infinite amount of white noise will be supplied.  quietLevel is a value from 0-31 for how much quieter to reduce the volume of the generated audio.
        WhiteNoiseSource(float seconds=0, nuint sampleRate=44100, int quietLevel=0);

        virtual nuint FillData(void *data, nuint maxSamples);
        virtual nuint GetRemainingSamples() const;
        virtual bool IsStereo() const;
        virtual nuint GetSampleRate() const;
        virtual void Seek(nuint sampleNumber=0);

    private:
        nuint samplesRemaining;
        nuint rate;
        float origTimeLeft;
        uint32 lastSample;
        int quietness;
    };
}

#endif //#ifdef MPMA_COMPILE_AUDIO
