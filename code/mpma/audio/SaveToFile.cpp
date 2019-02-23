//!\file SaveToFile.cpp  Save a sound source to a file.
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"
#include "../base/DebugRouter.h"

#ifdef MPMA_COMPILE_AUDIO

#include "SaveToFile.h"
#include <fstream>
#include <array>

#ifdef AUDIO_OGG_VORBIS_ENABLED
    #include <vorbis/vorbisfile.h>
    #include <vorbis/vorbisenc.h>
#endif

#ifdef AUDIO_OGG_OPUS_ENABLED
    #include <opus/opus.h>
    #include <opus/opusfile.h>
#endif

namespace
{
#if defined(AUDIO_OGG_VORBIS_ENABLED) || defined (AUDIO_OGG_OPUS_ENABLED)
    class OggStreamWrapper
    {
    public:
        OggStreamWrapper()
        {
            memset(&oggStream, 0, sizeof(oggStream));
            ogg_stream_init(&oggStream, rand());
        }

        ~OggStreamWrapper()
        {
            ogg_stream_clear(&oggStream);
        }

        operator ogg_stream_state*()
        {
            return &oggStream;
        }

        void PageOutAppend(std::vector<uint8> &data, bool forceAll)
        {
            while (true)
            {
                ogg_page oggPage;
                int ospoRet;
                if (forceAll)
                    ospoRet=ogg_stream_flush(&oggStream, &oggPage);
                else
                    ospoRet=ogg_stream_pageout(&oggStream, &oggPage);
                if (ospoRet==0)
                    break;

                data.insert(data.end(), oggPage.header, oggPage.header+oggPage.header_len);
                data.insert(data.end(), oggPage.body, oggPage.body+oggPage.body_len);

                if (ogg_page_eos(&oggPage))
                    break;
            }
        }

    private:
        ogg_stream_state oggStream;
    };
#endif

#if defined(AUDIO_OGG_VORBIS_ENABLED)
    //thin wrapper around all of the vorbis objects needed just to ensure they get properly cleaned up on failures
    class VorbisWrapper
    {
    public:
        VorbisWrapper()
        {
            memset(&vi, 0, sizeof(vi));
            memset(&dsp, 0, sizeof(dsp));
            memset(&vorbisBlock, 0, sizeof(vorbisBlock));
            memset(&comment, 0, sizeof(comment));

            viInited=false;
            dspInited=false;
            vorbisBlockInited=false;
            commentInited=false;
        }

        bool Init(long channels, long rate, float quality)
        {
            vorbis_info_init(&vi);
            viInited=true;

            if (vorbis_encode_init_vbr(&vi, channels, rate, quality))
            {
                MPMA::ErrorReport()<<"vorbis_encode_init_vbr failed.\n";
                return false;
            }

            if (vorbis_analysis_init(&dsp, &vi))
            {
                MPMA::ErrorReport()<<"vorbis_analysis_init failed\n";
                return false;
            }
            dspInited=true;

            if (vorbis_block_init(&dsp, &vorbisBlock))
            {
                MPMA::ErrorReport()<<"vorbis_block_init failed\n";
                return false;
            }
            vorbisBlockInited=true;

            vorbis_comment_init(&comment);
            commentInited=true;

            return true;
        }

        ~VorbisWrapper()
        {
            if (commentInited)
                vorbis_comment_clear(&comment);

            if (vorbisBlockInited)
                vorbis_block_clear(&vorbisBlock);

            if (dspInited)
                vorbis_dsp_clear(&dsp);

            if (viInited)
                vorbis_info_clear(&vi);
        }

        vorbis_info vi;
        bool viInited;
        vorbis_dsp_state dsp;
        bool dspInited;
        vorbis_block vorbisBlock;
        bool vorbisBlockInited;
        vorbis_comment comment;
        bool commentInited;
    };
#endif

#ifdef AUDIO_OGG_OPUS_ENABLED
    struct OpusHeader
    {
        uint8 Version;
        uint8 Channels;
        uint16 Preskip;
        uint32 OriginalSampleRate;
        uint16 Gain;
        uint8 ChannelMapping;

        OpusHeader()
        {
            Version=1;
            Channels=0;
            Preskip=0;
            OriginalSampleRate=0;
            Gain=0;
            ChannelMapping=0; //1-2 channels, mono or stereo, no mapping table.
        }

        std::vector<uint8> CreateOggPacketBytes()
        {
            std::vector<uint8> blob;
            std::array<sint8, 8> magicHeader={'O','p','u','s','H','e','a','d'};
            blob.insert(blob.end(), magicHeader.begin(), magicHeader.end());
            blob.insert(blob.end(), &Version, &ChannelMapping+1);
            return blob;
        }
    };

