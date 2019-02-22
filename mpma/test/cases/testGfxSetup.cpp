//tests that the geo library functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "base/Timer.h"
#include "gfxsetup/GFXSetup.h"
#include "gfxsetup/GL.h"
#include "gfxsetup/Extensions.h"
#include "geo/Geo.h"
#include <GL/glu.h>
#include <iostream>
#include <cmath>

#ifdef MPMA_COMPILE_GEO
#ifdef DECLARE_TESTS_CODE
class GfxSetup: public UnitTest
{
public:
    bool Run()
    {
        GFX::GraphicsSetup setup;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        GFX::ScreenSize screenSize=GFX::GetCurrentScreenSize();
        std::cout<<"Current screen size: "<<screenSize.Width<<"x"<<screenSize.Height<<std::endl;

        //setup.Resizable=false;
        setup.Width+=50;
        int expectedWidth=setup.Width;
        //setup.Minimized=true;
        GFX::SetupWindow(setup);

        const GFX::GraphicsSetup *tmp=GFX::GetWindowState();
        if (tmp!=0)
        {
            if (tmp->Width!=expectedWidth)
            {
                std::cout<<"Width is wrong.  Expected "<<expectedWidth<<" but got "<<tmp->Width<<std::endl;
                ok=false;
            }
        }
        
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
            //GEO::Matrix4 matProj=GEO::MatProjectionOrtho(-1, 1, -1, 1, 1, 20);
            GEO::Matrix4 matProj=GEO::MatProjectionFoV(1, 1, 1, 20);
            glLoadMatrixf(matProj.ToGL());
            //glLoadIdentity();
            //glOrtho(-1, 1, -1, 1, 1, 20);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            GEO::Matrix4 matView=GEO::MatViewLookAt(GEO::Vector3(0,0,2), GEO::Vector3(0,0,0), GEO::Vector3(0,1,0));
            glLoadMatrixf(matView.ToGL());
            //glLoadIdentity();
            //gluLookAt(0, 0, 2, 0, 0, 0, 0, 1, 0);

            glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0); glVertex3f(std::sin(amt), std::cos(amt), 0);
            glColor3f(0, 1, 0); glVertex3f(std::sin(amt+1.05f), std::cos(amt+1.05f), 0);
            glColor3f(0, 0, 1); glVertex3f(std::sin(amt+2.1f), std::cos(amt+2.1f), 0);
            glEnd();
        }

        GFX::ShutdownWindow();

        if (screenSize.Width<=1 || screenSize.Height<=1)
            ok=false;

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(GfxSetup);
#endif //MPMA_COMPILE_GEO

#ifdef DECLARE_TESTS_CODE
class GfxGLExtensions: public UnitTest
{
public:
    bool Run()
    {
        //start opengl
        GFX::GraphicsSetup setup;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        //check the multitexture extension (everyone should have this) and a nonexistant one
        bool extAvailable=GFX::IsExtensionAvailable("GL_ARB_multitexture");
        if (!extAvailable)
        {
            std::cout<<"IsExtensionAvailable says GL_ARB_multitexture isn't available.. that should be everywhere.."<<std::endl;
            GFX::ShutdownWindow();
            return false;
        }

        extAvailable=GFX::IsExtensionAvailable("FooBar_MadeUpName");
        if (extAvailable)
        {
            std::cout<<"IsExtensionAvailable says FooBar_MadeUpName is available."<<std::endl;
            GFX::ShutdownWindow();
            return false;
        }

        //now get a pointer to a multitexture extension function, and to a nonexistant one
        if (GFX::GetExtensionFunction("glMultiTexCoord2fvARB")==0)
        {
            std::cout<<"GetExtensionFunction returned 0 for glMultiTexCoord2fvARB."<<std::endl;
            GFX::ShutdownWindow();
            return false;
        }

        if (GFX::GetExtensionFunction("fooBarMadeUpFunctionName")!=0)
        {
            std::cout<<"GetExtensionFunction returned something for fooBarMadeUpFunctionName."<<std::endl;
            GFX::ShutdownWindow();
            return false;
        }

        GFX::ShutdownWindow();
        return true;
    }
};
#endif
DECLARE_TEST_CASE(GfxGLExtensions);

