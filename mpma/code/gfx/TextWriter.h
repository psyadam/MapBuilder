//!\file TextWriter.h Helper for rendering text.
//Luke Lenhart, 2010
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/Types.h"
#include "../base/Vary.h"
#include "../gfxsetup/GL.h"
#include "../gfxsetup/Extensions.h"
#include "Texture.h"
#include "Vertex.h"
#include <string>
#include <cstdint>

#ifdef GFX_USES_FREETYPE

namespace
{
    struct FontCacheEntry;
}

namespace GFX
{
    const std::string VARIABLE_SERIF="_BUILTIN_VARIABLE_SERIF"; //!<May be used for TextFont.  A font suitable for the current system will be selected with these properties.
    const std::string VARIABLE_SANSERIF="_BUILTIN_VARIABLE_SANSERIF"; //!<May be used for TextFont.  A font suitable for the current system will be selected with these properties.
    const std::string FIXED_SERIF="_BUILTIN_FIXED_SERIF"; //!<May be used for TextFont.  A font suitable for the current system will be selected with these properties.
    const std::string FIXED_SANSERIF="_BUILTIN_FIXED_SANSERIF"; //!<May be used for TextFont.  A font suitable for the current system will be selected with these properties.

    //!Represents a text color change.
    struct TextColor
    {
        uint8_t Red; //!<
        uint8_t Green; //!<
        uint8_t Blue; //!<
        uint8_t Opacity; //!<From 0=transparent to 255=opaque