    class OpusComments
    {
    public:
        void AddComment(const std::string &key, const std::string &val)
        {
            comments.emplace_back(std::pair<std::string, std::string>(key, val));
        }

        std::vector<uint8> GetOggPacketBytes()
        {
            std::vector<uint8> blob;
            std::array<sint8, 8> magicHeader={'O','p','u','s','T','a','g','s'};
            blob.insert(blob.end(), magicHeader.begin(), magicHeader.end());
            
            std::string vender="MPMA Ghetto Encoder";
            uint32 venderLength=(uint32)vender.length();
            blob.insert(blob.end(), (uint8*)&venderLength, (uint8*)(&venderLength+1));
            blob.insert(blob.end(), vender.begin(), vender.end());

            uint32 lengthCount=(uint32)comments.size();
            blob.insert(blob.end(), (uint8*)&lengthCount, (uint8*)(&lengthCount+1));

            for (auto c=comments.begin(); c!=comments.end(); ++c)
            {
                std::string commentString=c->first+"="+c->second;
                uint32 commentLength=(uint32)commentString.length();
                blob.insert(blob.end(), (uint8*)&commentLength, (uint8*)(&commentLength+1));
                blob.insert(blob.end(), commentString.begin(), commentString.end());
            }

            return blob;
        }

    private:
        std::list<std::pair<std::string, std::string>> comments;
    };
#endif
}

namespace AUDIO
{
    //Saves a finite audio source to a wave file.  If source is infinite or on failure, returns false.
    bool SaveSourceToWaveFile(const std::string &filename, std::shared_ptr<AUDIO::Source> source)
    {
        if (source->GetRemainingSamples()==INFINITE_DATA)
            return false;

        //read all the data to start since we need to know the size for the header
        std::vector<uint8> fileData;

        while (source->GetRemainingSamples()>0)
        {
            const int maxSamples=1024;
            sint16 sourceData[maxSamples*2*2];
            int sampleCount=(int)source->FillData(sourceData, maxSamples);
            fileData.insert(fileData.end(), (char*)sourceData, (char*)(sourceData+sampleCount*(source->IsStereo()?2:1)));
        }

        //write RIFF header
        std::ofstream outFile(filename, std::ios_base::out|std::ios_base::binary);
        if (!outFile.is_open())
            return false;

        outFile.write("RIFF", 4);
        uint32 totalSize=4+24+8+(uint32)fileData.size();
        outFile.write((char*)&totalSize, 4);

        //write WAVE header
        outFile.write("WAVE", 4);

        //fmt chunk of WAVE
        outFile.write("fmt ", 4);
        uint32 fmtSize=16;
        outFile.write((char*)&fmtSize, 4);
        uint16 pcmConstant=1;
        outFile.write((char*)&pcmConstant, 2);
        uint16 channelCount=source->IsStereo()?2:1;
        outFile.write((char*)&channelCount, 2);
        uint32 samplesPerSecond=(uint32)source->GetSampleRate();
        outFile.write((char*)&samplesPerSecond, 4);
        uint32 bytesPerSecond=samplesPerSecond*channelCount*2;
        outFile.write((char*)&bytesPerSecond, 4);
        uint16 blockAlign=channelCount*2;
        outFile.write((char*)&blockAlign, 2);
        uint16 bitsPerSample=16;
        outFile.write((char*)&bitsPerSample, 2);

        //data chunk of WAVE
        outFile.write("data", 4);
        uint32 dataSize=(uint32)fileData.size();
        outFile.write((char*)&dataSize, 4);
        outFile.write((char*)&fileData[0], fileData.size());

        return true;
    }

#ifdef AUDIO_OGG_VORBIS_ENABLED

