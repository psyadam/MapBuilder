//!\file Source.cpp Sound sample sources.
//Luke Lenhart, 2008-2011
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include "Source.h"
#include "../base/DebugRouter.h"

#include <stdlib.h>

#ifdef AUDIO_OGG_VORBIS_ENABLED
    #include <vorbis/vorbisfile.h>
#endif

#ifdef AUDIO_OGG_FLAC_ENABLED
    #include <FLAC++/decoder.h>
#endif

#ifdef AUDIO_OGG_OPUS_ENABLED
    #include <opus/opus.h>
    #include <opus/opusfile.h>
#endif

namespace AUDIO
{
    // -- base Source implementation

    Source::Source()
    {
    }

    Source::~Source()
    {
    }

    void Source::Seek(nuint sampleNumber)
    {
    }

    // -- LoopSource

    LoopSource::LoopSource(std::shared_ptr<Source> source, nuint loopStartSample, nuint loopEndSample)
    {
        this->source=source;
        loopStart=loopStartSample;
        loopEnd=loopEndSample;
        currentSample=0;
    }

    nuint LoopSource::FillData(void *data, nuint maxSamples)
    {
        //restart if we passed the end
        nuint remainingSamples=source->GetRemainingSamples();
        if (currentSample>=loopEnd || remainingSamples==0)
        {
            source->Seek(loopStart);
            currentSample=loopStart;
            remainingSamples=source->GetRemainingSamples();
            if (remainingSamples==0)
                return 0;
        }

        //decide how many samples to try to read
        if (maxSamples>remainingSamples)
            maxSamples=remainingSamples;
        nuint samplesTillLoop=loopEnd-currentSample;
        if (maxSamples>samplesTillLoop)
            maxSamples=samplesTillLoop;

        //
        return source->FillData(data, maxSamples);
    }


    // -- MonoToStereoSource

    MonoToStereoSource::MonoToStereoSource(std::shared_ptr<Source> sourceLeft, std::shared_ptr<Source> sourceRight)
    {
        //first verify that their sample rate and format are the same.
        bool valid=true;
        if (sourceLeft->GetSampleRate()!=sourceRight->GetSampleRate())
            valid=false;
        if (sourceLeft->IsStereo()!=sourceRight->IsStereo())
            valid=false;
        if (sourceLeft->IsStereo())
            valid=false;
        if (sourceRight->IsStereo())
            valid=false;

        if (valid)
        {
            sourceL=sourceLeft;
            sourceR=sourceRight;
        }
        else //error, so substitute the sources
        {
            MPMA::ErrorReport()<<"A sound source was passed to MonoToStereoSource that is not valid for use here.\n";

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            sourceL=std::make_shared<WhiteNoiseSource>(1.0f);
            sourceR=std::make_shared<WhiteNoiseSource>(1.0f);
#else //use silence
            sourceL=std::make_shared<EmptySource>();
            sourceR=std::make_shared<EmptySource>();
#endif
        }
    }

    nuint MonoToStereoSource::FillData(void *data, nuint maxSamples)
    {
        if (maxSamples>2048) maxSamples=2048;
        uint16 buffL[2048];
        uint16 buffR[2048];

        //get data for both channels
        nuint samples=0;
        if (sourceL->GetRemainingSamples()>sourceR->GetRemainingSamples()) //left has more samples than right
        {
            //pull data from L, then fill in data from R up to that much
            nuint samplesL=sourceL->FillData(buffL, maxSamples);
            samples=samplesL;
            nuint samplesR=0;

            while (samplesR<samplesL) //pull data until R runs out
            {
                nuint ret=sourceR->FillData(buffR+samplesR, samplesL-samplesR);
                samplesR+=ret;
                if (ret==0) break;
            }

            while (samplesR<samplesL) //fill the rest of R with 0
            {
                buffR[samplesR]=0;
                ++samplesR;
            }
        }
        else //right has more samples than left
        {
            //pull data from R, then fill in data from L up to that much
            nuint samplesR=sourceR->FillData(buffR, maxSamples);
            samples=samplesR;
            nuint samplesL=0;

            while (samplesL<samplesR) //pull data until L runs out
            {
                nuint ret=sourceL->FillData(buffL+samplesL, samplesR-samplesL);
                samplesL+=ret;
                if (ret==0)
                    break;
            }

            while (samplesL<samplesR) //fill the rest of L with 0
            {
                buffL[samplesL]=0;
                ++samplesL;
            }
        }

        //interleave the 2 channel data now
        for (nuint i=0; i<samples; ++i)
        {
            ((uint16*)data)[2*i+0]=buffL[i];
            ((uint16*)data)[2*i+1]=buffR[i];
        }

        return samples;
    }

