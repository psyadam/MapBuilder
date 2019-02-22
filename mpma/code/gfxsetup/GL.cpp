//!\file GL.cpp This contains a couple utility functions common to different parts of OpenGL.
//Luke Lenhart, 2009
//See /docs/License.txt for details on how this code may be used.

#include "GL.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include "../base/Debug.h"
#include "../base/DebugRouter.h"
#include <GL/glu.h>

namespace GFX
{
#ifdef GFX_CHECK_FOR_ERRORS
    //Returns and dequeues an entry from the OpenGL error queue.  Returns GL_NO_ERROR if no errors occured.  If there were errors they will be sent to MPMA's ErrorReport object.
    GLenum GetGLError()
    {
        GLenum err=glGetError();
        if (err==GL_NO_ERROR)
            return GL_NO_ERROR;

        if (VerboseGLErrorsOnGet)
        {
            const char *errorAsString=(const char *)gluErrorString(err);
            MPMA::Vary errorMessage="OpenGL error detected: ";
            errorMessage+=(int)err;
            errorMessage+=" ()";
            if (errorAsString)
                errorMessage+=errorAsString;
            MPMA::ErrorReport()<<errorMessage;
            MPMA::ErrorReport()<<"\n Backtrace:\n"<<MPMA::GetCallStack()<<"\n";
        }

        return err;
    }
#endif

    bool VerboseGLErrorsOnGet=true;
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