    //Saves a finite audio source to an ogg vorbis file using variable bit rate encoding.  If source is infinite or on failure, returns false.  vbrQuality may be from -0.1 to 1.0.  0.4 is roughly 128kbps.
    bool SaveSourceToVorbisFileVBR(const std::string &filename, std::shared_ptr<AUDIO::Source> source, float vbrQuality, const std::list<std::pair<std::string, std::string>> &fileComments)
    {
        if (source->GetRemainingSamples()==INFINITE_DATA)
            return false;

        std::vector<uint8> fileData;
        if (source->GetRemainingSamples()!=FINITE_DATA) //give an initial guess to reduce reallocs
            fileData.reserve((source->IsStereo()?2:1)*2*source->GetRemainingSamples()/10);
        else
            fileData.reserve(65536);

        //setup stream and headers
        OggStreamWrapper oggStream;

        VorbisWrapper vw;
        if (!vw.Init(source->IsStereo()?2:1, (long)source->GetSampleRate(), vbrQuality))
            return false;

        for (auto fc=fileComments.begin(); fc!=fileComments.end(); ++fc)
            vorbis_comment_add_tag(&vw.comment, fc->first.c_str(), fc->second.c_str());

        int ret=0;
        ogg_packet headerPacket0;
        ogg_packet headerPacket1;
        ogg_packet headerPacket2;
        ret=vorbis_analysis_headerout(&vw.dsp, &vw.comment, &headerPacket0, &headerPacket1, &headerPacket2);
        if (ret)
            return false;

        //write header
        ogg_stream_packetin(oggStream, &headerPacket0);
        ogg_stream_packetin(oggStream, &headerPacket1);
        ogg_stream_packetin(oggStream, &headerPacket2);

        oggStream.PageOutAppend(fileData, true);

        //write the stream data
        bool haveWrittenEndOfStreamPacket=false;
        while (!haveWrittenEndOfStreamPacket)
        {
            //submit a block of data to the encoder
            const int maxSamples=1024;
            sint16 sourceData[maxSamples*2*2];
            int sampleCount=(int)source->FillData(sourceData, maxSamples);
            if (sampleCount!=0)
            {
                float **vorbisBuffers=vorbis_analysis_buffer(&vw.dsp, maxSamples);
                if (source->IsStereo())
                {
                    for (int s=0; s<sampleCount; ++s)
                    {
                        vorbisBuffers[0][s]=sourceData[s*2+0]/32768.0f;
                        vorbisBuffers[1][s]=sourceData[s*2+1]/32768.0f;
                    }
                }
                else
                {
                    for (int s=0; s<sampleCount; ++s)
                        vorbisBuffers[0][s]=sourceData[s]/32768.0f;
                }
            }
            else
            {
                haveWrittenEndOfStreamPacket=true;
            }

            vorbis_analysis_wrote(&vw.dsp, sampleCount);

            //tell the encoder to process what we gave it
            ret=1;
            while (ret==1)
            {
                ret=vorbis_analysis_blockout(&vw.dsp, &vw.vorbisBlock);
                if (ret==1)
                {
                    vorbis_analysis(&vw.vorbisBlock, 0);
                    vorbis_bitrate_addblock(&vw.vorbisBlock);

                    ogg_packet oggPacket;
                    while (vorbis_bitrate_flushpacket(&vw.dsp, &oggPacket))
                    {
                        ogg_stream_packetin(oggStream, &oggPacket);

                        oggStream.PageOutAppend(fileData, false);
                    }
                }
            }
        }

        oggStream.PageOutAppend(fileData, true);

        //save to file
        if (fileData.empty())
            return false;

        std::ofstream outFile(filename, std::ios_base::out|std::ios_base::binary);
        if (!outFile.is_open())
            return false;

        outFile.write((char*)&fileData[0], fileData.size());

        return true;
    }

#endif

#ifdef AUDIO_OGG_OPUS_ENABLED


