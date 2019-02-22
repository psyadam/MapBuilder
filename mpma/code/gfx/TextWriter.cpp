//!\file TextWriter.cpp Helper for rendering text.
//Luke Lenhart, 2010
//See /docs/License.txt for details on how this code may be used.

#define PROFILE_TEXT_WRITER

#include "TextWriter.h"

#ifdef MPMA_COMPILE_GFX

#ifdef GFX_USES_FREETYPE

#include "../base/DebugRouter.h"
#include "../Setup.h"

#include <utility>
#include <unordered_map>
#include <array>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef PROFILE_TEXT_WRITER
    #include "../base/Profiler.h"
#endif

namespace
{
    using namespace GFX;

    FT_Library ftlibrary;
    bool IsFTInited=false;

    void SetupFreeType()
    {
        int ret=FT_Init_FreeType(&ftlibrary);
        if (ret)
        {
            MPMA::ErrorReport()<<"Failed to initialize FreeType\n";
            IsFTInited=false;
        }
        else
            IsFTInited=true;
    }

    void CleanupFreeType()
    {
        if (IsFTInited)
        {
            FT_Done_FreeType(ftlibrary);
        }
        IsFTInited=false;
    }

    class AutoInitializeFreeType
    {
    public:
        AutoInitializeFreeType()
        {
            MPMA::Internal_AddInitCallback(SetupFreeType, 4500);
            MPMA::Internal_AddShutdownCallback(CleanupFreeType, 4500);
        }

    } autoInitializeFreeType;

    struct FontGlyph
    {
        std::vector<uint8_t> Pixels;
        uint32_t PixelWidth=0;
        uint32_t PixelHeight=0;

        int32_t XOffset=0;
        int32_t YOffset=0;

        std::array<uint32_t, 128> KerningAdvance; //index of glpyh after it
    };

    struct FontCacheEntryKey
    {
        FontCacheEntryKey()=default;
        FontCacheEntryKey(uint32_t size, const std::string &type): Size(size), Type(type) {}

        uint32_t Size=0;
        std::string Type;

        bool operator==(const FontCacheEntryKey &o) const
        {
            return Size==o.Size && Type==o.Type;
        }

        //convert built-in types to a file that exists on the system
        std::string TranslateType() const
        {
            std::string fontName;
#if defined(_WIN32) || defined(_WIN64)
            if (Type==VARIABLE_SERIF)
                fontName="C:\\windows\\fonts\\times.ttf"; //Times New Roman
            else if (Type==VARIABLE_SANSERIF)
                fontName="C:\\windows\\fonts\\arial.ttf"; //Arial
            else if (Type==FIXED_SERIF)
                fontName="C:\\windows\\fonts\\lucon.ttf"; //Lucida Console
            else if (Type==FIXED_SANSERIF)
                fontName="C:\\windows\\fonts\\cour.ttf"; //Courier New
#else
            if (Type==VARIABLE_SERIF)
                fontName="/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif.ttf"; //Serif
            else if (Type==VARIABLE_SANSERIF)
                fontName="/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf"; //Sans Serif
            else if (Type==FIXED_SERIF)
                fontName="/usr/share/fonts/truetype/freefont/FreeMono.ttf"; //Free Mono
            else if (Type==FIXED_SANSERIF)
                fontName="/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf"; //Monospace
#endif
            else
                fontName=Type;

            return std::move(fontName);
        }
    };

    struct FontCacheEntryKeyHasher
    {
        std::hash<uint32_t> uint32Hasher;
        std::hash<std::string> stringHasher;

        size_t operator()(const FontCacheEntryKey &key) const
        {
            return uint32Hasher(key.Size)^stringHasher(key.Type);
        }
    };

    //cached set of images of all the characters for a particular size/type combination.
    struct FontCacheEntry
    {
        std::array<FontGlyph, 128> Glyphs;