        //!
        TextColor(): Red(255), Green(255), Blue(255), Opacity(255) {}
        //!
        TextColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity=255): Red(red), Green(green), Blue(blue), Opacity(opacity) {}
    };

    //!Text encoded with color information.
    class EncodedText
    {
    public:
        //!
        inline EncodedText() {}
        //!
        inline EncodedText(const std::string &plainText)
        {
            AppendPlainText(plainText);
        }

        //!Clears all stored data.
        inline void Clear()
        {
            data.clear();
        }

        //!Appends a color change to the text stream.
        void AppendColorChange(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity=255);
        //!Appends a color change to the text stream.
        inline void AppendColorChange(const TextColor &color)
        {
            AppendColorChange(color.Red, color.Green, color.Blue, color.Opacity);
        }
        //!Appends text to the text stream.
        void AppendPlainText(const std::string &text);

        //!Used for ostream-like notation to append a text string.
        EncodedText& operator<<(const char *sstring);
        //!Used for ostream-like notation to append a text string.
        EncodedText& operator<<(const std::string &sstring);
        //!Used for ostream-like notation to append a text string.
        EncodedText& operator<<(const MPMA::Vary &vstring);
        //!Used for ostream-like notation to append a color change.
        EncodedText& operator<<(const TextColor &color);
        //!Used for ostream-like notation.
        EncodedText& operator<<(const EncodedText &et);
    private:
        std::vector<uint8> data;

        friend class TextWriter;
    };

    /*!
        \brief Generates and renders a transparent texture with opaque text.
        \details General use is as follows: (Step 1) Set the desired text information to be rendered then call CreateTextImage to rebuild the image in memory.  (Step 2) Set the view clipping for the text then call CreateTexture to load the image into an OpenGL texture.  (Step 3) Call RenderTexture to draw the texture.
	*/
    class TextWriter
    {
    public:
        //!Clears the internal cache of font images (every type and size combination ever rendered results in a set of images being stored for re-use).
        static void ClearFontCache();

        // -- Step 1: Convert the text to an internal image.

        //!The text to render.
        EncodedText Text;

        //!Size of font to render with.
        nuint FontSize;

        //!Line spacing.  Default value of 0 means to use FontSize.
        nuint LineSpacing;

        //!Filename of the font to render with, or one of the special font constants
        std::string TextFont;

        //!The max width of the text image, in pixels, after which text will be wrapped to the next line.  Set to 0 to disable wrapping.
        nuint TextWidth;

        //!Recreates the internal text image based on the input parameters.
        bool CreateTextImage();

        //!The height of the internal text image that was created.
        inline nuint GetTextHeight() const { return textHeight; }

        //!The width of the internal text image that was created.
        inline nuint GetTextWidth() const { return sourceTextWidth; }

        // -- Step 2: Convert the internal image to a texture that can be used for rendering.

        //!The height of the view window that will be placed onto the texture, or 0 to show all of it.
        nuint ViewWindowHeight;

        //!This is a value between 0 (top) and GetMaxViewWindowScroll() (bottom) to control which horizontal section of the internal image to create the texture from.
        nuint ViewWindowScroll;

        //!Retrieves the maximum value ViewWindowScroll can be set to, which is equivilent to scrolling to the bottom of the internal image.
        inline nuint GetMaxViewWindowScroll() const
        {
            nsint actualViewHeight=(nsint)GetActualViewHeight();
            nsint extraHeightPixels=textHeight-actualViewHeight;
            if (extraHeightPixels<0)
                extraHeightPixels=0;
            return (nuint)extraHeightPixels;
        }

        //!Draws a border on th edges of the texture and the usable texture portions for debugging purposes.
        bool DebugDrawBorder;

        //!Renders the internal text image to a texture based on the texture size and view window.  This also updates the render properties.
        bool CreateTexture();

        //!The width of the texture that was generated.
        inline nuint GetTextureWidth();
        //!The height of the texture that was generated.
        inline nuint GetTextureHeight();

        // -- Step 3: Render the texture using these properties.

        //!The texture used for rendering the text block.
        inline Texture2D& GetTexture() { return texture; }

        //!The u texture coordinate representing the left side of the texture space to be rendered.
        inline float GetTexCoordUMin() const { return renderUMin; }
        //!The u texture coordinate representing the right side of the texture space to be rendered.
        inline float GetTexCoordUMax() const { return renderUMax; }
        //!The v texture coordinate representing the bottom side of the texture space to be rendered.
        inline float GetTexCoordVMin() const { return renderVMin; }
        //!The v texture coordinate representing the top side of the texture space to be rendered.
        inline float GetTexCoordVMax() const { return renderVMax; }

        //!The screen space width to render the texture at in order to attain 1-1 mapping with the intended text size.
        inline nuint GetRenderWidth() const { return renderWidth; }
        //!The screen space height to render the texture at in order to attain 1-1 mapping with the intended text size.
        inline nuint GetRenderHeight() const { return renderHeight; }

        //!This renders the text texture starting from the lower left location specified by x and y.  This assumes the standard 2D projection transformation is already set.
        void RenderTexture(float leftX, float topY);

        // --

        //
        TextWriter(); //!<
        TextWriter(const TextWriter &other); //!<copy constructor
        TextWriter(TextWriter &&other); //!<move constructor

    private:
        void Init();
        void CopyValues(const TextWriter &other, bool copyMovableValues);

        std::vector<uint32> textPixels;
        nuint textHeight;
        nuint sourceTextWidth;

        struct CharacterRenderListEntry
        {
            nuint CharacterIndex;
            nuint X;
            nuint Y;
            uint32 Color;
        };
        nuint ComputeWrappedCharacterRendering(const FontCacheEntry &font, std::vector<CharacterRenderListEntry> &outRenderList);
        void ComputeUnboundedCharacterRendering(const FontCacheEntry &font, std::vector<CharacterRenderListEntry> &outRenderList, nuint &outTextWidth, nuint &outTextHeight);

        nuint textureWidth, textureHeight;

        Texture2D texture;
        VertexBuffer vb;
        InterleavedVertexFormat vbFormat;
        void InitGL();
        void SetVB(float leftX, float bottomY);

        float renderUMin, renderUMax;
        float renderVMin, renderVMax;
        nuint renderWidth, renderHeight;

        inline nuint GetActualViewHeight() const
        {
            nuint actualViewHeight=ViewWindowHeight;

            if (actualViewHeight==0)
                actualViewHeight=GetTextHeight();

            if (actualViewHeight>GetTextHeight())
                actualViewHeight=GetTextHeight();

            return actualViewHeight;
        }

        std::vector<uint32> texturePixels; //temporary cache during CreateTexture, doesn't need persisted

        bool createTextureEverCalled;
        bool createTextImageEverCalled;
    };
}

#endif //#ifdef GFX_USES_FREETYPE

#endif //#ifdef MPMA_COMPILE_GFX
