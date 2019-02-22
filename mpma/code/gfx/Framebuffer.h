//!\file Framebuffer.h OpenGL Framebuffer Object wrapper.
//Luke Lenhart, 2013
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/Types.h"
#include "../base/ReferenceCount.h"
#include "../gfxsetup/GL.h"
#include "../gfxsetup/Extensions.h"
#include "Texture.h"

#include <string>

namespace GFX
{
    struct RenderbufferData
    {
        GLuint object;

        inline RenderbufferData(): object(0)
        {}
    };

    //!Represents a renderbuffer, which can be attached to a framebuffer
    class Renderbuffer: public MPMA::ReferenceCountedData<RenderbufferData>
    {
    public:
         virtual ~Renderbuffer();

        //!Creates a new renderbuffer
        bool Create();
        //!Frees the renderbuffer object.
        void Free();

        //!Retrieves the OpenGL renderbuffer object.
        inline operator GLuint() const { return Data().object; }

        //!Binds the renderbuffer.
        inline void Bind()
        {
            if (glBindRenderbuffer)
                glBindRenderbuffer(GL_RENDERBUFFER, *this);
        }

        //!Unbinds the renderbuffer.
        inline void Unbind()
        {
            if (glBindRenderbuffer)
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    };

    struct FramebufferData
    {
        GLuint object;
        Renderbuffer rbDepth;
        Renderbuffer rbStencil;
        Texture2D texTarget;

        inline FramebufferData(): object(0)
        {}
    };

    //!Represents a framebuffer
    class Framebuffer: public MPMA::ReferenceCountedData<FramebufferData>
    {
    public:
        virtual ~Framebuffer();

        //!Creates a new framebuffer with nothing attached.
        bool Create();
        //!Creates a new framebuffer to render to a texture.  The texture must have its format and size set prior to this call.  Optionally attaches a matching depth/stencil render buffer.
        bool Create(Texture2D &color, bool depth=true, bool stencil=false);
        //!Frees the framebuffer object.
        void Free();

        //!Retrieves the OpenGL framebuffer object.
        inline operator GLuint() const { return Data().object; }
        //!Retrieves the depth render buffer, if one was automatically created from the Create function.
        inline Renderbuffer& GetCreatedDepthRenderbuffer() { return Data().rbDepth; }
        //!Retrieves the stencil render buffer, if one was automatically created from the Create function.
        inline Renderbuffer& GetCreatedStencilRenderbuffer() { return Data().rbStencil; }
        inline Texture2D& GetCreatedTextureTarget() { return Data().texTarget; }

        //!Binds the framebuffer.
        inline void Bind()
        {
            if (glBindFramebuffer)
                glBindFramebuffer(GL_FRAMEBUFFER, *this);
        }

        //!Unbinds the framebuffer.
        inline void Unbind()
        {
            if (glBindFramebuffer)
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };


    //!Helper to automatically bind a framebuffer then unbind it when the scope ends.
    class AutoBindFramebuffer
    {
    public:
        inline AutoBindFramebuffer(Framebuffer &fbToBind): fb(fbToBind) { fb.Bind(); }
        inline ~AutoBindFramebuffer() { fb.Unbind(); }

    private:
        Framebuffer &fb;

        //prevent copying
        AutoBindFramebuffer(const AutoBindFramebuffer &o);
        bool operator=(const AutoBindFramebuffer &o);
    };
}

#endif //#ifdef MPMA_COMPILE_GFX
