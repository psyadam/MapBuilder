//internal use: includes the OpenAL headers for the current platform
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"

#ifdef MPMA_COMPILE_AUDIO

#if defined(_WIN32) || defined(_WIN64) //windows - msvc

    //For whatever reason, OpenAL sdk on windows by default does NOT have the headers under the AL/ directory.
    #include <al.h>
    #include <alc.h>

#else //linux

    #include <AL/al.h>
    #include <AL/alc.h>

#endif

#endif