    nuint MonoToStereoSource::GetRemainingSamples() const
    {
        //get the max of the two sources
        nuint samps=sourceL->GetRemainingSamples();
        nuint sampsR=sourceR->GetRemainingSamples();
        if (sampsR>samps)
            samps=sampsR;

        return samps;
    }

    bool MonoToStereoSource::IsStereo() const
    {
        return true;
    }

    nuint MonoToStereoSource::GetSampleRate() const
    {
        return sourceL->GetSampleRate();
    }

    void MonoToStereoSource::Seek(nuint sampleNumber)
    {
        sourceL->Seek(sampleNumber);
        sourceR->Seek(sampleNumber);
    }

    // -- EmptySource

    nuint EmptySource::FillData(void *data, nuint maxSamples)
    {
        return 0;
    }

    nuint EmptySource::GetRemainingSamples() const
    {
        return 0;
    }

    bool EmptySource::IsStereo() const
    {
        return false;
    }

    nuint EmptySource::GetSampleRate() const
    {
        return 1;
    }


    // -- WhiteNoiseSource

    WhiteNoiseSource::WhiteNoiseSource(float seconds, nuint sampleRate, int quietLevel): rate(sampleRate), origTimeLeft(seconds), quietness(quietLevel)
    {
        samplesRemaining=INFINITE_DATA;
        if (seconds>0.0f)
        {
            samplesRemaining=(nuint)(seconds*sampleRate);
        }

        lastSample=(uint16)rand();
    }

    nuint WhiteNoiseSource::FillData(void *data, nuint maxSamples)
    {
        nuint samples=maxSamples;
        if (samplesRemaining!=INFINITE_DATA && samples>samplesRemaining) samples=samplesRemaining;

        for (nuint i=0; i<samples; ++i)
        {
            //rotate left a chunk, xor one half with the other, then add one to stir it up a bit
            lastSample=(lastSample<<11) | (lastSample>>(32-11));
            lastSample^=lastSample>>16;
            ++lastSample;

            ((uint16*)data)[i]=(uint16)(lastSample)>>quietness;
        }

        if (samplesRemaining!=INFINITE_DATA) samplesRemaining-=samples;
        return samples;
    }

    nuint WhiteNoiseSource::GetRemainingSamples() const
    {
        return samplesRemaining;
    }

    bool WhiteNoiseSource::IsStereo() const
    {
        return false;
    }

    nuint WhiteNoiseSource::GetSampleRate() const
    {
        return rate;
    }

    void WhiteNoiseSource::Seek(nuint sampleNumber)
    {
        if (origTimeLeft>0.0f)
        {
            nsint rem=(nsint)(origTimeLeft*rate)-sampleNumber;
            if (rem<0)
                MPMA::ErrorReport()<<"Bad sample number passed to WhiteNoiseSource::Seek.";
            else
                samplesRemaining=rem;
        }
    }


    // -- WavFileSource

