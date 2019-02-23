//!\file Extensions.cpp OpenGL Extension setup
//Luke Lenhart, 2009
//See /docs/License.txt for details on how this code may be used.

#include "Extensions.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include <set>
#include "../base/MiscStuff.h"
#include "../base/DebugRouter.h"

namespace GFX_INTERNAL
{
    std::set<std::string> extensionList;

    //Gets an extension function and writes error output if that function can't be retrieved
    template <typename T>
    T GetExtensionFunctionVerbose(const std::string &name)
    {
        T func=(T)GFX::GetExtensionFunction(name);
        if (!func)
            MPMA::ErrorReport()<<"GL GetExtensionFunction FAILED for function: "<<name<<"\n";

        return func;
    }

    //these are called by GFXSetup whenever the window is setup or destroyed
    void ExtensionInit()
    {
        //try to load all extensions
        glGenBuffersARB=GetExtensionFunctionVerbose<PFNGLGENBUFFERSARBPROC>("glGenBuffersARB");
        glDeleteBuffersARB=GetExtensionFunctionVerbose<PFNGLDELETEBUFFERSARBPROC>("glDeleteBuffersARB");
        glBindBufferARB=GetExtensionFunctionVerbose<PFNGLBINDBUFFERARBPROC>("glBindBufferARB");
        glBufferDataARB=GetExtensionFunctionVerbose<PFNGLBUFFERDATAARBPROC>("glBufferDataARB");
#if !defined(linux) //curiously, these seem to already be defined in the base gl on linux even though they're extensions... which causes a name conflict with our declaration
        glClientActiveTextureARB=GetExtensionFunctionVerbose<PFNGLCLIENTACTIVETEXTUREARBPROC>("glClientActiveTextureARB");
        glActiveTextureARB=GetExtensionFunctionVerbose<PFNGLACTIVETEXTUREPROC>("glActiveTextureARB");
#endif

        glCreateShader=GetExtensionFunctionVerbose<PFNGLCREATESHADERPROC>("glCreateShader");
        glDeleteShader=GetExtensionFunctionVerbose<PFNGLDELETESHADERPROC>("glDeleteShader");
        glShaderSourceARB=GetExtensionFunctionVerbose<PFNGLSHADERSOURCEPROC>("glShaderSourceARB");
        glCompileShaderARB=GetExtensionFunctionVerbose<PFNGLCOMPILESHADERPROC>("glCompileShaderARB");
        glGetShaderiv=GetExtensionFunctionVerbose<PFNGLGETSHADERIVPROC>("glGetShaderiv");
        glGetShaderInfoLog=GetExtensionFunctionVerbose<PFNGLGETSHADERINFOLOGPROC>("glGetShaderInfoLog");
        glLinkProgramARB=GetExtensionFunctionVerbose<PFNGLLINKPROGRAMARBPROC>("glLinkProgramARB");
        glGetProgramiv=GetExtensionFunctionVerbose<PFNGLGETPROGRAMIVNVPROC>("glGetProgramiv");
        glGetProgramInfoLog=GetExtensionFunctionVerbose<PFNGLGETPROGRAMINFOLOGPROC>("glGetProgramInfoLog");
        glUseProgram=GetExtensionFunctionVerbose<PFNGLUSEPROGRAMPROC>("glUseProgram");
        glCreateProgram=GetExtensionFunctionVerbose<PFNGLCREATEPROGRAMPROC>("glCreateProgram");
        glDeleteProgram=GetExtensionFunctionVerbose<PFNGLDELETEPROGRAMPROC>("glDeleteProgram");
        glAttachShader=GetExtensionFunctionVerbose<PFNGLATTACHSHADERPROC>("glAttachShader");
        glDetachShader=GetExtensionFunctionVerbose<PFNGLDETACHSHADERPROC>("glDetachShader");
        glGetUniformLocationARB=GetExtensionFunctionVerbose<PFNGLGETUNIFORMLOCATIONARBPROC>("glGetUniformLocationARB");
        glUniform1fARB=GetExtensionFunctionVerbose<PFNGLUNIFORM1FPROC>("glUniform1fARB");
        glUniform2fARB=GetExtensionFunctionVerbose<PFNGLUNIFORM2FPROC>("glUniform2fARB");
        glUniform3fARB=GetExtensionFunctionVerbose<PFNGLUNIFORM3FPROC>("glUniform3fARB");
        glUniform4fARB=GetExtensionFunctionVerbose<PFNGLUNIFORM4FPROC>("glUniform4fARB");
        glUniform1iARB=GetExtensionFunctionVerbose<PFNGLUNIFORM1IPROC>("glUniform1iARB");
        glUniform2iARB=GetExtensionFunctionVerbose<PFNGLUNIFORM2IPROC>("glUniform2iARB");
        glUniform3iARB=GetExtensionFunctionVerbose<PFNGLUNIFORM3IPROC>("glUniform3iARB");
        glUniform4iARB=GetExtensionFunctionVerbose<PFNGLUNIFORM4IPROC>("glUniform4iARB");
        glUniform1fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORM1FVPROC>("glUniform1fvARB");
        glUniform2fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORM2FVPROC>("glUniform2fvARB");
        glUniform3fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORM3FVPROC>("glUniform3fvARB");
        glUniform4fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORM4FVPROC>("glUniform4fvARB");
        glUniform1ivARB=GetExtensionFunctionVerbose<PFNGLUNIFORM1IVPROC>("glUniform1ivARB");
        glUniform2ivARB=GetExtensionFunctionVerbose<PFNGLUNIFORM2IVPROC>("glUniform2ivARB");
        glUniform3ivARB=GetExtensionFunctionVerbose<PFNGLUNIFORM3IVPROC>("glUniform3ivARB");
        glUniform4ivARB=GetExtensionFunctionVerbose<PFNGLUNIFORM4IVPROC>("glUniform4ivARB");
        glUniformMatrix2fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORMMATRIX2FVPROC>("glUniformMatrix2fvARB");
        glUniformMatrix3fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORMMATRIX3FVPROC>("glUniformMatrix3fvARB");
        glUniformMatrix4fvARB=GetExtensionFunctionVerbose<PFNGLUNIFORMMATRIX4FVPROC>("glUniformMatrix4fvARB");
        glBindFragDataLocation=GetExtensionFunctionVerbose<PFNGLBINDFRAGDATALOCATIONPROC>("glBindFragDataLocation");

        glGenFramebuffers=GetExtensionFunctionVerbose<PFNGLGENFRAMEBUFFERSPROC>("glGenFramebuffers");
        glDeleteFramebuffers=GetExtensionFunctionVerbose<PFNGLDELETEFRAMEBUFFERSPROC>("glDeleteFramebuffers");
        glBindFramebuffer=GetExtensionFunctionVerbose<PFNGLBINDFRAMEBUFFERPROC>("glBindFramebuffer");
        glFramebufferTexture2D=GetExtensionFunctionVerbose<PFNGLFRAMEBUFFERTEXTURE2DPROC>("glFramebufferTexture2D");
        glFramebufferRenderbuffer=GetExtensionFunctionVerbose<PFNGLFRAMEBUFFERRENDERBUFFERPROC>("glFramebufferRenderbuffer");
        glCheckFramebufferStatus=GetExtensionFunctionVerbose<PFNGLCHECKFRAMEBUFFERSTATUSPROC>("glCheckFramebufferStatus");
        glGenRenderbuffers=GetExtensionFunctionVerbose<PFNGLGENRENDERBUFFERSPROC>("glGenRenderbuffers");
        glDeleteRenderbuffers=GetExtensionFunctionVerbose<PFNGLDELETERENDERBUFFERSPROC>("glDeleteRenderbuffers");
        glBindRenderbuffer=GetExtensionFunctionVerbose<PFNGLBINDRENDERBUFFERPROC>("glBindRenderbuffer");
        glRenderbufferStorage=GetExtensionFunctionVerbose<PFNGLRENDERBUFFERSTORAGEPROC>("glRenderbufferStorage");
    }

