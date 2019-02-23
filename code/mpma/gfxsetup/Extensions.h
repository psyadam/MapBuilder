//!\file Extensions.h OpenGL Extension setup
//Luke Lenhart, 2009
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include "GL.h"
//#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h> //Some platforms may need to obtain this from http://www.opengl.org/registry/
#include <string>

namespace GFX
{
    typedef void (*VoidFuncPtrType)();

    //!Returns whether a specifically named OpenGL extension is present on the current system.
    bool IsExtensionAvailable(const std::string &name);

    //!Returns a pointer to a OpenGL extension function with a specific name, or 0 if not found.
    VoidFuncPtrType GetExtensionFunction(const std::string &name);
}

#ifdef OPENGL_LOAD_COMMON_EXTENSIONS

namespace GFX_OPENGL_EXTENSIONS
{
    extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
    extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
    extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
    extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
#if !defined(linux) //curiously, these seem to already be defined in the base gl on linux even though they're extensions... which causes a name conflict with our declaration
    extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
    extern PFNGLACTIVETEXTUREPROC glActiveTextureARB;
#endif

    extern PFNGLCREATESHADERPROC glCreateShader;
    extern PFNGLDELETESHADERPROC glDeleteShader;
    extern PFNGLSHADERSOURCEPROC glShaderSourceARB;
    extern PFNGLCOMPILESHADERPROC glCompileShaderARB;
    extern PFNGLGETSHADERIVPROC glGetShaderiv;
    extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    extern PFNGLCREATEPROGRAMPROC glCreateProgram;
    extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
    extern PFNGLATTACHSHADERPROC glAttachShader;
    extern PFNGLDETACHSHADERPROC glDetachShader;
    extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
    extern PFNGLGETPROGRAMIVNVPROC glGetProgramiv;
    extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    extern PFNGLUSEPROGRAMPROC glUseProgram;
    extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
    extern PFNGLUNIFORM1FPROC glUniform1fARB;
    extern PFNGLUNIFORM2FPROC glUniform2fARB;
    extern PFNGLUNIFORM3FPROC glUniform3fARB;
    extern PFNGLUNIFORM4FPROC glUniform4fARB;
    extern PFNGLUNIFORM1IPROC glUniform1iARB;
    extern PFNGLUNIFORM2IPROC glUniform2iARB;
    extern PFNGLUNIFORM3IPROC glUniform3iARB;
    extern PFNGLUNIFORM4IPROC glUniform4iARB;
    extern PFNGLUNIFORM1FVPROC glUniform1fvARB;
    extern PFNGLUNIFORM2FVPROC glUniform2fvARB;
    extern PFNGLUNIFORM3FVPROC glUniform3fvARB;
    extern PFNGLUNIFORM4FVPROC glUniform4fvARB;
    extern PFNGLUNIFORM1IVPROC glUniform1ivARB;
    extern PFNGLUNIFORM2IVPROC glUniform2ivARB;
    extern PFNGLUNIFORM3IVPROC glUniform3ivARB;
    extern PFNGLUNIFORM4IVPROC glUniform4ivARB;
    extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fvARB;
    extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fvARB;
    extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fvARB;
    extern PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;

    extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
    extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
    extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
    extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
    extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
    extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
}
using namespace GFX_OPENGL_EXTENSIONS;

#endif //GFX_OPENGL_EXTENSIONS

#endif //#ifdef MPMA_COMPILE_GFXSETUP
