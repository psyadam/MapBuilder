//todo.. header..

#include "Vertex.h"

#ifdef MPMA_COMPILE_GFX

#ifndef OPENGL_LOAD_COMMON_EXTENSIONS
    #define OPENGL_LOAD_COMMON_EXTENSIONS
#endif
#include "../gfxsetup/Extensions.h"

namespace GFX
{
    // -- VertexBuffer

    VertexBuffer::~VertexBuffer()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool VertexBuffer::Create()
    {
        Free();
        glGenBuffersARB(1, &Data().object);

        return Data().object!=0;
    }

    void VertexBuffer::Free()
    {
        if (Data().object!=0)
        {
            glDeleteBuffersARB(1, &Data().object);
            Data().object=0;
        }

        Data().format.CurrentVertexSize=0;
        Data().format.Components.clear();
    }

    bool VertexBuffer::LoadInterleaved(const InterleavedVertexFormat &format, const void *bytes, nuint vertexCount)
    {
        if (!Create())
            return false;

        //store
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, Data().object);

        GFX::ClearGLErrors();
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, format.GetVertexSize()*vertexCount, bytes, format.Usage);
        if (GFX::GetGLError())
            return false;

        Data().format=format;

        return true;
    }

    void VertexBuffer::BindAndSetState()
    {
        if (Data().object==0)
            return;

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, Data().object);

        for (std::vector<VertexComponentFormat>::const_iterator i=Data().format.Components.begin(); i!=Data().format.Components.end(); ++i)
        {
            if (i->Meaning==VertexPosition)
            {
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(i->Count, i->Type, Data().format.GetVertexSize(), (GLvoid*)i->AutoOffset);
            }
            else if (i->Meaning>=VertexTextureCoordinate0 && i->Meaning<=VertexTextureCoordinate15)
            {
                nuint texStage=i->Meaning-VertexTextureCoordinate0;
                glClientActiveTextureARB((GLenum)(GL_TEXTURE0_ARB+texStage));
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(i->Count, i->Type, Data().format.GetVertexSize(), (GLvoid*)i->AutoOffset);
            }
            else if (i->Meaning==VertexNormal)
            {
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(i->Type, Data().format.GetVertexSize(), (GLvoid*)i->AutoOffset);
            }
            else if (i->Meaning==VertexColor)
            {
                glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(i->Count, i->Type, Data().format.GetVertexSize(), (GLvoid*)i->AutoOffset);
            }
        }
    }

    void VertexBuffer::Unbind()
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        for (std::vector<VertexComponentFormat>::const_iterator i=Data().format.Components.begin(); i!=Data().format.Components.end(); ++i)
        {
            if (i->Meaning==VertexPosition)
            {
                glDisableClientState(GL_VERTEX_ARRAY);
            }
            else if (i->Meaning>=VertexTextureCoordinate0 && i->Meaning<=VertexTextureCoordinate15)
            {
                nuint texStage=i->Meaning-VertexTextureCoordinate0;
                glClientActiveTextureARB((GLenum)(GL_TEXTURE0_ARB+texStage));
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
            else if (i->Meaning==VertexNormal)
            {
                glDisableClientState(GL_NORMAL_ARRAY);
            }
            else if (i->Meaning==VertexColor)
            {
                glDisableClientState(GL_COLOR_ARRAY);
            }
        }

        glClientActiveTextureARB(GL_TEXTURE0_ARB);
    }

    // -- Index Buffer

    IndexBuffer::~IndexBuffer()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool IndexBuffer::Create()
    {
        Free();
        glGenBuffersARB(1, &Data().object);

        return Data().object!=0;
    }

    void IndexBuffer::Free()
    {
        if (Data().object!=0)
        {
            glDeleteBuffersARB(1, &Data().object);
            Data().object=0;
        }
    }

    bool IndexBuffer::Load(uint16 *bytes, nuint indexCount, GLenum usage)
    {
        if (!Create())
            return false;

        //store
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, Data().object);

        GFX::ClearGLErrors();
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, 2*indexCount, bytes, usage);
        if (GFX::GetGLError())
            return false;

        return true;
    }

    void IndexBuffer::BindAndSetState()
    {
        if (Data().object==0)
            return;

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, Data().object);
    }

    void IndexBuffer::Unbind()
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

#endif //#ifdef MPMA_COMPILE_GFX
