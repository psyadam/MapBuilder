//!\file Vertex.h OpenGL Vertex buffer and Index buffer wrappers.
//Luke Lenhart, 2010
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/Types.h"
#include "../base/ReferenceCount.h"
#include "../gfxsetup/GL.h"
#include "../gfxsetup/Extensions.h"

#include <vector>

namespace GFX
{
    //!Returns the numbere of bytes needed to store a specific opengl data type.
    inline GLsizei GetGLTypeSize(GLenum type)
    {
        GLsizei componentBytes=4;
        if (type==GL_UNSIGNED_BYTE || type==GL_BYTE) componentBytes=1;
        else if (type==GL_UNSIGNED_SHORT || type==GL_SHORT) componentBytes=2;
        else if (type==GL_DOUBLE) componentBytes=8;
        return componentBytes;
    }

    //!What a component of a stream represents.
    enum VertexComponentMeaning
    {
        VertexPosition,            //!<Count must be 2, 3, or 4
        VertexNormal,              //!<Count must be 3
        VertexColor,               //!<Count must be 3, or 4
        VertexTextureCoordinate0,  //!<
        VertexTextureCoordinate1,  //!<
        VertexTextureCoordinate2,  //!<
        VertexTextureCoordinate3,  //!<
        VertexTextureCoordinate4,  //!<
        VertexTextureCoordinate5,  //!<
        VertexTextureCoordinate6,  //!<
        VertexTextureCoordinate7,  //!<
        VertexTextureCoordinate8,  //!<
        VertexTextureCoordinate9,  //!<
        VertexTextureCoordinate10, //!<
        VertexTextureCoordinate11, //!<
        VertexTextureCoordinate12, //!<
        VertexTextureCoordinate13, //!<
        VertexTextureCoordinate14, //!<
        VertexTextureCoordinate15, //!<
        VertexPadding              //!<
    };

    //!An individual vertex component's use, type, and number of elements.  One or more of these are combined to make a InterleavedVertexFormat.
    struct VertexComponentFormat
    {
        //!What this component of a stream represents.
        VertexComponentMeaning Meaning;
        //!Data type of the component (GL_FLOAT, etc)
        GLenum Type;
        //!Number of elements in this component.
        GLuint Count;

        nuint AutoOffset; //computed automatically when added to a InterleavedVertexFormat

        //!
        inline VertexComponentFormat(): Meaning(VertexPadding), Type(GL_FLOAT), Count(0) {}
        //!
        inline VertexComponentFormat(VertexComponentMeaning meaning, GLenum type, GLuint count): Meaning(meaning), Type(type), Count(count) {}
    };

    //!Represents the combination of vertex components.
    struct InterleavedVertexFormat
    {
        //!In-order list of elements that make up the vertex format.
        std::vector<VertexComponentFormat> Components;
        //!Usage parameter to glBufferData (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, etc)
        GLenum Usage;

        GLsizei CurrentVertexSize; //updated automatically as components are added with the << operator

        //!
        inline InterleavedVertexFormat(): Usage(GL_STATIC_DRAW), CurrentVertexSize(0) {}

        //!Adds another component to the element list.
        inline InterleavedVertexFormat& operator<<(const VertexComponentFormat &newEntry)
        {
            Components.emplace_back(newEntry);
            Components.back().AutoOffset=CurrentVertexSize;
            CurrentVertexSize+=GetGLTypeSize(newEntry.Type)*newEntry.Count;
            return *this;
        }

        //!Returns the number of bytes needed to store a single vertex.
        inline GLsizei GetVertexSize() const
        {
            return CurrentVertexSize;
        }
    };

    struct VertexBufferData
    {
        GLuint object;

        InterleavedVertexFormat format;

        inline VertexBufferData(): object(0)
        {}
    };

    //!Represents a vertex buffer object.
    class VertexBuffer: public MPMA::ReferenceCountedData<VertexBufferData>
    {
    public:
        virtual ~VertexBuffer();

        //!Creates an empty vertex buffer.
        bool Create();
        //!Frees the current vertex buffer object.
        void Free();

        //!Retrieves the current vertex buffer object that can be passed to OpenGL calls.
        inline operator GLuint() const { return Data().object; }

        //!Creates a new empty vertex buffer and loads it with data vertex data.
        bool LoadInterleaved(const InterleavedVertexFormat &format, const void *bytes, nuint vertexCount);

        //!Binds the vertex buffer and sets up the GL vertex component pointers for using it.
        void BindAndSetState();
        //!Unbinds the vertex buffer and unsets the GL vertex component pointers.
        void Unbind();
    };

    //!Helper to automatically bind a vertex buffer then unbind it when the scope ends.
    class AutoBindVertexBuffer
    {
    public:
        //!
        inline AutoBindVertexBuffer(VertexBuffer &vbToBind): vb(vbToBind) { vb.BindAndSetState(); }
        //!
        inline ~AutoBindVertexBuffer() { vb.Unbind(); }

    private:
        VertexBuffer &vb;

        //prevent copying
        AutoBindVertexBuffer(const AutoBindVertexBuffer &o);
        bool operator=(const AutoBindVertexBuffer &o);
    };

    struct IndexBufferData
    {
        GLuint object;

        inline IndexBufferData(): object(0)
        {}
    };

    //!Represents an index buffer object.
    class IndexBuffer: public MPMA::ReferenceCountedData<IndexBufferData>
    {
    public:
        virtual ~IndexBuffer();

        //!Creates an empty index buffer.
        bool Create();
        //!Frees the current index buffer object.
        void Free();

        //!Retrieves the current index buffer object that can be passed to OpenGL calls.
        inline operator GLuint() const { return Data().object; }

        //!Creates a new empty index buffer and loads it with 16-bit data index data.
        bool Load(uint16 *bytes, nuint indexCount, GLenum usage=GL_STATIC_DRAW);

        //!Binds the index buffer.
        void BindAndSetState();
        //!Unbinds the index buffer.
        void Unbind();
    };

    //!Helper to automatically bind an index buffer then unbind it when the scope ends.
    class AutoBindIndexBuffer
    {
    public:
        //!
        inline AutoBindIndexBuffer(IndexBuffer &ibToBind): ib(ibToBind) { ib.BindAndSetState(); }
        //!
        inline ~AutoBindIndexBuffer() { ib.Unbind(); }

    private:
        IndexBuffer &ib;

        //prevent copying
        AutoBindIndexBuffer(const AutoBindIndexBuffer &o);
        bool operator=(const AutoBindIndexBuffer &o);
    };

    class BufferMapper
    {
        //TODO: Memory mappying of buffers
    };
}

#endif //#ifdef MPMA_COMPILE_GFX