    //Creates a new source from a .wav file.
    WavFileSource::WavFileSource(const std::string &filename): fileHandle(0), rate(0), sourceBits(0), isStereo(false), dataStartSeekPos(0), hitEof(false), bad(0)
    {
        this->filename=filename;

        //open the file
        FILE *f=fopen(filename.c_str(), "rb");
        if (!f)
        {
            MPMA::ErrorReport()<<"WavFileSource: Failed to open file: "<<filename<<"\n";
            LoadFailed();
            return;
        }
        fileHandle=f;

        //read in the file header and validate it is what we expect
        struct
        {
            char type[4];
            uint32 remainingSize;
            char format[4];
        } fileHeader;

        if (!fread(&fileHeader, 12, 1, f))
        {
            MPMA::ErrorReport()<<"WavFileSource: Unable to read file header: "<<filename<<"\n";
            LoadFailed();
            return;
        }

        if (memcmp("RIFF", fileHeader.type, 4))
        {
            MPMA::ErrorReport()<<"WavFileSource: File does not use a supported file header type: "<<filename<<"\n";
            LoadFailed();
            return;
        }

        if (memcmp("WAVE", fileHeader.format, 4))
        {
            MPMA::ErrorReport()<<"WavFileSource: File does not use a supported file format type: "<<filename<<"\n";
            LoadFailed();
            return;
        }

        //now read through the chunks and make sure we at least find the data
        bool foundData=false;
        bool foundFmt=false;
        isStereo=false;
        rate=44100;
        sourceBits=16;

        while (!feof(f) || !(foundData && foundFmt))
        {
            //read the chunk header
            struct
            {
                char id[4];
                uint32 size;
            } chunkHeader;

            if (fread(&chunkHeader, 8, 1, f))
            {
                if (memcmp(chunkHeader.id, "fmt ",4)==0) //fmt
                {
                    size_t fmtStartPos=ftell(f);
                    struct
                    {
                        uint16 format;
                        uint16 channels;
                        uint32 sampleRate;
                        uint32 byteRate;
                        uint16 blockAlign;
                        uint16 bitsPerSample;
                    } fmtData;

                    if (fread(&fmtData, sizeof(fmtData), 1, f))
                    {
                        foundFmt=true;

                        if (fmtData.format!=1)
                            MPMA::ErrorReport()<<"WavFileSource: File fmt chunk says it is not PCM, which is the only supported format.  Ignoring and trying anyways though: "<<filename<<"\n";

                        if (fmtData.channels==0 || fmtData.channels>2)
                            MPMA::ErrorReport()<<"WavFileSource: File has an unsupported number of channels.  Ignoring and treating as mono: "<<filename<<"\n";
                        isStereo=(fmtData.channels==2);

                        rate=8000;
                        if (fmtData.sampleRate<256 || fmtData.sampleRate>1024*1024)
                            MPMA::ErrorReport()<<"WavFileSource: File has an unsupported sample rate.  Ignoring and treating as 8000: "<<filename<<"\n";
                        else
                            rate=fmtData.sampleRate;

                        sourceBits=16;
                        if (!(fmtData.bitsPerSample==8 || fmtData.bitsPerSample==16))
                            MPMA::ErrorReport()<<"WavFileSource: File has an unsupported number of bits per channel.  Ignoring and treating as 16 bit: "<<filename<<"\n";
                        else
                            sourceBits=fmtData.bitsPerSample;
                    }

                    fseek(f, (long)(fmtStartPos+chunkHeader.size), SEEK_SET);
                }
                else if (memcmp(chunkHeader.id, "data", 4)==0) //data
                {
                    foundData=true;
                    dataStartSeekPos=ftell(f);

                    //verify we can read at least 1 sample more
                    char blah[16];
                    if (!fread(blah, GetSampleSize(), 1, f))
                    {
                        MPMA::ErrorReport()<<"WavFileSource: File does not contain any full sound samples: "<<filename<<"\n";
                        LoadFailed();
                        return;
                    }
                }
                else //unknown, skip it
                {
                    foundData=true;
                    fseek(f, chunkHeader.size, SEEK_CUR);
                }
            }
            else
                break;
        }

        if (!foundFmt)
            MPMA::ErrorReport()<<"WavFileSource: File does not contain a fmt chunk.  Ignoring and using defaults: "<<filename<<"\n";

        if (!foundData)
        {
            MPMA::ErrorReport()<<"WavFileSource: File does not contain a data chunk: "<<filename<<"\n";
            LoadFailed();
            return;
        }

        fseek(f, (long)(dataStartSeekPos), SEEK_SET);
    }

    //called when loading fails
    void WavFileSource::LoadFailed()
    {
        //close the file if it was opened
        if (fileHandle)
        {
            fclose((FILE*)fileHandle);
            fileHandle=0;
        }

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            bad=new3(WhiteNoiseSource(1.0f));
#else //use silence
            bad=new3(EmptySource());
#endif
    }

