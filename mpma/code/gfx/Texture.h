//!\file Texture.h OpenGL Texture wrapper.
//Luke Lenhart, 2010
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/Types.h"
#include "../base/ReferenceCount.h"
#include "../base/File.h"
#include "../gfxsetup/GL.h"
#include "../gfxsetup/Extensions.h"

#include <string>

namespace GFX
{
    //!Represents the dimensions of a texture.
    struct TextureDimensions
    {
        nuint Width; //!<
        nuint Height; //!<
        //nuint Depth;

        //!
        inline TextureDimensions(nuint wid=0, nuint hei=1/*, nuint dep=1*/)
        { Width=wid; Height=hei; /*Depth=dep;*/ }
    };

    //!Parameters used in the creation of a texture.
    struct TextureCreateParameters
    {
        //!Preferred internal format of the texture (GL_RGBA8, etc)
        GLint Format;
        //!Whether to automatically generate mipmaps when the CreateFrom* functions are called
        bool GenerateMipMaps;
        //!The dimensions of the texture to create.  For CreateFromFile* calls, if this is set to 0 area then the size is taken from the source file, else the source data will be scaled to the specified size.
        TextureDimensions Dimensions; //Resize source image to this (if non-zero size)

        //bool Compress; //TODO

        //!Whether to free and recreate the texture object if it already exists.
        bool Recreate;

        inline TextureCreateParameters(): Format(GL_RGBA8), GenerateMipMaps(true), Recreate(false) //, Compress(false)
        {}
    };

    struct TextureData
    {
        GLuint object;
        TextureDimensions origDimensions;

        inline TextureData(): object(0)
        {}
    };

    //!Base class for the other texture classes.
    class TextureBase: public MPMA::ReferenceCountedData<TextureData>
    {
    public:
        virtual ~TextureBase();

        //!Creates an empty texture object.
        bool Create();
        //!Creates an empty texture with specific properties but no data.
        bool Create(const TextureCreateParameters &properties);
        //!Frees the texture object.
        void Free();

        //!Retrieves the OpenGL texture object.
        inline operator GLuint() const { return Data().object; }

        //!Retrieves the width of the original texture if loaded from a file.
        inline const nuint& OriginalWidth() const { return Data().origDimensions.Width; }
        //!Retrieves the height of the original texture if loaded from a file.
        inline const nuint& OriginalHeight() const { return Data().origDimensions.Height; }
        //inline const nuint& OriginalDepth() const { return Data().origDimensions.Depth; }

#ifdef GFX_USES_FREEIMAGE
        //!Creates a texture from a file.
        bool CreateFromFile(const MPMA::Filename &filename, const TextureCreateParameters *properties=0);
        //bool CreateFromFileInMemory(const unsigned char *memory, nuint byteCount, const TextureCreateParameters *properties=0); //TODO
#endif
        //!Creates a texture from data in memory.
        bool CreateFromMemory(void *sourceData, const TextureCreateParameters *properties, GLint pixelFormat=GL_RGBA, GLenum pixelSize=GL_UNSIGNED_BYTE, GLenum usage=GL_STATIC_DRAW);

        //!Binds the texture and sets it to a texture stage.
        inline void BindAndSetState(int stage)
        {
            glActiveTextureARB(GL_TEXTURE0_ARB+stage);
            glBindTexture(target, *this);
            glEnable(target);
        }

        //!Unbinds the texture and clears it from a texture stage.
        inline void Unbind(int stage)
        {
            glActiveTextureARB(GL_TEXTURE0_ARB+stage);
            glBindTexture(target, 0);
            glDisable(target);
        }

    protected:
        inline TextureBase(GLenum textureTarget): target(textureTarget)
        {}

        void SetDefaultProperties();

        GLenum target;
    };

    //!Represents a two dimensional texture.
    class Texture2D: public TextureBase
    {
    public:
        inline Texture2D(): TextureBase(GL_TEXTURE_2D)
        {}

        inline void GetDimensions(int &width, int &height)
        {
            width=height=0;
            BindAndSetState(0);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            Unbind(0);
        }
    };

    //!Helper to automatically bind a texture then unbind it when the scope ends.
    class AutoBindTexture
    {
    public:
        inline AutoBindTexture(TextureBase &texToBind, int stage=0): tex(texToBind), boundStage(stage) { tex.BindAndSetState(stage); }
        inline ~AutoBindTexture() { tex.Unbind(boundStage); }

    private:
        TextureBase &tex;
        int boundStage;

        //prevent copying
        AutoBindTexture(const AutoBindTexture &o);
        bool operator=(const AutoBindTexture &o);
    };
}

#endif //#ifdef MPMA_COMPILE_GFX
