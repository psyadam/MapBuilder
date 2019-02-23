//!\file SaveToFile.h Save a sound source to a file.
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#include <string>
#include <memory>
#include <list>
#include <utility>
#include "Source.h"

namespace AUDIO
{
    //!Saves a finite audio source to a wave file.  If source is infinite or on failure, returns false.
    bool SaveSourceToWaveFile(const std::string &filename, std::shared_ptr<AUDIO::Source> source);

#ifdef AUDIO_OGG_VORBIS_ENABLED

    //!Saves a finite audio source to an ogg vorbis file using variable bit rate encoding.  If source is infinite or on failure, returns false.  vbrQuality may be from -0.1 to 1.0.  0.4 is roughly 128kbps.
    bool SaveSourceToVorbisFileVBR(const std::string &filename, std::shared_ptr<AUDIO::Source> source, float vbrQuality, const std::list<std::pair<std::string, std::string>> &fileComments);

#endif

#ifdef AUDIO_OGG_OPUS_ENABLED

    //!Saves a finite audio source to an ogg opus file using variable bit rate encoding.  If source is infinite or on failure, returns false.  Due to limitations in the opus encoder, only sources whose sample rate is 8000, 12000, 16000, 24000, or 48000 are allowed.
    bool SaveSourceToOpusFileVBR(const std::string &filename, std::shared_ptr<AUDIO::Source> source, const std::list<std::pair<std::string, std::string>> &fileComments);

#endif

}

#endif //#ifdef MPMA_COMPILE_AUDIO