    WavFileSource::~WavFileSource()
    {
        //close our file
        if (fileHandle)
        {
            fclose((FILE*)fileHandle);
            fileHandle=0;
        }

        //free our bad source
        if (bad)
        {
            delete3(bad);
            bad=0;
        }
    }

    nuint WavFileSource::FillData(void *data, nuint maxSamples)
    {
        if (bad)
            return bad->FillData(data, maxSamples);

        MPMA::TakeSpinLock takeLock(lock);

        nuint destSampSize=GetSampleSize();
        if (sourceBits==16) //16 bit, no conversion needed
        {
            nuint ret=fread(data, destSampSize, maxSamples, (FILE*)fileHandle);
            if (ret<maxSamples)
                hitEof=true;
            return ret;
        }
        else //we'll pull in a piece then convert it
        {
            nuint srcSampSize=sourceBits/8;
            uint8 src[8192];
            nuint idealSamples=sizeof(src)/srcSampSize;
            if (idealSamples>maxSamples) idealSamples=maxSamples;

            nuint ret=fread(src, srcSampSize, idealSamples, (FILE*)fileHandle);
            if (ret<idealSamples)
                hitEof=true;

            //24 or 32 bit support could be implemented here by checking sourceBits (which our loader currently blocks).  So for now it can only be 8 bit here.
            uint16 *dst=(uint16*)data;
            nuint pieces=ret*destSampSize/2;
            for (nuint i=0; i<pieces; ++i)
                dst[i]=(src[i]-127)*256;

            return ret;
        }
    }

    nuint WavFileSource::GetRemainingSamples() const
    {
        if (bad)
            return bad->GetRemainingSamples();

        if (hitEof)
            return 0;

        return FINITE_DATA;
    }

    bool WavFileSource::IsStereo() const
    {
        if (bad)
            return bad->IsStereo();

        return isStereo;
    }

    nuint WavFileSource::GetSampleRate() const
    {
        if (bad)
            return bad->GetSampleRate();

        return rate;
    }

    void WavFileSource::Seek(nuint sampleNumber)
    {
        if (bad)
            return bad->Seek(sampleNumber);

        MPMA::TakeSpinLock takeLock(lock);

        hitEof=false;
        fseek((FILE*)fileHandle, (long)(dataStartSeekPos+sampleNumber*GetSampleSize()), SEEK_SET);
    }


    // -- VorbisFileSource

#ifdef AUDIO_OGG_VORBIS_ENABLED
    VorbisFileSource::VorbisFileSource(const std::string &filename): vorbisFile(0), rate(0), sourceBits(0), isStereo(false), hitEof(false), bad(0)
    {
       this->filename=filename;

        //open file
        vorbisFile=new3(OggVorbis_File);
        char *fnameCStr=(char*)filename.c_str(); //temp until the vorbis lib fixes its non-const filename parameter
        int ret=ov_fopen(fnameCStr, vorbisFile);
        if (ret) //call failed
        {
            MPMA::ErrorReport()<<"VorbisFileSource: Failed ov_fopen on file: "<<filename<<"\n";
            delete3(vorbisFile);
            vorbisFile=0;

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            bad=new3(WhiteNoiseSource(1.0f));
#else //use silence
            bad=new3(EmptySource());
#endif
            return;
        }

        //get info
        vorbis_info *info=ov_info(vorbisFile, -1);
        if (!info)
        {
            MPMA::ErrorReport()<<"VorbisFileSource: Failed ov_info on file: "<<filename<<".  Will make guesses.\n";
            isStereo=true;
            sourceBits=16;
            rate=44100;
        }
        else
        {
            isStereo=info->channels>=2;
            sourceBits=16;
            rate=info->rate;
        }
    }

    typedef int (*oggVorbisSeekFuncType)(void*, ogg_int64_t, int); //our definition isn't exact but is compatible