    //Saves a finite audio source to an ogg opus file using variable bit rate encoding.  If source is infinite or on failure, returns false.
    bool SaveSourceToOpusFileVBR(const std::string &filename, std::shared_ptr<AUDIO::Source> source, const std::list<std::pair<std::string, std::string>> &fileComments)
    {
        if (source->GetRemainingSamples()==INFINITE_DATA)
            return false;

        std::vector<uint8> fileData;
        if (source->GetRemainingSamples()!=FINITE_DATA) //give an initial guess to reduce reallocs
            fileData.reserve((source->IsStereo()?2:1)*2*source->GetRemainingSamples()/10);
        else
            fileData.reserve(65536);

        //TODO: figure out resampling since opus encoder only supports a few very specific sample rates.

        //set up encoder
        std::vector<uint8> encoderState;
        encoderState.resize(opus_encoder_get_size(source->IsStereo()?2:1));
        OpusEncoder *opusEncoder=(OpusEncoder*)encoderState.data();
        int ret=opus_encoder_init(opusEncoder, (uint32)source->GetSampleRate(), source->IsStereo()?2:1, OPUS_APPLICATION_AUDIO);
        if (ret!=OPUS_OK)
            return false;

        int sampleRate=(int)source->GetSampleRate();
        const int bytesPerSample=2*(source->IsStereo()?2:1);

        std::array<int, 6> allowedOpusFrameSizes;
        allowedOpusFrameSizes[0]=(int)sampleRate*60/1000; //60ms
        allowedOpusFrameSizes[1]=(int)sampleRate*40/1000;
        allowedOpusFrameSizes[2]=(int)sampleRate*20/1000;
        allowedOpusFrameSizes[3]=(int)sampleRate*10/1000;
        allowedOpusFrameSizes[4]=(int)sampleRate*5/1000;
        allowedOpusFrameSizes[5]=(int)sampleRate*5/2000; //2.5ms

        //generate ogg opus headers
        OggStreamWrapper oggStream;

        OpusHeader header;
        header.Channels=source->IsStereo()?2:1;
        header.OriginalSampleRate=(uint32)source->GetSampleRate();
        std::vector<uint8> oggPacketData=header.CreateOggPacketBytes();

        ogg_packet oggPacket={0};
        oggPacket.packet=oggPacketData.data();
        oggPacket.bytes=(long)oggPacketData.size();
        oggPacket.b_o_s=1;

        ogg_stream_packetin(oggStream, &oggPacket);
        oggStream.PageOutAppend(fileData, false);

        OpusComments opusComments;
        for (auto comment=fileComments.begin(); comment!=fileComments.end(); ++comment)
            opusComments.AddComment(comment->first, comment->second);
        oggPacketData=opusComments.GetOggPacketBytes();

        oggPacket.packet=oggPacketData.data();
        oggPacket.bytes=(long)oggPacketData.size();
        oggPacket.b_o_s=0;
        ++oggPacket.packetno;

        ogg_stream_packetin(oggStream, &oggPacket);
        oggStream.PageOutAppend(fileData, false);

        //encode data
        std::array<uint8, 4096> encoderOutput;
        std::vector<uint8> bufferedData;
        int bufferedDataInUse=0;
        const int &maxSamplesPerEncode=allowedOpusFrameSizes[0];
        bufferedData.resize(maxSamplesPerEncode*bytesPerSample);

        while (true)
        {
            int samplesToRequest=maxSamplesPerEncode-bufferedDataInUse/bytesPerSample;
            int ret=(int)source->FillData(bufferedData.data()+bufferedDataInUse, samplesToRequest);
            if (ret==0)
                break;
            bufferedDataInUse+=ret*bytesPerSample;

            if (bufferedDataInUse/bytesPerSample==maxSamplesPerEncode)
            {
                ret=opus_encode(opusEncoder, (opus_int16*)bufferedData.data(), maxSamplesPerEncode, encoderOutput.data(), (opus_int32)encoderOutput.size());
                if (ret<0)
                    return false;

                oggPacket.packet=encoderOutput.data();
                oggPacket.bytes=ret;
                oggPacket.b_o_s=0;
                oggPacket.e_o_s=0;
                ++oggPacket.packetno;
                oggPacket.granulepos+=maxSamplesPerEncode*48000/sampleRate;

                ogg_stream_packetin(oggStream, &oggPacket);
                oggStream.PageOutAppend(fileData, false);

                bufferedDataInUse=0;
            }
        }

        //encode last piece of data, padding the end with 0
        int samplesForFinalFrame=bufferedDataInUse/bytesPerSample;
        bufferedData.resize(samplesForFinalFrame);
        for (int i=(int)allowedOpusFrameSizes.size()-1; i>=0; --i)
        {
            if (samplesForFinalFrame==allowedOpusFrameSizes[i])
                break;
            else if (allowedOpusFrameSizes[i]>samplesForFinalFrame)
            {
                samplesForFinalFrame=allowedOpusFrameSizes[i];
                break;
            }
        }

        while ((int)bufferedData.size()<samplesForFinalFrame)
            bufferedData.push_back(0);

        if (samplesForFinalFrame>0)
        {
            int ret=opus_encode(opusEncoder, (opus_int16*)bufferedData.data(), samplesForFinalFrame, encoderOutput.data(), (opus_int32)encoderOutput.size());
            if (ret<0)
                return false;

            oggPacket.packet=encoderOutput.data();
            oggPacket.bytes=ret;
            oggPacket.b_o_s=0;
            oggPacket.e_o_s=1;
            ++oggPacket.packetno;
            oggPacket.granulepos+=samplesForFinalFrame*48000/sampleRate;
            ogg_stream_packetin(oggStream, &oggPacket);
        }
        else
        {
            oggPacket.packet=encoderOutput.data();
            oggPacket.bytes=0;
            oggPacket.b_o_s=0;
            oggPacket.e_o_s=1;
            ++oggPacket.packetno;
            ogg_stream_packetin(oggStream, &oggPacket);
        }

        oggStream.PageOutAppend(fileData, true);

        //write file
        if (fileData.empty())
            return false;

        std::ofstream outFile(filename, std::ios_base::out|std::ios_base::binary);
        if (!outFile.is_open())
            return false;

        outFile.write((char*)&fileData[0], fileData.size());

        return true;
    }

#endif
}

#endif //#ifdef MPMA_COMPILE_AUDIO
