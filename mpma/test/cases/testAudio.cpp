//tests audio functions
//Luke Lenhart, 2010-2011
//See /docs/License.txt for details on how this code may be used.


#include "../UnitTest.h"
#include "audio/Listener.h"
#include "audio/Source.h"
#include "audio/Player.h"
#include "audio/SaveToFile.h"
#include "geo/Geo.h"
#include "base/Thread.h"
#include "base/Timer.h"

#include <string>
#include <cmath>
/*
#ifdef DECLARE_TESTS_CODE
class AudioStaticBuffer: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::WhiteNoiseSource> src=std::make_shared<AUDIO::WhiteNoiseSource>(1.0f, 44100, 2);
        std::shared_ptr<AUDIO::StaticBuffer> buf=std::make_shared<AUDIO::StaticBuffer>(src);

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        MPMA::Timer timer;
        plr->PlayStatic(buf, AUDIO::FIXED);
        double timeToStart=timer.Step();
        std::cout<<"Time to call PlayStatic: "<<timeToStart<<std::endl;

        while (plr->IsPlaying())
            MPMA::Sleep(50);

        double timePlayed=timer.Step();
        if (std::fabs(timePlayed-1.0)>0.1)
        {
            std::cout<<"Time played was too far from expected 1 second: "<<timePlayed<<std::endl;
            return false;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioStaticBuffer)

#ifdef DECLARE_TESTS_CODE
class AudioStreamBufferManual: public UnitTest
{
public:
    bool Run()
    {
        AUDIO::SetBackgroundStreaming(false);

        std::shared_ptr<AUDIO::WhiteNoiseSource> src=std::make_shared<AUDIO::WhiteNoiseSource>(1.0f, 44100, 2);

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        MPMA::Timer timer;
        plr->PlayStreaming(src, AUDIO::FIXED);
        double timeToStart=timer.Step();
        std::cout<<"Time to call PlayStreaming: "<<timeToStart<<std::endl;

        while (plr->IsPlaying())
        {
            AUDIO::UpdateStreams();
            MPMA::Sleep(50);
        }

        double timePlayed=timer.Step();
        if (std::fabs(timePlayed-1.0)>0.15)
        {
            std::cout<<"Time played was too far from expected 1 second: "<<timePlayed<<std::endl;
            AUDIO::SetBackgroundStreaming(true);
            return false;
        }

        MPMA::Sleep(250);

        AUDIO::SetBackgroundStreaming(true);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioStreamBufferManual)

#ifdef DECLARE_TESTS_CODE
class AudioStreamBufferBackground: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::WhiteNoiseSource> src=std::make_shared<AUDIO::WhiteNoiseSource>(1.0f, 44100, 2);

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        MPMA::Timer timer;
        plr->PlayStreaming(src, AUDIO::FIXED);
        double timeToStart=timer.Step();
        std::cout<<"Time to call PlayStreaming: "<<timeToStart<<std::endl;

        while (plr->IsPlaying())
            MPMA::Sleep(50);

        double timePlayed=timer.Step();
        if (std::fabs(timePlayed-1.0)>0.15)
        {
            std::cout<<"Time played was too far from expected 1 second: "<<timePlayed<<std::endl;
            return false;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioStreamBufferBackground)

#ifdef DECLARE_TESTS_CODE
class AudioWavSource: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;
        good=TestWav("data/testAudioStereo16.wav") || good;
        good=TestWav("data/testAudioMono16.wav") || good;
        good=TestWav("data/testAudioMono8.wav") || good;
        good=TestWav("data/testAudioMono8_8000.wav") || good;
        return good;
    }

private:
    bool TestWav(const std::string &fname)
    {
        std::cout<<"Testing "<<fname<<std::endl;
        std::shared_ptr<AUDIO::WavFileSource> src=std::make_shared<AUDIO::WavFileSource>(fname);
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed for "<<fname<<std::endl;
            return false;
        }

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        plr->PlayStreaming(src, AUDIO::FIXED);

        int count=0;
        while (plr->IsPlaying())
        {
            ++count;
            MPMA::Sleep(50);
        }

        if (count<20) //stopped early?
        {
            std::cout<<"Sound did not play for long enough..."<<std::endl;
            return false;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioWavSource)

#ifdef AUDIO_OGG_VORBIS_ENABLED
#ifdef DECLARE_TESTS_CODE
class AudioWavSave: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::WavFileSource> src=std::make_shared<AUDIO::WavFileSource>("data/testAudioStereo16.wav");
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed for source."<<std::endl;
            return false;
        }

        MPMA::FileUtils::CreateDirectory("temp");

        if (!AUDIO::SaveSourceToWaveFile("temp/testSaveWave.wav", src))
        {
            std::cout<<"SaveSourceToWaveFile failed."<<std::endl;
            return false;
        }

        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioWavSave)
#endif

#ifdef AUDIO_OGG_VORBIS_ENABLED
#ifdef DECLARE_TESTS_CODE
class AudioVorbisSource: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::VorbisFileSource> src=std::make_shared<AUDIO::VorbisFileSource>("data/testAudioVorbis.ogg");
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed."<<std::endl;
            return false;
        }

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        plr->PlayStreaming(src, AUDIO::FIXED);

        int count=0;
        while (plr->IsPlaying())
        {
            ++count;
            MPMA::Sleep(50);
        }

        if (count<20) //stopped early?
        {
            std::cout<<"Sound did not play for long enough..."<<std::endl;
            return false;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioVorbisSource)
#endif

*/
#ifdef AUDIO_OGG_VORBIS_ENABLED
#ifdef DECLARE_TESTS_CODE
class AudioVorbisSave: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::WavFileSource> src=std::make_shared<AUDIO::WavFileSource>("data/testAudioStereo16.wav");
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed for source."<<std::endl;
            return false;
        }

        MPMA::FileUtils::CreateDirectory("temp");

        std::list<std::pair<std::string, std::string>> fileComments;
        fileComments.push_back(std::pair<std::string, std::string>("Title", "Rar"));
        fileComments.push_back(std::pair<std::string, std::string>("Comment", "Rum"));

        if (!AUDIO::SaveSourceToVorbisFileVBR("temp/testSaveVorbis.ogm", src, 0.4f, fileComments))
        {
            std::cout<<"SaveSourceToVorbisFileVBR failed."<<std::endl;
            return false;
        }

        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioVorbisSave)