#ifdef MPMA_COMPILE_GEO
#ifdef DECLARE_TESTS_CODE
class FullscreenTest: public UnitTest
{
public:
    bool Run()
    {
        std::cout<<"Starting windowed."<<std::endl;

        GFX::GraphicsSetup setup;
        setup.FullScreen=false;
        setup.Width=650;
        int expectedWidth=setup.Width;
        
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        GFX::ScreenSize screenSize=GFX::GetCurrentScreenSize();
        
        GFX::SetupWindow(setup);

        const GFX::GraphicsSetup *tmp=GFX::GetWindowState();
        if (tmp!=0)
        {
            setup=*tmp;
            if (tmp->Width!=expectedWidth)
            {
                std::cout<<"Width is wrong.  Expected "<<expectedWidth<<" but got "<<tmp->Width<<std::endl;
                ok=false;
            }
        }
        
        bool didSwitch1=false;
        bool didSwitch2=false;
        bool didSwitch3=false;
        bool didSwitch4=false;
        
        //render a bit
        MPMA::Timer timer;
        for (double total=0; total<20; total+=timer.Step())
        {
            //do some default random stuff
            GFX::UpdateWindow();

            const GFX::GraphicsSetup *state=GFX::GetWindowState();
            if (state==0)
                break;

            if (total>4 && !didSwitch1)
            {
                std::cout<<"Switching to fullscreen."<<std::endl;
                didSwitch1=true;
                GFX::GraphicsSetup newState=*state;
                newState.FullScreen=true;
                GFX::SetupWindow(newState);
                
                expectedWidth=screenSize.Width;
            }
            else if (total>8 && !didSwitch2)
            {
                std::cout<<"Switching to windowed original size."<<std::endl;
                didSwitch2=true;
                GFX::GraphicsSetup newState=*state;
                newState.FullScreen=false;
                GFX::SetupWindow(newState, GFX::DEFAULT_SETUPWINDOW_FLAGS|GFX::LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE);
                
                expectedWidth=650;
            }
            else if (total>12 && !didSwitch3)
            {
                std::cout<<"Switching to fullscreen."<<std::endl;
                didSwitch3=true;
                GFX::GraphicsSetup newState=*state;
                newState.FullScreen=true;
                GFX::SetupWindow(newState);
                
                expectedWidth=screenSize.Width;
            }
            else if (total>16 && !didSwitch4)
            {
                std::cout<<"Switching to windowed specific size."<<std::endl;
                didSwitch4=true;
                GFX::GraphicsSetup newState=*state;
                newState.FullScreen=false;
                newState.Width=400;
                newState.Height=200;
                GFX::SetupWindow(newState, GFX::DEFAULT_SETUPWINDOW_FLAGS&~GFX::LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE);
                
                expectedWidth=400;
            }
            
            state=GFX::GetWindowState();
            if (state==0)
                break;
            
            if (state->Width!=expectedWidth && ok)
            {
                std::cout<<"At time "<<total<<", Width is wrong.  Expected "<<expectedWidth<<" but got "<<state->Width<<std::endl;
                ok=false;
            }
            
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

            glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0); glVertex3f(std::sin(amt), std::cos(amt), 0);
            glColor3f(0, 1, 0); glVertex3f(std::sin(amt+1.05f), std::cos(amt+1.05f), 0);
            glColor3f(0, 0, 1); glVertex3f(std::sin(amt+2.1f), std::cos(amt+2.1f), 0);
            glEnd();
        }

        GFX::ShutdownWindow();

        if (screenSize.Width<=1 || screenSize.Height<=1)
            ok=false;

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(FullscreenTest);
#endif //MPMA_COMPILE_GEO