    VorbisFileSource::VorbisFileSource(const void *pFileInMemory, uint32 memoryLength): vorbisFile(0), rate(0), sourceBits(0), isStereo(false), hitEof(false), bad(0)
    {
        this->filename="Memory";

        //open file
        vorbisFile=new3(OggVorbis_File);

        ov_callbacks callbacks;
        callbacks.read_func=VorbisMemoryRead;
        callbacks.seek_func=(oggVorbisSeekFuncType)VorbisMemorySeek;
        callbacks.close_func=VorbisMemoryClose;
        callbacks.tell_func=VorbisMemoryTell;

        vorbisInMemory.Data.assign((unsigned char*)pFileInMemory, (unsigned char*)pFileInMemory+memoryLength);

        int ret=ov_open_callbacks(&vorbisInMemory, vorbisFile, 0, 0, callbacks);
        if (ret) //call failed
        {
            MPMA::ErrorReport()<<"VorbisFileSource: Failed ov_open_callbacks.\n";
            delete3(vorbisFile);
            vorbisFile=0;

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            bad=new3(WhiteNoiseSource(1.0f));
#else //use silence
            bad=new3(EmptySource());
#endif
            return;
        }

        //get info
        vorbis_info *info=ov_info(vorbisFile, -1);
        if (!info)
        {
            MPMA::ErrorReport()<<"VorbisFileSource: Failed ov_info on memory file.  Will make guesses.\n";
            isStereo=true;
            sourceBits=16;
            rate=44100;
        }
        else
        {
            isStereo=info->channels>=2;
            sourceBits=16;
            rate=info->rate;
        }
    }

    VorbisFileSource::~VorbisFileSource()
    {
        //close our file
        if (vorbisFile)
        {
            ov_clear(vorbisFile);
            delete3(vorbisFile);
            vorbisFile=0;
        }

        //free our bad source
        if (bad)
        {
            delete3(bad);
            bad=0;
        }
    }

    nuint VorbisFileSource::FillData(void *data, nuint maxSamples)
    {
        if (bad)
            return bad->FillData(data, maxSamples);

        MPMA::TakeSpinLock takeLock(lock);

        int unusedStreamValue=0;
        nsint sampleSize=GetSampleSize();
        long ret=ov_read(vorbisFile, (char*)data, (int)(maxSamples*sampleSize), 0, 2, 1, &unusedStreamValue);
        if (ret>0) //normal case
        {
            return ret/sampleSize;
        }
        else if (ret==OV_HOLE) //corrupt data... just return a 0 sample back, so they'll try again to continue with the rest
        {
            memset(data, 0, sampleSize);
            return 1;
        }
        else //0 in the normal case, a bad error otherwise
        {
            hitEof=true;
            return 0;
        }
    }

    nuint VorbisFileSource::GetRemainingSamples() const
    {
        if (bad)
            return bad->GetRemainingSamples();
        if (hitEof)
            return 0;

        uint64 total=ov_pcm_total(vorbisFile, -1);
        uint64 current=ov_pcm_tell(vorbisFile);
        uint64 remaining=total-current;
        if (remaining>0x00000000fffffff0)
            return FINITE_DATA;
        else
            return (nuint)remaining;
    }

    bool VorbisFileSource::IsStereo() const
    {
        if (bad)
            return bad->IsStereo();

        return isStereo;
    }

    nuint VorbisFileSource::GetSampleRate() const
    {
        if (bad)
            return bad->GetSampleRate();

        return rate;
    }

    void VorbisFileSource::Seek(nuint sampleNumber)
    {
        if (bad)
            return bad->Seek(sampleNumber);

        MPMA::TakeSpinLock takeLock(lock);

        hitEof=false;
        ov_pcm_seek(vorbisFile, sampleNumber);
    }

    size_t VorbisFileSource::VorbisMemoryRead(void *ptr, size_t size, size_t nmemb, void *datasource)
    {
        VorbisMemoryData *vorbisInMemory=(VorbisMemoryData*)datasource;

        size_t bytesToRead=size*nmemb;
        if (bytesToRead>vorbisInMemory->Data.size()-vorbisInMemory->Cursor)
            bytesToRead=vorbisInMemory->Data.size()-vorbisInMemory->Cursor;

        if (bytesToRead==0)
            return 0;

        memcpy(ptr, &vorbisInMemory->Data[vorbisInMemory->Cursor], bytesToRead);
        vorbisInMemory->Cursor+=(uint32)bytesToRead;

        return bytesToRead;
    }