    void ExtensionShutdown()
    {
        //clear out all extensions as they might change later
        extensionList.clear();

        glGenBuffersARB=0;
        glDeleteBuffersARB=0;
        glBindBufferARB=0;
        glBufferDataARB=0;
#if !defined(linux) //curiously, these seem to already be defined in the base gl on linux even though they're extensions... which causes a name conflict with our declaration
        glClientActiveTextureARB=0;
        glActiveTextureARB=0;
#endif

        glCreateShader=0;
        glDeleteShader=0;
        glShaderSourceARB=0;
        glCompileShaderARB=0;
        glGetShaderiv=0;
        glGetShaderInfoLog=0;
        glCreateProgram=0;
        glDeleteProgram=0;
        glAttachShader=0;
        glDetachShader=0;
        glLinkProgramARB=0;
        glGetProgramiv=0;
        glGetProgramInfoLog=0;
        glUseProgram=0;
        glGetUniformLocationARB=0;
        glUniform1fARB=0;
        glUniform2fARB=0;
        glUniform3fARB=0;
        glUniform4fARB=0;
        glUniform1iARB=0;
        glUniform2iARB=0;
        glUniform3iARB=0;
        glUniform4iARB=0;
        glUniform1fvARB=0;
        glUniform2fvARB=0;
        glUniform3fvARB=0;
        glUniform4fvARB=0;
        glUniform1ivARB=0;
        glUniform2ivARB=0;
        glUniform3ivARB=0;
        glUniform4ivARB=0;
        glUniformMatrix2fvARB=0;
        glUniformMatrix3fvARB=0;
        glUniformMatrix4fvARB=0;
        glBindFragDataLocation=0;

        glGenFramebuffers=0;
        glDeleteFramebuffers=0;
        glBindFramebuffer=0;
        glFramebufferTexture2D=0;
        glFramebufferRenderbuffer=0;
        glCheckFramebufferStatus=0;
        glGenRenderbuffers=0;
        glDeleteRenderbuffers=0;
        glBindRenderbuffer=0;
        glRenderbufferStorage=0;
    }
}
using namespace GFX_INTERNAL;