#endif

#ifdef AUDIO_OGG_FLAC_ENABLED
#ifdef DECLARE_TESTS_CODE
class AudioFlacSource: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::FlacFileSource> src=std::make_shared<AUDIO::FlacFileSource>("data/testAudioFlac.flac");
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed."<<std::endl;
            return false;
        }

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        plr->PlayStreaming(src, AUDIO::FIXED);

        int count=0;
        while (plr->IsPlaying())
        {
            ++count;
            MPMA::Sleep(50);
        }

        if (count<20) //stopped early?
        {
            std::cout<<"Sound did not play for long enough..."<<std::endl;
            return false;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioFlacSource)
#endif


#ifdef AUDIO_OGG_OPUS_ENABLED
#ifdef DECLARE_TESTS_CODE
class AudioOpusSave: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::WavFileSource> src=std::make_shared<AUDIO::WavFileSource>("data/testAudioStereo48000.wav");
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed for source."<<std::endl;
            return false;
        }

        MPMA::FileUtils::CreateDirectory("temp");

        std::list<std::pair<std::string, std::string>> fileComments;
        fileComments.push_back(std::pair<std::string, std::string>("Title", "Rar"));
        fileComments.push_back(std::pair<std::string, std::string>("Comment", "Rum"));

        if (!AUDIO::SaveSourceToOpusFileVBR("temp/testSaveOpus.opus", src, fileComments))
        {
            std::cout<<"SaveSourceToOpusFileVBR failed."<<std::endl;
            return false;
        }

        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioOpusSave)
#endif