    int VorbisFileSource::VorbisMemorySeek(void *datasource, sint64 offset, int whence)
    {
        VorbisMemoryData *vorbisInMemory=(VorbisMemoryData*)datasource;

        sint64 seekTarget=0;

        if (whence==SEEK_SET)
            seekTarget=offset;
        else if (whence==SEEK_CUR)
            seekTarget=vorbisInMemory->Cursor+offset;
        else if (whence==SEEK_END)
            seekTarget=vorbisInMemory->Data.size()-offset;
        else
            return -1;

        if (seekTarget<0 || seekTarget>(sint64)vorbisInMemory->Data.size())
            return -1;

        vorbisInMemory->Cursor=(uint32)seekTarget;

        return 0;
    }

    int VorbisFileSource::VorbisMemoryClose(void *datasource)
    {
        return 0;
    }

    long VorbisFileSource::VorbisMemoryTell(void *datasource)
    {
        VorbisMemoryData *vorbisInMemory=(VorbisMemoryData*)datasource;
        return vorbisInMemory->Cursor;
    }

#endif //AUDIO_OGG_VORBIS_ENABLED


    // -- FlacFileSource

#ifdef AUDIO_OGG_FLAC_ENABLED

    //Implementation of the flac reader
    class InternalMpmaFlacDecoder: public FLAC::Decoder::File
    {
    public:
        int SampleRate;
        int Channels;
        int BitsPerSample;
        std::vector<unsigned char> Bytes; //blob of bytes spit out by libflac
        nuint BytesStart; //start index of the next unread data in Bytes

        InternalMpmaFlacDecoder()
        {
            SampleRate=44100;
            Channels=1;
            BitsPerSample=8;
            BytesStart=0;
        }

        //write data callback
        virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32* const buffer[])
        {
            //MPMA::ErrorReport()<<"write_callback, blocksize="<<frame->header.blocksize<<"\n";

            if (BitsPerSample==8) //8 bits per sample
            {
                if (Channels==1) //mono
                {
                    Bytes.reserve(Bytes.size()+frame->header.blocksize);
                    for (nuint s=0; s<frame->header.blocksize; ++s)
                    {
                        Bytes.push_back(((unsigned char*)&buffer[0][s])[0]);
                    }
                }
                else //pull in first two channels as stereo
                {
                    for(nuint s=0; s<frame->header.blocksize; ++s)
                    {
                        Bytes.push_back(((unsigned char*)&buffer[0][s])[0]);
                        Bytes.push_back(((unsigned char*)&buffer[1][s])[0]);
                    }
                }
            }
            else if (BitsPerSample==16) //16 bits per sample
            {
                if (Channels==1) //mono
                {
                    Bytes.reserve(Bytes.size()+frame->header.blocksize);
                    for (nuint s=0; s<frame->header.blocksize; ++s)
                    {
                        Bytes.push_back(((unsigned char*)&buffer[0][s])[0]);
                        Bytes.push_back(((unsigned char*)&buffer[0][s])[1]);
                    }
                }
                else //pull in first two channels as stereo
                {
                    for(nuint s=0; s<frame->header.blocksize; ++s)
                    {
                        Bytes.push_back(((unsigned char*)&buffer[0][s])[0]);
                        Bytes.push_back(((unsigned char*)&buffer[0][s])[1]);
                        Bytes.push_back(((unsigned char*)&buffer[1][s])[0]);
                        Bytes.push_back(((unsigned char*)&buffer[1][s])[1]);
                    }
                }
            }
            else //unsupported bits per sample
            {
                MPMA::ErrorReport()<<"FLAC stream has an unspported number of bits per sample: "<<BitsPerSample<<"\n";
                return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
            }

            return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
        }

        //error callback
        void error_callback(::FLAC__StreamDecoderErrorStatus status)
        {
            MPMA::ErrorReport()<<"FLAC error_callback called.\n";
        }