        bool BuildGlpyhs(const FontCacheEntryKey &key)
        {
            std::string faceName=key.TranslateType();
            FT_Face face;
            int ret=FT_New_Face(ftlibrary, faceName.c_str(), 0, &face);
            if (ret)
            {
                MPMA::ErrorReport()<<"Failed to load font: "<<faceName.c_str()<<" (size "<<key.Size<<")\n";
                return false;
            }

            ret=FT_Set_Pixel_Sizes(face, 0, key.Size);
            if (!ret)
            {
                for (int i=32; i<127; ++i)
                {
                    FontGlyph &g=Glyphs[i];
                    ret=FT_Load_Char(face, i, FT_LOAD_RENDER);
                    if (!ret)
                    {
                        g.PixelWidth=face->glyph->bitmap.width;
                        g.PixelHeight=face->glyph->bitmap.rows;
                        g.Pixels.resize(g.PixelWidth*g.PixelHeight);
                        size_t p=0;
                        for (int r=0; r<(int)face->glyph->bitmap.rows; ++r)
                        {
                            uint8_t *src=face->glyph->bitmap.buffer+r*face->glyph->bitmap.pitch;
                            for (int c=0; c<(int)face->glyph->bitmap.width; ++c)
                            {
                                g.Pixels[p]=*src;
                                ++src;
                                ++p;
                            }
                        }

                        for (int k=0; k<(int)g.KerningAdvance.size(); ++k)
                        {
                            g.KerningAdvance[k]=face->glyph->metrics.horiAdvance>>6;

                            if (k>=32 && k<128)
                            {
                                FT_Vector kernVec;
                                ret=FT_Get_Kerning(face, i, k, FT_KERNING_DEFAULT, &kernVec);
                                if (!ret)
                                    g.KerningAdvance[k]+=kernVec.x>>6;
                            }
                        }

                        g.XOffset=face->glyph->bitmap_left;
                        g.YOffset=-face->glyph->bitmap_top;
                    }
                    else
                    {
                        for (auto &ka:g.KerningAdvance)
                            ka=0;
                    }
                }
            }

            FT_Done_Face(face);

            return true;
        }
    };

    std::unordered_map<FontCacheEntryKey, FontCacheEntry, FontCacheEntryKeyHasher> fontCache;

    //clears all cached fonts
    void ClearFontCache()
    {
        fontCache.clear();
    }

    //retrieves the images that make up a particular font type and size combination.  If not already cached it will be built and added to the cache.
    const FontCacheEntry& GetFont(nuint size, const std::string &type)
    {
        /*
        std::map<uint32, FontCacheEntry>::iterator entry=fontCache.find(FontCacheEntry::GetHash((uint32)size, type));
        if (entry!=fontCache.end())
        {
            return entry->second;
        }

        fontCache.insert(std::pair<uint32, FontCacheEntry>(FontCacheEntry::GetHash((uint32)size, type), FontCacheEntry(size, type)));
        entry=fontCache.find(FontCacheEntry::GetHash((uint32)size, type));
        entry->second.Build();
        return entry->second;*/

        auto fce=fontCache.find(FontCacheEntryKey((uint32_t)size, type));
        if (fce!=fontCache.end())
        {
            return fce->second;
        }

        fce=fontCache.emplace(std::make_pair(FontCacheEntryKey((uint32_t)size, type), FontCacheEntry())).first;
        fce->second.BuildGlpyhs(fce->first);
        return fce->second;
    }

    bool IsFontCached(nuint size, const std::string &type)
    {
        return fontCache.find(FontCacheEntryKey((uint32_t)size, type))!=fontCache.end();
    }
}

namespace GFX
{
    // -- EncodedText

    namespace
    {
        enum
        {
            ENCTEXT_COLOR=128
        };
    }

    void EncodedText::AppendColorChange(uint8_t red, uint8_t green, uint8_t blue, uint8_t opacity)
    {
        data.push_back(ENCTEXT_COLOR);
        data.push_back(red);
        data.push_back(green);
        data.push_back(blue);
        data.push_back(opacity);
    }

    void EncodedText::AppendPlainText(const std::string &text)
    {
        for (nuint i=0; i<text.length(); ++i)
        {
            uint8 c=text[i];
            if ((c>=32 && c<=127) || c=='\n')
                data.push_back(c);
            else if (c=='\t')
                data.push_back(' ');
        }
    }

    //Used for ostream-like notation to append a text string.
    EncodedText& EncodedText::operator<<(const char *sstring)
    {
        AppendPlainText(sstring);
        return *this;
    }

    //Used for ostream-like notation to append a text string.
    EncodedText& EncodedText::operator<<(const std::string &sstring)
    {
        AppendPlainText(sstring);
        return *this;
    }

    //Used for ostream-like notation to append a text string.
    EncodedText& EncodedText::operator<<(const MPMA::Vary &vstring)
    {
        AppendPlainText(vstring.AsString());
        return *this;
    }
    //Used for ostream-like notation to append a color change.
    EncodedText& EncodedText::operator<<(const TextColor &color)
    {
        AppendColorChange(color);
        return *this;
    }
    //Used for ostream-like notation.
    EncodedText& EncodedText::operator<<(const EncodedText &et)
    {
        data.insert(data.end(), et.data.begin(), et.data.end());
        return *this;
    }

