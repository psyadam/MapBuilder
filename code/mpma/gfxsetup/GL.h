//!\file GL.h Includes OpenGL's gl.h.  This should be used instead of directly including gl.h, and before glu.h or others are included.  This also contains a couple utility functions common to different parts of OpenGL.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFXSETUP

#if defined(_WIN32) || defined(_WIN64) //windows - msvc

    //For whatever reason, msvc's gl.h tries to use a bunch of stuff it does not define.
    //Since we don't want to pull windows.h in to the whole world, we'll just define those things here.
    #include "../base/win32/alt_windows.h"

    #include <GL/gl.h>

#else //linux

    #include <GL/gl.h>

#endif

namespace GFX
{
    static_assert(GL_NO_ERROR==0, "GL_NO_ERROR should evaluate to 0");

#ifdef GFX_CHECK_FOR_ERRORS
    //!Clears any OpenGL errors from the error queue
    inline void ClearGLErrors() { glGetError(); }

	//!Returns and dequeues an entry from the OpenGL error queue.  Returns GL_NO_ERROR if no errors occured.
    GLenum GetGLError();
#else
    inline void ClearGLErrors() {}
    inline GLenum GetGLError() { return GL_NO_ERROR; }
#endif

    //!This controls whether GetGLError reports all errors to MPMA's error reporting object.  Default is true.  This is ignored if GFX_CHECK_FOR_ERRORS is not defined.
    extern bool VerboseGLErrorsOnGet;
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