        //metadata callback
        void metadata_callback(const ::FLAC__StreamMetadata *metadata)
        {
            //MPMA::ErrorReport()<<"metadata_callback\n";
            if(metadata->type==FLAC__METADATA_TYPE_STREAMINFO)
            {
                SampleRate=metadata->data.stream_info.sample_rate;
                Channels=metadata->data.stream_info.channels;
                BitsPerSample=metadata->data.stream_info.bits_per_sample;
            }
        }
    };

    //Implementation of the actual source

    FlacFileSource::FlacFileSource(const std::string &filename): flac(0), hitEof(false), bad(0)
    {
        this->filename=filename;

        //open file
        flac=new3(InternalMpmaFlacDecoder);
        ::FLAC__StreamDecoderInitStatus ret=flac->init(filename);
        if (ret!=FLAC__STREAM_DECODER_INIT_STATUS_OK) //call failed
        {
            MPMA::ErrorReport()<<"FlacFileSource: Failed init_ogg on file: "<<filename;
            if (ret==FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER) MPMA::ErrorReport()<<": UNSUPPORTED_CONTAINER";
            if (ret==FLAC__STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS) MPMA::ErrorReport()<<": INVALID_CALLBACKS";
            if (ret==FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE) MPMA::ErrorReport()<<": ERROR_OPENING_FILE";
            MPMA::ErrorReport()<<"\n";
            delete3(flac);
            flac=0;

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            bad=new3(WhiteNoiseSource(1.0f));
#else //use silence
            bad=new3(EmptySource());
#endif
            return;
        }

        //decode the first bit
        flac->process_until_end_of_metadata();
    }

    FlacFileSource::~FlacFileSource()
    {
        //close our file
        if (flac)
        {
            delete3(flac);
            flac=0;
        }

        //free our bad source
        if (bad)
        {
            delete3(bad);
            bad=0;
        }
    }

    nuint FlacFileSource::FillData(void *data, nuint maxSamples)
    {
        //MPMA::ErrorReport()<<"FillData\n";
        if (bad)
            return bad->FillData(data, maxSamples);

        MPMA::TakeSpinLock takeLock(lock);

        int sampleSize=GetSampleSize();
        int maxBytes=sampleSize*maxSamples;

        //if we already have data waiting in the buffer, just consume that
        int byteBufferLeft=flac->Bytes.size()-flac->BytesStart;

        if (byteBufferLeft==0) //need to decode some more data
        {
            //MPMA::ErrorReport()<<"process_single\n";
            bool procSingleRet=flac->process_single();
            //MPMA::ErrorReport()<<"process_single returned: "<<procSingleRet<<"\n";
            if (!procSingleRet)
            {
                hitEof=true;
                return 0;
            }

            if (flac->get_state()==FLAC__STREAM_DECODER_END_OF_STREAM || flac->get_state()==FLAC__STREAM_DECODER_ABORTED)
            {
                hitEof=true;
                return 0;
            }
        }

        //transfer buffer bytes out
        if (maxBytes>byteBufferLeft) maxBytes=byteBufferLeft;
        maxSamples=maxBytes/sampleSize;

        memcpy(data, &flac->Bytes[flac->BytesStart], maxBytes);
        flac->BytesStart+=maxBytes;
        if (flac->Bytes.size()==flac->BytesStart) //all used, so clear out buffer
        {
            flac->Bytes.clear();
            flac->BytesStart=0;
        }

        return maxSamples;
    }

    nuint FlacFileSource::GetRemainingSamples() const
    {
        if (bad)
            return bad->GetRemainingSamples();
        if (hitEof)
            return 0;

        return FINITE_DATA;
    }

    bool FlacFileSource::IsStereo() const
    {
        if (bad)
            return bad->IsStereo();

        return flac->Channels>=2;
    }

    nuint FlacFileSource::GetSampleRate() const
    {
        if (bad)
            return bad->GetSampleRate();

        return flac->SampleRate;
    }

    void FlacFileSource::Seek(nuint sampleNumber)
    {
        if (bad)
            return bad->Seek(sampleNumber);

        MPMA::TakeSpinLock takeLock(lock);

        hitEof=false;
        flac->seek_absolute(sampleNumber);
    }

#endif


    // -- OpusFileSource