    // -- TextWriter

    //Clears the internal cache of font images (every type and size combination rendered results in a set of images being stored for re-use).
    void TextWriter::ClearFontCache()
    {
        ::ClearFontCache();
    }

    //Recreates the internal text image based on the input parameters.
    bool TextWriter::CreateTextImage()
    {
#ifdef PROFILE_TEXT_WRITER
        std::string profileString="TextWriter::CreateTextImage";
        if (IsFontCached(FontSize, TextFont))
            profileString+="(cache hit)";
        else
            profileString+="(cache miss)";
        MPMAProfileScope(profileString);
#endif
        MPMAProfileScope("TextWriter::CreateTextImage");

        //validate input
        textPixels.clear();
        textHeight=0;
        if (FontSize==0)
            return false;

        //figures out the offset and properties with which to render each character
        const FontCacheEntry &font=GetFont(FontSize, TextFont);

        std::vector<CharacterRenderListEntry> renderList;
        sourceTextWidth=TextWidth;
        if (sourceTextWidth==0)
            ComputeUnboundedCharacterRendering(font, renderList, sourceTextWidth, textHeight);
        else
            textHeight=ComputeWrappedCharacterRendering(font, renderList);

        //render the character to the image
        textPixels.resize(sourceTextWidth*textHeight, 0);

        for (std::vector<CharacterRenderListEntry>::iterator rl=renderList.begin(); rl!=renderList.end(); ++rl)
        {
            nuint blockHeight=font.Glyphs[rl->CharacterIndex].PixelHeight;
            nuint blockWidth=font.Glyphs[rl->CharacterIndex].PixelWidth;
            const uint8_t *blockSource=font.Glyphs[rl->CharacterIndex].Pixels.data();

            nuint destY=rl->Y;
            nuint destX=rl->X;
            uint32 srcTextAlpha=rl->Color>>24;
            uint32 srcTextColor=rl->Color&0x00ffffff;

            //set the pixels
            for (nuint y=0; y<blockHeight; ++y)
            {
                for (nuint x=0; x<blockWidth; ++x)
                {
                    uint32 srcPixelAlpha=blockSource[y*blockWidth+x];
                    if (srcPixelAlpha)
                    {
                        uint32 alpha=srcPixelAlpha*srcTextAlpha;
                        alpha&=0x0000ff00;
                        alpha<<=16;
                        uint32_t pixel=alpha|srcTextColor;

                        uint32_t existingPixel=textPixels[(destY+y)*sourceTextWidth+destX+x];
                        uint32_t newPixel=existingPixel|pixel; //not an ideal combiner, will improve later
                        textPixels[(destY+y)*sourceTextWidth+destX+x]=newPixel;
                    }
                }
            }
        }

        createTextImageEverCalled=true;
        return true;
    }

