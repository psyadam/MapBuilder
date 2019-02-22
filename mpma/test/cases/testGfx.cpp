//tests that the geo library functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "base/Timer.h"
#include "gfx/Texture.h"
#include "gfx/Vertex.h"
#include "gfx/Shader.h"
#include "gfx/TextWriter.h"
#include "gfxsetup/GFXSetup.h"
#include "gfxsetup/GL.h"
#include "geo/Geo.h"
#include <GL/glu.h>
#include <iostream>
#include <cmath>

#ifdef MPMA_COMPILE_GEO
#ifdef DECLARE_TESTS_CODE
class TextureBasic: public UnitTest
{
public:
    bool Run()
    {
        //setup
        GFX::GraphicsSetup setup;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        {
            //load textures
            GFX::Texture2D tex0;
            if (!tex0.CreateFromFile("data/testGfxOpaque256x256.jpg"))
            {
                std::cout<<"CreateFromFile failed.\n";
                ok=false;
            }

            GFX::Texture2D tex1;
            GFX::TextureCreateParameters tex1create;
            if (!tex1.CreateFromFile("data/testGfxOpaque256x256.jpg", &tex1create))
            {
                std::cout<<"CreateFromFile with default properties failed.\n";
                ok=false;
            }

            GFX::Texture2D tex2;
            GFX::TextureCreateParameters tex2create;
            tex2create.Dimensions.Width=64;
            tex2create.Dimensions.Height=64;
            //tex2create.Compress=true;
            tex2create.GenerateMipMaps=false;
            tex2create.Format=GL_RGBA4;
            if (!tex2.CreateFromFile("data/testGfxOpaque256x256.jpg", &tex2create))
            {
                std::cout<<"CreateFromFile with altered properties failed.\n";
                ok=false;
            }

            if (ok)
            {
                //render a bit
                MPMA::Timer timer;
                for (double total=0; total<10; total+=timer.Step())
                {
                    //do some default random stuff
                    GFX::UpdateWindow();

                    const GFX::GraphicsSetup *state=GFX::GetWindowState();
                    if (state==0)
                        break;

                    //calc a rotation amount
                    float amt=0.25f*(float)total*2*3.14f;

                    //draw a triangle!
                    glViewport(0, 0, state->Width, state->Height);

                    glClearColor(1, 1, 1, 1);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                    glMatrixMode(GL_PROJECTION);
                    GEO::Matrix4 matProj=GEO::MatProjectionFoV(1, 1, 1, 20);
                    glLoadMatrixf(matProj.ToGL());

                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    GEO::Matrix4 matView=GEO::MatViewLookAt(GEO::Vector3(0,0,2), GEO::Vector3(0,0,0), GEO::Vector3(0,1,0));
                    glLoadMatrixf(matView.ToGL());

                    {
                        GFX::AutoBindTexture autoTexture0(tex0, 0);
                        glBegin(GL_TRIANGLES);
                        glTexCoord2f(0.5f, 0); glColor3f(1, 0, 0); glVertex3f(std::sin(amt)*.1f, std::cos(amt)*.1f, 0);
                        glTexCoord2f(0, 1); glColor3f(0, 1, 0); glVertex3f(std::sin(amt+1.05f), std::cos(amt+1.05f), 0);
                        glTexCoord2f(1, 1); glColor3f(0, 0, 1); glVertex3f(std::sin(amt+2.1f), std::cos(amt+2.1f), 0);
                        glEnd();
                    }

                    {
                        GFX::AutoBindTexture autoTexture0(tex1, 0);
                        glBegin(GL_TRIANGLES);
                        glTexCoord2f(0.5f, 0); glColor3f(1, 0, 0); glVertex3f(std::sin(amt+GEO::PI*2/3)*.1f, std::cos(amt+GEO::PI*2/3)*.1f, 0);
                        glTexCoord2f(0, 1); glColor3f(0, 1, 0); glVertex3f(std::sin(amt+1.05f+GEO::PI*2/3), std::cos(amt+1.05f+GEO::PI*2/3), 0);
                        glTexCoord2f(1, 1); glColor3f(0, 0, 1); glVertex3f(std::sin(amt+2.1f+GEO::PI*2/3), std::cos(amt+2.1f+GEO::PI*2/3), 0);
                        glEnd();
                    }

                    {
                        GFX::AutoBindTexture autoTexture0(tex2, 0);
                        glBegin(GL_TRIANGLES);
                        glTexCoord2f(0.5f, 0); glColor3f(1, 0, 0); glVertex3f(std::sin(amt+GEO::PI*4/3)*.1f, std::cos(amt+GEO::PI*4/3)*.1f, 0);
                        glTexCoord2f(0, 1); glColor3f(0, 1, 0); glVertex3f(std::sin(amt+1.05f+GEO::PI*4/3), std::cos(amt+1.05f+GEO::PI*4/3), 0);
                        glTexCoord2f(1, 1); glColor3f(0, 0, 1); glVertex3f(std::sin(amt+2.1f+GEO::PI*4/3), std::cos(amt+2.1f+GEO::PI*4/3), 0);
                        glEnd();
                    }
                }
            }
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(TextureBasic);


#ifdef DECLARE_TESTS_CODE
class VertexIndexBufferTest: public UnitTest
{
public:
    bool Run()
    {
        //setup
        GFX::GraphicsSetup setup;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        {
            //load texture
            GFX::Texture2D tex0;
            if (!tex0.CreateFromFile("data/testGfxOpaque256x256.jpg"))
            {
                std::cout<<"Texture CreateFromFile failed.\n";
                ok=false;
            }

            //create vertex buffer
            GFX::InterleavedVertexFormat vbFormat;
            vbFormat.Usage=GL_STATIC_DRAW;
            vbFormat<<GFX::VertexComponentFormat(GFX::VertexPosition,           GL_FLOAT, 3);
            vbFormat<<GFX::VertexComponentFormat(GFX::VertexTextureCoordinate0, GL_FLOAT, 2);

            struct VertStruct
            {
                float pos[3];
                float tex[2];
            };
            VertStruct verts[4]=
            {
                {{-1, -1, 0}, {0, 0}},
                {{1, -1, 0}, {1, 0}},
                {{1, 1, 0}, {1, 1}},
                {{-1, 1, 0}, {0, 1}}
            };

            GFX::VertexBuffer vb;
            vb.LoadInterleaved(vbFormat, verts, 4);

            //create index buffer
            uint16 inds[6]=
            {
                0, 1, 2,
                2, 0, 3
            };

            GFX::IndexBuffer ib;
            ib.Load(inds, 6);

            if (ok)
            {
                //render a bit
                MPMA::Timer timer;
                for (double total=0; total<10; total+=timer.Step())
                {
                    //do some default random stuff
                    GFX::UpdateWindow();

                    const GFX::GraphicsSetup *state=GFX::GetWindowState();
                    if (state==0)
                        break;

                    //calc a rotation amount
                    float amt=0.25f*(float)total*2*3.14f;

                    //draw a triangle!
                    glViewport(0, 0, state->Width, state->Height);

                    glClearColor(1, 1, 1, 1);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                    glMatrixMode(GL_PROJECTION);
                    GEO::Matrix4 matProj=GEO::MatProjectionFoV(1, 1, 1, 20);
                    glLoadMatrixf(matProj.ToGL());

                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    GEO::Matrix4 matView=GEO::MatViewLookAt(GEO::Vector3(0,0,3), GEO::Vector3(0,0,0), GEO::Vector3(sin(amt),cos(amt),0));
                    glLoadMatrixf(matView.ToGL());

                    {
                        GFX::AutoBindTexture autoTexture0(tex0, 0);
                        GFX::AutoBindVertexBuffer autoVB(vb);
                        GFX::AutoBindIndexBuffer autoIB(ib);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
                    }
                }
            }
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(VertexIndexBufferTest);

#ifdef DECLARE_TESTS_CODE
class ShaderTest: public UnitTest
{
public:
    bool Run()
    {
        //setup
        GFX::GraphicsSetup setup;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        {
            //load texture
            GFX::Texture2D tex0;
            if (!tex0.CreateFromFile("data/testGfxOpaque256x256.jpg"))
            {
                std::cout<<"Texture CreateFromFile failed.\n";
                ok=false;
            }

            //create vertexand index buffers
            GFX::InterleavedVertexFormat vbFormat;
            vbFormat.Usage=GL_STATIC_DRAW;
            vbFormat<<GFX::VertexComponentFormat(GFX::VertexPosition,           GL_FLOAT, 3);

            struct VertStruct
            {
                float pos[3];
            };
            VertStruct verts[4]=
            {
                {{-1, -1, 0}},
                {{1, -1, 0}},
                {{1, 1, 0}},
                {{-1, 1, 0}}
            };

            GFX::VertexBuffer vb;
            vb.LoadInterleaved(vbFormat, verts, 4);

            uint16 inds[6]=
            {
                0, 1, 2,
                2, 0, 3
            };

            GFX::IndexBuffer ib;
            ib.Load(inds, 6);

            //compile and link the shader
            GFX::ShaderCode shdVert, shdFrag;
            if (!shdVert.CompileFile(GL_VERTEX_SHADER, "data/testShaderVert0.glsl"))
            {
                std::cout<<"Failed to compile vertex shader.\n";
                ok=false;
            }

            if (!shdFrag.CompileFile(GL_FRAGMENT_SHADER, "data/testShaderFrag0.glsl"))
            {
                std::cout<<"Failed to compile fragment shader.\n";
                ok=false;
            }

            GFX::ShaderProgram prog;
            prog.Create();
            prog.AttachShader(shdVert);
            prog.AttachShader(shdFrag);
            if (!prog.Link())
            {
                std::cout<<"Failed to link shader program.\n";
                ok=false;
            }

            if (ok)
            {
                //render a bit
                MPMA::Timer timer;
                for (double total=0; total<10; total+=timer.Step())
                {
                    //do some default random stuff
                    GFX::UpdateWindow();

                    const GFX::GraphicsSetup *state=GFX::GetWindowState();
                    if (state==0)
                        break;

                    //draw a triangle!
                    glViewport(0, 0, state->Width, state->Height);

                    glClearColor(1, 1, 1, 1);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                    glMatrixMode(GL_PROJECTION);
                    GEO::Matrix4 matProj=GEO::MatProjectionFoV(1, 1, 1, 20);
                    glLoadMatrixf(matProj.ToGL());

                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    GEO::Matrix4 matView=GEO::MatViewLookAt(GEO::Vector3(0,0,3), GEO::Vector3(0,0,0), GEO::Vector3(0,1,0));
                    glLoadMatrixf(matView.ToGL());

                    {
                        GFX::AutoBindShaderProgram autoProg(prog);
                        prog.FindVariable("testSampler").SetTextureStage(0);
                        prog.FindVariable("time").SetFloat1((float)total);


                        GFX::AutoBindTexture autoTexture0(tex0, 0);
                        GFX::AutoBindVertexBuffer autoVB(vb);
                        GFX::AutoBindIndexBuffer autoIB(ib);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
                    }
                }
            }
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(ShaderTest);
#endif //MPMA_COMPILE_GEO

#ifdef GFX_USES_FREETYPE
#ifdef DECLARE_TESTS_CODE
class TextWriterTest: public UnitTest
{
public:
    bool Run()
    {
        //setup
        GFX::GraphicsSetup setup;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        {
            //simple static texts
            GFX::TextWriter text1a;
            //text1a.DebugDrawBorder=true;
            text1a.TextWidth=0;
            text1a.Text=GFX::EncodedText("Hello World");
            text1a.TextFont=GFX::VARIABLE_SERIF;

            ok=text1a.CreateTextImage();
            if (!ok)
            {
                std::cout<<"CreateTextImage 1a failed"<<std::endl;
                return false;
            }

            ok=text1a.CreateTexture();
            if (!ok)
            {
                std::cout<<"CreateTexture 1a failed"<<std::endl;
                return false;
            }

            GFX::TextWriter text1b;
            //text1b.DebugDrawBorder=true;
            text1b.TextWidth=0;
            text1b.Text=GFX::EncodedText("Hello World");
            text1b.TextFont=GFX::VARIABLE_SANSERIF;

            ok=text1b.CreateTextImage();
            if (!ok)
            {
                std::cout<<"CreateTextImage 1b failed"<<std::endl;
                return false;
            }

            ok=text1b.CreateTexture();
            if (!ok)
            {
                std::cout<<"CreateTexture 1b failed"<<std::endl;
                return false;
            }

            //one we'll animate scrolling on
            GFX::TextWriter text2;
            //text2.DebugDrawBorder=true;
            text2.TextWidth=100;
            text2.ViewWindowHeight=50;

            std::string lastWord=" blossemed...";

            text2.Text.Clear();
            text2.Text<<GFX::TextColor(255, 255, 255)<<"A long time ago, in a "<<GFX::TextColor(0, 0, 255)<<"galaxy"<<GFX::TextColor(255, 255, 255)<<" far far away, there existed a";
            text2.Text<<GFX::TextColor(0, 255, 0)<<" programmer"<<GFX::TextColor(255, 255, 255)<<".  The "<<GFX::TextColor(0, 255, 0)<<"programmer "<<GFX::TextColor(255, 255, 255);
            text2.Text<<"had "<<2<<" "<<GFX::TextColor(255, 0, 0)<<"cats"<<GFX::TextColor(255, 255, 255)<<".  Life was good.  Then one day, the"<<GFX::TextColor(255, 255, 0)<<" tomato plant";
            text2.Text<<GFX::TextColor(255, 255, 255)<<lastWord;

            ok=text2.CreateTextImage();
            if (!ok)
            {
                std::cout<<"CreateTextImage 2 failed"<<std::endl;
                return false;
            }

            ok=text2.CreateTexture();
            if (!ok)
            {
                std::cout<<"CreateTexture 2 failed"<<std::endl;
                return false;
            }

            //This one uses a large text size without serifs
            GFX::TextWriter text3;
            //text3.DebugDrawBorder=true;
            text3.TextWidth=500;
            text3.FontSize=85;

            text3.Text.Clear();
            text3.Text<<GFX::TextColor(255, 255, 255)<<"Really "<<GFX::TextColor(255, 255, 255, 192)<<"big"<<GFX::TextColor(255, 255, 255, 128)<<" transparent"<<GFX::TextColor(255, 255, 255, 64)<<" text.";

            ok=text3.CreateTextImage();
            if (!ok)
            {
                std::cout<<"CreateTextImage 3 failed"<<std::endl;
                return false;
            }

            ok=text3.CreateTexture();
            if (!ok)
            {
                std::cout<<"CreateTexture 3 failed"<<std::endl;
                return false;
            }

            //This one uses a fixed-width font, and the text is updated every frame
            GFX::TextWriter text4;
            //text4.DebugDrawBorder=true;
            text4.TextFont=GFX::FIXED_SANSERIF;

            nuint frameCount=0;

            //
            if (ok)
            {
                //render a bit
                MPMA::Timer timer;
                for (double total=0; total<10; total+=timer.Step())
                {
                    //update
                    GFX::UpdateWindow();

                    const GFX::GraphicsSetup *state=GFX::GetWindowState();
                    if (state==0)
                        break;

                    //update text2
                    float mod=fmod((float)total*0.2f, 2.0f);
                    if (mod>1.0f)
                        mod=2.0f-mod;
                    text2.ViewWindowScroll=(nuint)(mod*text2.GetMaxViewWindowScroll());
                    ok=text2.CreateTexture();
                    if (!ok)
                    {
                        std::cout<<"CreateTexture 2b failed"<<std::endl;
                        return false;
                    }

                    //update text4
                    text4.Text.Clear();
                    text4.Text<<"Fixed width font.\nFrame count: "<<++frameCount;

                    ok=text4.CreateTextImage();
                    if (!ok)
                    {
                        std::cout<<"CreateTextImage 4 failed"<<std::endl;
                        return false;
                    }

                    ok=text4.CreateTexture();
                    if (!ok)
                    {
                        std::cout<<"CreateTexture 4 failed"<<std::endl;
                        return false;
                    }

                    //draw the text
                    glViewport(0, 0, state->Width, state->Height);

                    glClearColor(0.1f, 0.1f, 0.1f, 1);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                    glDisable(GL_DEPTH_TEST);

                    glMatrixMode(GL_PROJECTION);
                    //GEO::Matrix4 matProj=GEO::MatProjectionOrtho(0, state->Width, 0, state->Height); //TODO: I think this is broken, look into it
                    //glLoadMatrixf(matProj.ToGL());
                    glLoadIdentity();
                    gluOrtho2D(0, state->Width, 0, state->Height);

                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();

                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    text1a.RenderTexture(10, 25);
                    text1b.RenderTexture(10, 50);
                    text2.RenderTexture(10, 200);
                    text3.RenderTexture(10, 400);
                    text4.RenderTexture(10, 500);
                }
            }
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif //DECLARE_TESTS_CODE
DECLARE_TEST_CASE(TextWriterTest);
#endif //GFX_USES_FREETYPE