#ifdef AUDIO_OGG_OPUS_ENABLED
#ifdef DECLARE_TESTS_CODE
class AudioOpusSource: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::OpusFileSource> src=std::make_shared<AUDIO::OpusFileSource>("data/testAudioOpus.opus");
        if (!src->IsLoaded())
        {
            std::cout<<"Load failed."<<std::endl;
            return false;
        }

        std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
        plr->PlayStreaming(src, AUDIO::FIXED);

        int count=0;
        while (plr->IsPlaying())
        {
            ++count;
            MPMA::Sleep(50);
        }

        if (count<20) //stopped early?
        {
            std::cout<<"Sound did not play for long enough..."<<std::endl;
            return false;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioOpusSource)
#endif

/*
#ifdef DECLARE_TESTS_CODE
class LoopingSound: public UnitTest
{
public:
    bool Run()
    {
        std::cout<<".wav source..."<<std::endl;
        {
            std::shared_ptr<AUDIO::WavFileSource> src=std::make_shared<AUDIO::WavFileSource>("data/testAudioLoopableTone.wav");
            std::shared_ptr<AUDIO::LoopSource> looper=std::make_shared<AUDIO::LoopSource>(src);

            std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
            plr->PlayStreaming(looper, AUDIO::FIXED);

            MPMA::Timer timer;
            while (timer.Step(false)<1.75f)
            {
                if (!plr->IsPlaying())
                {
                    std::cout<<"Sound is not playing but should still be looping.\n";
                    return false;
                }

                MPMA::Sleep(50);
            }
            plr->Stop();
        }
#ifdef AUDIO_OGG_VORBIS_ENABLED
        std::cout<<".ogg source..."<<std::endl;
        {
            std::shared_ptr<AUDIO::VorbisFileSource> src=std::make_shared<AUDIO::VorbisFileSource>("data/testAudioLoopableTone.ogg");
            std::shared_ptr<AUDIO::LoopSource> looper=std::make_shared<AUDIO::LoopSource>(src);

            std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
            plr->PlayStreaming(looper, AUDIO::FIXED);

            MPMA::Timer timer;
            while (timer.Step(false)<1.75f)
            {
                if (!plr->IsPlaying())
                {
                    std::cout<<"Sound is not playing but should still be looping.\n";
                    return false;
                }

                MPMA::Sleep(50);
            }
            plr->Stop();
        }
#endif
#ifdef AUDIO_OGG_FLAC_ENABLED
        std::cout<<".flac source..."<<std::endl;
        {
            std::shared_ptr<AUDIO::FlacFileSource> src=std::make_shared<AUDIO::FlacFileSource>("data/testAudioLoopableTone.flac");
            std::shared_ptr<AUDIO::LoopSource> looper=std::make_shared<AUDIO::LoopSource>(src);

            std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();
            plr->PlayStreaming(looper, AUDIO::FIXED);

            MPMA::Timer timer;
            while (timer.Step(false)<1.75f)
            {
                if (!plr->IsPlaying())
                {
                    std::cout<<"Sound is not playing but should still be looping.\n";
                    return false;
                }

                MPMA::Sleep(50);
            }
            plr->Stop();
        }
#endif
        return true;
    }
};
#endif
DECLARE_TEST_CASE(LoopingSound)

#ifdef DECLARE_TESTS_CODE
class AudioMultiplePlays: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;

        std::shared_ptr<AUDIO::WavFileSource> src0=std::make_shared<AUDIO::WavFileSource>("data/testAudioStereo16.wav");
        std::shared_ptr<AUDIO::WavFileSource> src1=std::make_shared<AUDIO::WavFileSource>("data/testAudioMono16.wav");
        std::shared_ptr<AUDIO::WavFileSource> src2=std::make_shared<AUDIO::WavFileSource>("data/testAudioMono8.wav");
        std::shared_ptr<AUDIO::WavFileSource> src3=std::make_shared<AUDIO::WavFileSource>("data/testAudioMono8_8000.wav");

        std::vector<std::shared_ptr<AUDIO::StaticBuffer>> staticBufs;
        staticBufs.push_back(std::make_shared<AUDIO::StaticBuffer>(src1));
        staticBufs.push_back(std::make_shared<AUDIO::StaticBuffer>(src3));

        players.push_back(AUDIO::Player::Create());
        players.push_back(AUDIO::Player::Create());
        players.push_back(AUDIO::Player::Create());
        players.push_back(AUDIO::Player::Create());
        players.push_back(AUDIO::Player::Create()); //repeat of a static
        players.push_back(AUDIO::Player::Create()); //repeat of a streaming (will fail)

        players[0]->PlayStatic(staticBufs[0], AUDIO::FIXED);
        MPMA::Sleep(250);
        players[1]->PlayStatic(staticBufs[1], AUDIO::FIXED);
        MPMA::Sleep(250);
        players[2]->PlayStreaming(src0, AUDIO::FIXED);
        MPMA::Sleep(250);
        players[3]->PlayStreaming(src2, AUDIO::FIXED);
        MPMA::Sleep(250);
        players[4]->PlayStatic(staticBufs[0], AUDIO::FIXED);
        MPMA::Sleep(250);
        players[5]->PlayStreaming(src0, AUDIO::FIXED);

        while (IsStillPlaying())
            MPMA::Sleep(50);

        MPMA::Sleep(250);
        return good;
    }

private:
    bool IsStillPlaying()
    {
        for (int i=0; i<(int)players.size(); ++i)
        {
            if (players[i]->IsPlaying())
                return true;
        }
        return false;
    }

    std::vector<std::shared_ptr<AUDIO::Player>> players;
};
#endif
DECLARE_TEST_CASE(AudioMultiplePlays)

#ifdef DECLARE_TESTS_CODE
class AudioRepeatedPlayStop: public UnitTest
{
public:
    bool Run()
    {
        std::shared_ptr<AUDIO::WavFileSource> src=std::make_shared<AUDIO::WavFileSource>("data/testAudioStereo16.wav");
        MPMA::Timer time;

        std::cout<<"Static..."<<std::endl;
        float totalPlayTime=0;
        float totalStopTime=0;
        {
            std::shared_ptr<AUDIO::StaticBuffer> buf=std::make_shared<AUDIO::StaticBuffer>(src);
            std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();

            for (int i=0; i<500; ++i)
            {
                time.Step(true);
                if (!plr->PlayStatic(buf, AUDIO::FIXED))
                {
                    std::cout<<"Play() failed."<<std::endl;
                    return false;
                }
                totalPlayTime+=(float)time.Step(true);

                while (time.Step(false)<i/10.0f/1000.0f)
                    MPMA::Sleep(0);

                time.Step(true);
                plr->Stop();
                totalStopTime+=(float)time.Step(true);
            }
            std::cout<<"Average Play() time="<<(totalPlayTime/500*1000)<<"ms"<<std::endl;
            std::cout<<"Average Stop() time="<<(totalStopTime/500*1000)<<"ms"<<std::endl;
        }

        std::cout<<"Streaming..."<<std::endl;
        totalPlayTime=0;
        totalStopTime=0;
        {
            std::shared_ptr<AUDIO::Player> plr=AUDIO::Player::Create();

            for (int i=0; i<500; ++i)
            {
                src->Seek(0);

                time.Step(true);
                if (!plr->PlayStreaming(src, AUDIO::FIXED))
                {
                    std::cout<<"Play() failed."<<std::endl;
                    return false;
                }
                totalPlayTime+=(float)time.Step(true);

                time.Step(true);
                while (time.Step(false)<i/10.0f/1000.0f)
                    MPMA::Sleep(0);

                time.Step(true);
                plr->Stop();
                totalStopTime+=(float)time.Step(true);
            }
            std::cout<<"Average Play() time="<<(totalPlayTime/500*1000)<<"ms"<<std::endl;
            std::cout<<"Average Stop() time="<<(totalStopTime/500*1000)<<"ms"<<std::endl;
        }

        MPMA::Sleep(250);
        return true;
    }
};
#endif
DECLARE_TEST_CASE(AudioRepeatedPlayStop)
*/