    //Computes a list of characters to render to the image in CreateTextImage for wrapped text.  Returns the height required to render it with.
    nuint TextWriter::ComputeWrappedCharacterRendering(const struct FontCacheEntry &font, std::vector<CharacterRenderListEntry> &outRenderList)
    {
        nuint curX=0, curY=0;
        uint32 color=0xff808080; //default to gray
        nuint lineSpacing=LineSpacing;
        if (!lineSpacing)
            lineSpacing=FontSize;

        nuint maxY=1;

        nuint consecutiveLetters=0;
        nuint consecutiveLettersStartInd=0;
        nuint consecutiveLettersStartX=0;

        for (nuint i=0; i<Text.data.size(); ++i)
        {
            nuint c=Text.data[i];
            nuint nextC=0;
            for (nuint n=i+1; n<Text.data.size(); ++n)
            {
                if (Text.data[n]==ENCTEXT_COLOR)
                    n+=4;
                else if (Text.data[n]>=32 || Text.data[n]<128)
                    nextC=Text.data[n];
            }

            if (c==ENCTEXT_COLOR)
            {
                uint32 b=Text.data[i+1];
                uint32 g=Text.data[i+2];
                uint32 r=Text.data[i+3];
                uint32 a=Text.data[i+4];
                color=a<<24 | r<<16 | g<<8 | b;
                i+=4;
            }
            else if (c==' ')
            {
                curX+=font.Glyphs[32].KerningAdvance[nextC];
                if (curX>=sourceTextWidth)
                {
                    curX=0;
                    curY+=lineSpacing;
                }
                consecutiveLetters=0;
            }
            else if (c=='\n')
            {
                curX=0;
                curY+=lineSpacing;
                consecutiveLetters=0;
            }
            else //actual letters
            {
                nuint cInd=c;
                if (cInd<128 && cInd>=32)
                {
                    int32_t rlx=font.Glyphs[cInd].XOffset+(int32_t)curX;
                    int32_t rly=font.Glyphs[cInd].YOffset+(int32_t)curY;

                    if (rlx<0)
                    {
                        curX=-font.Glyphs[cInd].XOffset;
                        rlx=0;
                    }

                    if (rly<0)
                    {
                        curY=-font.Glyphs[cInd].YOffset;
                        curX=0;
                        outRenderList.clear();
                        i=(nuint)(0-1);
                        continue;
                    }

                    if (font.Glyphs[cInd].PixelWidth>sourceTextWidth) //individual character is too wide for the window
                        continue;

                    CharacterRenderListEntry rl;
                    rl.CharacterIndex=cInd;
                    rl.Color=color;
                    rl.X=rlx;
                    rl.Y=rly;
                    outRenderList.emplace_back(rl);

                    if (consecutiveLetters==0)
                    {
                        consecutiveLettersStartX=curX;
                        consecutiveLettersStartInd=i;
                    }
                    ++consecutiveLetters;

                    if (rlx+font.Glyphs[cInd].PixelWidth>=sourceTextWidth) //this char went past, need to start on next line and rewind the current word
                    {
                        if (consecutiveLettersStartX!=0)
                        {
                            for (nuint rewind=0; rewind<consecutiveLetters; ++rewind)
                                outRenderList.pop_back();

                            i=consecutiveLettersStartInd-1;
                            consecutiveLetters=0;

                            curX=0;
                            curY+=lineSpacing;
                        }
                        else //single word is too long for a line, we already tried starting a new line for it, so only rewind one letter instead then start a new line
                        {
                            outRenderList.pop_back();

                            --i;

                            curX=0;
                            curY+=lineSpacing;
                        }
                    }
                    else
                    {
                        curX+=font.Glyphs[cInd].KerningAdvance[nextC];

                        if (rly+font.Glyphs[cInd].PixelHeight>maxY)
                            maxY=rly+font.Glyphs[cInd].PixelHeight;
                    }
                }
            }
        }

        return maxY;
    }

    //Computes a list of characters to render to the image in CreateTextImage for text that is not wrapped.  Returns the width and height required to render it with.
    void TextWriter::ComputeUnboundedCharacterRendering(const FontCacheEntry &font, std::vector<CharacterRenderListEntry> &outRenderList, nuint &outTextWidth, nuint &outTextHeight)
    {
        nuint curX=0, curY=0;
        uint32 color=0xff808080; //default to gray
        nuint lineSpacing=LineSpacing;
        if (!lineSpacing)
            lineSpacing=FontSize;

        nuint maxWidth=1;
        nuint maxY=1;

        for (nuint i=0; i<Text.data.size(); ++i)
        {
            nuint c=Text.data[i];
            nuint nextC=0;
            for (nuint n=i+1; n<Text.data.size(); ++n)
            {
                if (Text.data[n]==ENCTEXT_COLOR)
                    n+=4;
                else if (Text.data[n]>=32 || Text.data[n]<128)
                    nextC=Text.data[n];
            }

            if (c==ENCTEXT_COLOR)
            {
                uint32 b=Text.data[i+1];
                uint32 g=Text.data[i+2];
                uint32 r=Text.data[i+3];
                uint32 a=Text.data[i+4];
                color=a<<24 | r<<16 | g<<8 | b;
                i+=4;
            }
            else if (c==' ')
            {
                curX+=font.Glyphs[32].KerningAdvance[nextC];
            }
            else if (c=='\n')
            {
                curX=0;
                curY+=lineSpacing;
            }
            else //actual letters
            {
                nuint cInd=c;
                if (cInd>=32 && cInd<128)
                {
                    int32_t rlx=font.Glyphs[cInd].XOffset+(int32_t)curX;
                    int32_t rly=font.Glyphs[cInd].YOffset+(int32_t)curY;

                    if (rlx<0)
                    {
                        curX=-font.Glyphs[cInd].XOffset;
                        rlx=0;
                    }

                    if (rly<0)
                    {
                        curY=-font.Glyphs[cInd].YOffset;
                        curX=0;
                        outRenderList.clear();
                        i=(nuint)(0-1);
                        continue;
                    }

                    CharacterRenderListEntry rl;
                    rl.CharacterIndex=cInd;
                    rl.Color=color;
                    rl.X=rlx;
                    rl.Y=rly;
                    outRenderList.emplace_back(rl);

                    curX+=font.Glyphs[cInd].KerningAdvance[nextC];

                    if (rlx+font.Glyphs[cInd].PixelWidth>maxWidth)
                        maxWidth=rlx+font.Glyphs[cInd].PixelWidth;

                    if (rly+font.Glyphs[cInd].PixelHeight>maxY)
                        maxY=rly+font.Glyphs[cInd].PixelHeight;
                }
            }
        }

        outTextWidth=maxWidth;
        outTextHeight=maxY;
    }

