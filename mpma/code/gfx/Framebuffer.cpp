//OpenGL Framebuffer Object wrapper.
//Luke Lenhart, 2013
//See /docs/License.txt for details on how this code may be used.

#include "Framebuffer.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/DebugRouter.h"

namespace GFX
{
    // -- Renderbuffer

    Renderbuffer::~Renderbuffer()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool Renderbuffer::Create()
    {
        if (!glGenRenderbuffers)
        {
            MPMA::ErrorReport()<<"OpenGL Framebuffer objects not supported by the current hardware.\n";
            return false;
        }

        Free();
        glGenRenderbuffers(1, &Data().object);

        return Data().object!=0;
    }

    void Renderbuffer::Free()
    {
        if (Data().object!=0)
        {
            glDeleteRenderbuffers(1, &Data().object);
            Data().object=0;
        }
    }

    // -- Framebuffer

    Framebuffer::~Framebuffer()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool Framebuffer::Create()
    {
        if (!glGenFramebuffers)
        {
            MPMA::ErrorReport()<<"OpenGL Framebuffer objects not supported by the current hardware.\n";
            return false;
        }

        Free();
        glGenFramebuffers(1, &Data().object);

        return Data().object!=0;
    }

    bool Framebuffer::Create(Texture2D &color, bool depth, bool stencil)
    {
        if (!color || !Create())
            return false;

        int texWidth, texHeight;
        color.GetDimensions(texWidth, texHeight);

        if (texWidth<=0 || texHeight<=0)
            return false;

        Bind();

        Data().texTarget=color;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        GFX::ClearGLErrors();
        if (depth && Data().rbDepth.Create())
        {
            Data().rbDepth.Bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);
            Data().rbDepth.Unbind();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Data().rbDepth);
        }
        if (GFX::GetGLError())
        {
            MPMA::ErrorReport()<<"Failed to attach depth renderbuffer to framebuffer object.\n";
        }

        if (stencil && Data().rbStencil.Create())
        {
            Data().rbStencil.Bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, texWidth, texHeight);
            Data().rbStencil.Unbind();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Data().rbStencil);
        }

        GLenum fbStatus=glCheckFramebufferStatus(GL_FRAMEBUFFER);

        Unbind();

        return fbStatus==GL_FRAMEBUFFER_COMPLETE;
    }

    void Framebuffer::Free()
    {
        if (Data().object!=0)
        {
            Bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            if (Data().rbDepth)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            if (Data().rbStencil)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            Unbind();
        }

        Data().rbDepth.Free();
        Data().rbStencil.Free();
        Data().texTarget.Free();

        if (Data().object!=0)
        {
            glDeleteFramebuffers(1, &Data().object);
            Data().object=0;
        }
    }
}

#endif //#ifdef MPMA_COMPILE_GFX