#ifdef AUDIO_OGG_OPUS_ENABLED

    OpusFileSource::OpusFileSource(const std::string &filename): bad(nullptr), hitEof(false), needStereoDownmix(false)
    {
        this->filename=filename;

        opusFile=op_open_file(filename.c_str(), nullptr);
        if (!opusFile)
        {
            MPMA::ErrorReport()<<"OpusFileSource: op_open_file failed on "<<filename<<"\n";

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            bad=new3(WhiteNoiseSource(1.0f));
#else //use silence
            bad=new3(EmptySource());
#endif
            return;
        }

        SetInfo();
    }

    OpusFileSource::OpusFileSource(const void *pFileInMemory, uint32 memoryLength): bad(nullptr), hitEof(false), needStereoDownmix(false)
    {
        this->filename="Memory";

        opusFile=op_open_memory((const unsigned char*)pFileInMemory, memoryLength, nullptr);
        if (!opusFile)
        {
            MPMA::ErrorReport()<<"OpusFileSource: op_open_memory failed.\n";

#ifdef USE_STATIC_SOUND_FOR_AUDIO_ERRORS
            bad=new3(WhiteNoiseSource(1.0f));
#else //use silence
            bad=new3(EmptySource());
#endif
            return;
        }

        SetInfo();
    }

    OpusFileSource::~OpusFileSource()
    {
        if (opusFile)
        {
            op_free(opusFile);
            opusFile=nullptr;
        }

        if (bad)
        {
            delete3(bad);
            bad=nullptr;
        }
    }

    void OpusFileSource::SetInfo()
    {
        totalSamples=op_pcm_total(opusFile, -1);

        int maxChannels=1;
        int totalLinks=op_link_count(opusFile);
        for (int l=0; l<totalLinks; ++l)
        {
            int linkChannels=op_channel_count(opusFile, l);
            if (linkChannels>maxChannels)
                maxChannels=linkChannels;
        }

        if (maxChannels>2)
        {
            needStereoDownmix=true;
            maxChannels=2;
        }
        isStereo=(maxChannels==2);

        sampleRate=48000; //opus is always 48k? wasn't clear in the opusfile docs

        
    }

    nuint OpusFileSource::FillData(void *data, nuint maxSamples)
    {
        if (bad)
            return bad->FillData(data, maxSamples);

        MPMA::TakeSpinLock takeLock(lock);

        int ret;
        if (!needStereoDownmix)
        {
            ret=op_read(opusFile, (opus_int16*)data, (int)maxSamples*(IsStereo()?2:1), nullptr);
        }
        else
        {
            ret=op_read_stereo(opusFile, (opus_int16*)data, (int)maxSamples*(IsStereo()?2:1));
        }

        if (ret>0)
        {
            return ret;
        }
        if (ret==OP_HOLE) //corrupt data maybe.. corrupt data maybe.. return back a 0 sample so they'll try again to continue past it
        {
            memset(data, 0, GetSampleSize());
            return 1;
        }
        else //0 or error
        {
            hitEof=true;
            return 0;
        }
    }

    nuint OpusFileSource::GetRemainingSamples() const
    {
        if (bad)
            return bad->GetRemainingSamples();
        if (hitEof)
            return 0;

        if (totalSamples==OP_EINVAL)
            return FINITE_DATA;

        ogg_int64_t nextSample=op_pcm_tell(opusFile);
        if (nextSample==OP_EINVAL)
            return FINITE_DATA;

        return (nuint)(totalSamples-nextSample);
    }

    bool OpusFileSource::IsStereo() const
    {
        if (bad)
            return bad->IsStereo();

        return isStereo;
    }

    nuint OpusFileSource::GetSampleRate() const
    {
        if (bad)
            return bad->GetSampleRate();

        return sampleRate;
    }

    void OpusFileSource::Seek(nuint sampleNumber)
    {
        if (bad)
            return bad->Seek(sampleNumber);

        MPMA::TakeSpinLock takeLock(lock);

        hitEof=false;
        op_pcm_seek(opusFile, sampleNumber);
    }

#endif
}

#endif //#ifdef MPMA_COMPILE_AUDIO