    //Renders the internal text image to a texture based on the view window.
    bool TextWriter::CreateTexture()
    {
#ifdef PROFILE_TEXT_WRITER
        MPMAProfileScope("TextWriter::CreateTexture");
#endif

        if (!createTextImageEverCalled)
        {
            if (!CreateTextImage())
                return false;
        }

        if (GetTextHeight()==0 || textPixels.empty())
            return false;

        nuint actualViewHeight=GetActualViewHeight();

        //figure out the texture dimensions
        textureWidth=2;
        textureHeight=2;

        while (textureWidth<sourceTextWidth)
            textureWidth<<=1;

        while (textureHeight<actualViewHeight)
            textureHeight<<=1;

        //figure out the view window offset
        nuint veiwHeightOffset=ViewWindowScroll;
        if (veiwHeightOffset>GetMaxViewWindowScroll())
            veiwHeightOffset=GetMaxViewWindowScroll();

        uint32 *srcPixels=&textPixels[0] + sourceTextWidth*veiwHeightOffset;

        //copy a chunk of the image into memory that fits the texture and compute the render height
        renderWidth=1;
        renderHeight=1;

        texturePixels.clear();
        texturePixels.reserve(textureWidth*textureHeight);
        for (nuint y=0; y<actualViewHeight; ++y)
        {
            for (nuint x=0; x<sourceTextWidth; ++x)
            {
                uint32 p=*srcPixels;

                if (p&0xff000000) //opaque pixel
                {
                    if (renderWidth<x)
                        renderWidth=x;

                    if (renderHeight<y)
                        renderHeight=y;
                }

                texturePixels.push_back(p);

                ++srcPixels;
            }
            for (nuint x=sourceTextWidth; x<textureWidth; ++x)
                texturePixels.push_back(0);
        }
        for (nuint y=actualViewHeight; y<textureHeight; ++y)
        {
            for (nuint x=0; x<textureWidth; ++x)
                texturePixels.push_back(0);
        }

        if (DebugDrawBorder)
        {
            //texture edges
            for (nuint y=0; y<textureHeight; ++y)
            {
                texturePixels[y*textureWidth]=0xff000000 | (rand()%0xffffff);
                texturePixels[y*textureWidth+textureWidth-1]=0xff000000 | (rand()%0xffffff);
            }

            for (nuint x=0; x<textureWidth; ++x)
            {
                texturePixels[x]=0xff000000 | (rand()%0xffffff);
                texturePixels[textureWidth*(textureHeight-1)+x]=0xff000000 | (rand()%0xffffff);
            }

            //usable section
            for (nuint y=0; y<renderHeight; ++y)
            {
                texturePixels[y*textureWidth+renderWidth]=0xff000000 | (rand()%0xffffff);
            }

            for (nuint x=0; x<renderWidth; ++x)
            {
                texturePixels[textureWidth*(renderHeight)+x]=0xff000000 | (rand()%0xffffff);
            }
        }

        //figure out texture coordinates to match the rendering width
        ++renderWidth;
        ++renderHeight;

        renderUMin=0;
        renderUMax=(float)renderWidth/textureWidth;
        renderVMin=0;
        renderVMax=(float)renderHeight/textureHeight;

        //load that into the texture
        TextureCreateParameters textureProperties;
        textureProperties.GenerateMipMaps=false;
        textureProperties.Dimensions.Width=textureWidth;
        textureProperties.Dimensions.Height=textureHeight;
        textureProperties.Recreate=false;

        bool ret=texture.CreateFromMemory(&texturePixels[0], &textureProperties, GL_RGBA, GL_UNSIGNED_BYTE, GL_DYNAMIC_DRAW);
        texturePixels.clear();

        AutoBindTexture autoBind(texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        createTextureEverCalled=true;
        return ret;
    }

    void TextWriter::SetVB(float leftX, float topY)
    {
        leftX=std::floor(leftX);
        topY=std::floor(topY);

        float rightX=leftX+GetRenderWidth();
        float bottomY=topY-GetRenderHeight();

        struct VertStruct
        {
            float pos[3];
            float texcoord[2];
        };

        VertStruct verts[6]=
        {
            {{leftX,  topY,    0}, {GetTexCoordUMin(), GetTexCoordVMin()}},
            {{rightX, topY,    0}, {GetTexCoordUMax(), GetTexCoordVMin()}},
            {{leftX,  bottomY, 0}, {GetTexCoordUMin(), GetTexCoordVMax()}},
            {{leftX,  bottomY, 0}, {GetTexCoordUMin(), GetTexCoordVMax()}},
            {{rightX, topY,    0}, {GetTexCoordUMax(), GetTexCoordVMin()}},
            {{rightX, bottomY, 0}, {GetTexCoordUMax(), GetTexCoordVMax()}}
        };

        vb.LoadInterleaved(vbFormat, verts, 6);
    }

    //This renders the text texture starting from the upper left location specified by x and y.  This assumes the standard 2D projection transformation is already set.
    void TextWriter::RenderTexture(float leftX, float topY)
    {
#ifdef PROFILE_TEXT_WRITER
        MPMAProfileScope("TextWriter::RenderTexture");
#endif

        if (!createTextureEverCalled)
        {
            if (!CreateTexture())
                return;
        }

        SetVB(leftX, topY);

        GFX::AutoBindTexture autoTexture0(texture, 0);
        GFX::AutoBindVertexBuffer autoVB(vb);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    //ctors
    TextWriter::TextWriter()
    {
        Init();
        InitGL();
    }

    TextWriter::TextWriter(const TextWriter &other)
    {
        CopyValues(other, true);

        //texture is reference counted so isn't copied but recreated, vb is new
        CreateTexture();
        vbFormat=other.vbFormat;
    }

    TextWriter::TextWriter(TextWriter &&other)
    {
        CopyValues(other, false);

        //old reference counted objects are used by the new instance
        texture=other.texture;
        vb=other.vb;

        //these can be moved
        textPixels=std::move(other.textPixels);
        vbFormat=std::move(other.vbFormat);
    }

    void TextWriter::CopyValues(const TextWriter &other, bool copyMovableValues)
    {
        Text=other.Text;
        FontSize=other.FontSize;
        TextFont=other.TextFont;

        TextWidth=other.TextWidth;
        sourceTextWidth=other.sourceTextWidth;
        textHeight=other.textHeight;
        textureWidth=other.textureWidth;
        textureHeight=other.textureHeight;
        ViewWindowHeight=other.ViewWindowHeight;
        ViewWindowScroll=other.ViewWindowScroll;
        DebugDrawBorder=other.DebugDrawBorder;
        renderUMin=other.renderUMin;
        renderUMax=other.renderUMax;
        renderVMin=other.renderVMin;
        renderVMax=other.renderVMax;
        renderWidth=other.renderWidth;
        renderHeight=other.renderHeight;
        createTextureEverCalled=other.createTextureEverCalled;
        createTextImageEverCalled=other.createTextImageEverCalled;

        if (copyMovableValues)
        {
            textPixels=other.textPixels;
        }
    }

    void TextWriter::Init()
    {
        TextWidth=0;
        sourceTextWidth=0;
        textHeight=0;
        textureWidth=0;
        textureHeight=0;
        ViewWindowHeight=0;
        ViewWindowScroll=0;
        DebugDrawBorder=false;
        renderUMin=0;
        renderUMax=0;
        renderVMin=0;
        renderVMax=0;
        renderWidth=0;
        renderHeight=0;
        createTextureEverCalled=false;
        createTextImageEverCalled=false;

        FontSize=12;
        LineSpacing=0;
        TextFont=VARIABLE_SERIF;
    }

    void TextWriter::InitGL()
    {
        texture.Create();
        vb.Create();

        vbFormat.Usage=GL_DYNAMIC_DRAW;
        vbFormat<<GFX::VertexComponentFormat(GFX::VertexPosition,           GL_FLOAT, 3);
        vbFormat<<GFX::VertexComponentFormat(GFX::VertexTextureCoordinate0, GL_FLOAT, 2);
    }
}

bool mpmaForceReferenceToTextWriterCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef GFX_USES_FREETYPE

#endif //MPMA_COMPILE_GFX
