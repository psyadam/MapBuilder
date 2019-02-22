//OpenGL Texture wrapper.
//Luke Lenhart, 2010
//See /docs/License.txt for details on how this code may be used.

#include "Texture.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/DebugRouter.h"
#include "../base/Profiler.h"
#include "../base/MiscStuff.h"
#include <GL/glu.h>

#ifdef GFX_USES_FREEIMAGE
    #include "../Setup.h"
    #if defined(_WIN32) || defined(_WIN64) //oddly they rely on windows.h being included on windows, but not on other platforms.. hack around it with our special alternative..
        #define _WINDOWS_
        #include "../base/win32/alt_windows.h"
        typedef struct tagRGBQUAD {
          BYTE rgbBlue;
          BYTE rgbGreen;
          BYTE rgbRed;
          BYTE rgbReserved;
        } RGBQUAD;

        typedef struct tagBITMAPINFOHEADER{
          DWORD biSize;
          LONG  biWidth; 
          LONG  biHeight; 
          WORD  biPlanes; 
          WORD  biBitCount;
          DWORD biCompression; 
          DWORD biSizeImage; 
          LONG  biXPelsPerMeter; 
          LONG  biYPelsPerMeter; 
          DWORD biClrUsed; 
          DWORD biClrImportant;
        } BITMAPINFOHEADER, *PBITMAPINFOHEADER; 

        typedef struct tagBITMAPINFO { 
          BITMAPINFOHEADER bmiHeader; 
          RGBQUAD          bmiColors[1];
        } BITMAPINFO, *PBITMAPINFO;
    #endif
    #define FREEIMAGE_LIB
    #include <FreeImage.h>
#endif

namespace
{
#ifdef GFX_USES_FREEIMAGE
    class AutoInitFreeImage
    {
    private:
        static void FreeImageSetup()
        {
            FreeImage_Initialise();
        }

        static void FreeImageCleanup()
        {
            FreeImage_DeInitialise();
        }

    public:
        //hookup init callbacks
        AutoInitFreeImage()
        {
            MPMA::Internal_AddInitCallback(FreeImageSetup, 4000);
            MPMA::Internal_AddShutdownCallback(FreeImageCleanup, 4000);
        }
    } autoInitFreeImage;
#endif
}

namespace GFX
{
    TextureBase::~TextureBase()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool TextureBase::Create()
    {
        Free();
        glGenTextures(1, &Data().object);

        return Data().object!=0;
    }

    bool TextureBase::Create(const TextureCreateParameters &properties)
    {
        if (!Create())
            return false;

        glBindTexture(target, *this);
        glTexParameteri(target, GL_GENERATE_MIPMAP, properties.GenerateMipMaps?GL_TRUE:GL_FALSE);

        GFX::ClearGLErrors();
        glTexImage2D(target, 0, properties.Format, (GLsizei)properties.Dimensions.Width, (GLsizei)properties.Dimensions.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(target, 0);
        if (GFX::GetGLError())
            return false;
        return true;
    }

    void TextureBase::Free()
    {
        if (Data().object!=0)
        {
            glDeleteTextures(1, &Data().object);
            Data().object=0;
        }
    }

#ifdef GFX_USES_FREEIMAGE
    bool TextureBase::CreateFromFile(const MPMA::Filename &filename, const TextureCreateParameters *properties)
    {
        MPMAProfileScope("GFX::TextureBase::CreateFromFile");

        FREE_IMAGE_FORMAT fif=FreeImage_GetFileType(filename.c_str(), 0);
        if (fif==FIF_UNKNOWN)
            fif=FreeImage_GetFIFFromFilename(filename.c_str());
        if (fif==FIF_UNKNOWN)
            return false;

        FIBITMAP *fibOrig=FreeImage_Load(fif, filename.c_str(), 0);
        if (!fibOrig)
            return false;

        FIBITMAP *fibConverted=FreeImage_ConvertTo32Bits(fibOrig);
        if (fibOrig)
        {
            FreeImage_Unload(fibOrig);
            fibOrig=nullptr;
        }

        if (!fibConverted)
            return false;

        unsigned int widOrig=FreeImage_GetWidth(fibConverted);
        unsigned int heiOrig=FreeImage_GetHeight(fibConverted);

        FIBITMAP *fibRescaled=nullptr;
        if (properties && ((properties->Dimensions.Width && properties->Dimensions.Height) && (properties->Dimensions.Width!=widOrig || properties->Dimensions.Height!=heiOrig)))
            fibRescaled=FreeImage_Rescale(fibConverted, (int)properties->Dimensions.Width, (int)properties->Dimensions.Height, FILTER_BICUBIC);

        bool success=false;
        FIBITMAP *fibChosen=(fibRescaled?fibRescaled:fibConverted);
        BYTE *bits=FreeImage_GetBits(fibChosen);
        if (bits)
        {
            unsigned int wid=FreeImage_GetWidth(fibChosen);
            unsigned int hei=FreeImage_GetHeight(fibChosen);

            static const TextureCreateParameters defaultProperties;
            TextureCreateParameters createProperties=(properties?*properties:defaultProperties);
            createProperties.Dimensions.Width=(nuint)wid;
            createProperties.Dimensions.Height=(nuint)hei;

            success=CreateFromMemory(bits, &createProperties, GL_BGRA, GL_UNSIGNED_BYTE, GL_STATIC_DRAW);
        }

        if (fibRescaled)
        {
            FreeImage_Unload(fibRescaled);
            fibRescaled=nullptr;
        }

        FreeImage_Unload(fibConverted);
        fibConverted=nullptr;

        return success;
    }
#endif

    bool TextureBase::CreateFromMemory(void *sourceData, const TextureCreateParameters *properties, GLint pixelFormat, GLenum pixelSize, GLenum usage)
    {
        MPMAProfileScope("GFX::TextureBase::CreateFromMemory");

        static const TextureCreateParameters defaultProperties;
        if (!properties)
            return false;

        if (properties->Recreate || !Data().object)
            Create();

        //copy into the texture
        glBindTexture(target, *this);

        GFX::ClearGLErrors();

        if (properties->GenerateMipMaps)
        {
            gluBuild2DMipmaps(target, properties->Format, (GLint)properties->Dimensions.Width, (GLint)properties->Dimensions.Height, pixelFormat, pixelSize, sourceData);
        }
        else
        {
            glTexImage2D(target, 0, properties->Format, (GLsizei)properties->Dimensions.Width, (GLsizei)properties->Dimensions.Height, 0, pixelFormat, pixelSize, sourceData);
        }

        if (GFX::GetGLError())
            return false;

        //set some defaults
        SetDefaultProperties();

        Data().origDimensions.Width=properties->Dimensions.Width;
        Data().origDimensions.Height=properties->Dimensions.Height;

        glBindTexture(target, 0);
        return true;
    }

    void TextureBase::SetDefaultProperties()
    {
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}

bool mpmaForceReferenceToTextureCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_GFX