namespace GFX
{
    //Returns whether a specifically named OpenGL extension is present on the current system.
    bool IsExtensionAvailable(const std::string &name)
    {
        //see if we need to populate the set
        if (extensionList.empty())
        {
            const char *rawExtensionStrings=(const char*)glGetString(GL_EXTENSIONS);
            if (rawExtensionStrings==0)
            {
                MPMA::ErrorReport()<<"glGetString(GL_EXTENSIONS) returned 0.  IsExtensionAvailable was probably called when OpenGL is not yet running.\n";
                return false;
            }

            std::string extensionsString=rawExtensionStrings;
            std::vector<std::string> strings;
            MISC::ExplodeString(extensionsString, strings);
            for (std::vector<std::string>::iterator i=strings.begin(); i!=strings.end(); ++i)
                extensionList.insert(*i);
        }

        //search the set for it
        return extensionList.find(name)!=extensionList.end();
    }
}

namespace GFX_OPENGL_EXTENSIONS
{
    PFNGLGENBUFFERSARBPROC glGenBuffersARB=0;
    PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB=0;
    PFNGLBINDBUFFERARBPROC glBindBufferARB=0;
    PFNGLBUFFERDATAARBPROC glBufferDataARB=0;
#if !defined(linux) //curiously, these seem to already be defined in the base gl on linux even though they're extensions... which causes a name conflict with our declaration
    PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB=0;
    PFNGLACTIVETEXTUREPROC glActiveTextureARB=0;
#endif

    PFNGLCREATESHADERPROC glCreateShader=0;
    PFNGLDELETESHADERPROC glDeleteShader=0;
    PFNGLSHADERSOURCEPROC glShaderSourceARB=0;
    PFNGLCOMPILESHADERPROC glCompileShaderARB=0;
    PFNGLGETSHADERIVPROC glGetShaderiv=0;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog=0;
    PFNGLCREATEPROGRAMPROC glCreateProgram=0;
    PFNGLDELETEPROGRAMPROC glDeleteProgram=0;
    PFNGLATTACHSHADERPROC glAttachShader=0;
    PFNGLDETACHSHADERPROC glDetachShader=0;
    PFNGLLINKPROGRAMARBPROC glLinkProgramARB=0;
    PFNGLGETPROGRAMIVNVPROC glGetProgramiv=0;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog=0;
    PFNGLUSEPROGRAMPROC glUseProgram=0;
    PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB=0;
    PFNGLUNIFORM1FPROC glUniform1fARB=0;
    PFNGLUNIFORM2FPROC glUniform2fARB=0;
    PFNGLUNIFORM3FPROC glUniform3fARB=0;
    PFNGLUNIFORM4FPROC glUniform4fARB=0;
    PFNGLUNIFORM1IPROC glUniform1iARB=0;
    PFNGLUNIFORM2IPROC glUniform2iARB=0;
    PFNGLUNIFORM3IPROC glUniform3iARB=0;
    PFNGLUNIFORM4IPROC glUniform4iARB=0;
    PFNGLUNIFORM1FVPROC glUniform1fvARB=0;
    PFNGLUNIFORM2FVPROC glUniform2fvARB=0;
    PFNGLUNIFORM3FVPROC glUniform3fvARB=0;
    PFNGLUNIFORM4FVPROC glUniform4fvARB=0;
    PFNGLUNIFORM1IVPROC glUniform1ivARB=0;
    PFNGLUNIFORM2IVPROC glUniform2ivARB=0;
    PFNGLUNIFORM3IVPROC glUniform3ivARB=0;
    PFNGLUNIFORM4IVPROC glUniform4ivARB=0;
    PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fvARB=0;
    PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fvARB=0;
    PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fvARB=0;
    PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation=0;

    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers=0;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers=0;
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer=0;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D=0;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer=0;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus=0;
    PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers=0;
    PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers=0;
    PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer=0;
    PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage=0;
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
