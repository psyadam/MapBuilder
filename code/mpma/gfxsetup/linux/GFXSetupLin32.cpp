//!\file GFXSetupLin32.cpp Window system and OpenGL setup
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../../Config.h"
#include "../../Setup.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include "../GFXSetup.h"
#include "../../base/DebugRouter.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <list>
#include <vector>
#include <set>

//internal event hookups to other parts that need window and update events
namespace GFX_INTERNAL
{
    std::list<void (*)()> *pUpdateCallbacks=0;
    std::list<void (*)()> *pAfterUpdateCallbacks=0;
    std::list<void (*)(XEvent*)> *pEventCallbacks=0;
    std::list<void (*)()> *pShutdownCallbacks=0;

    //internal use only - used to hook up window update callbacks needed by other components
    void AddUpdateCallback(void (*callback)())
    {
        //we do not free this, it is a one time ever alloc
        if (pUpdateCallbacks==0)
            pUpdateCallbacks=new std::list<void (*)()>();

        pUpdateCallbacks->push_back(callback);
    }

    //internal use only - used to hook up window update callbacks needed by other components
    void AddAfterUpdateCallback(void (*callback)())
    {
        //we do not free this, it is a one time ever alloc
        if (pAfterUpdateCallbacks==0)
            pAfterUpdateCallbacks=new std::list<void (*)()>();

        pAfterUpdateCallbacks->push_back(callback);
    }

    //internal use only - used to hook up window event callbacks needed by other components
    void AddEventCallback(void (*callback)(XEvent*))
    {
        //we do not free this, it is a one time ever alloc
        if (pEventCallbacks==0)
            pEventCallbacks=new std::list<void (*)(XEvent*)>();

        pEventCallbacks->push_back(callback);
    }

    //internal use only - add a callback which is called when the window is about to be shut down
    void AddWindowShutdownCallback(void (*callback)())
    {
        //we do not free this, it is a one time ever alloc
        if (pShutdownCallbacks==0)
            pShutdownCallbacks=new std::list<void (*)()>();

        pShutdownCallbacks->push_back(callback);
    }

    //declared in Extensions.cpp, we call these when we create or shut down the opengl window
    void ExtensionInit();
    void ExtensionShutdown();
}
using namespace GFX_INTERNAL;

namespace
{
    bool isInited=false;
    bool isRequestedShutdown=false;
    bool isShutdownHooked=false;
    GFX::GraphicsSetup state;

    Display *display=0;
    XVisualInfo *visualInfo=0;
    Window window=0;
    bool ignoreSizeEvent=false;

    Atom atomDeleteWindow=None;
    Atom atomChangeState=None;
    Atom atomNetWM=None;
    Atom atomNetWMFullscreen=None;
    Atom atomIcon=None;

    GLXContext context=0;

    bool windowHasFocus=false;

    GFX::ScreenSize windowSizeBeforeFullscreen;
    struct MeowSillyInit
    {
        MeowSillyInit()
        {
            windowSizeBeforeFullscreen.Width=800;
            windowSizeBeforeFullscreen.Height=600;
        }
    } meowInit;

    //Make the initial window
    bool CreateWindow()
    {
        //set up X
        display=XOpenDisplay(0);
        if (display==0)
        {
            MPMA::ErrorReport()<<"XOpenDisplay failed.\n";
            return false;
        }

        Window rootWindow=XDefaultRootWindow(display);

        //pick the attributes we want
        GLint visualAttributes[]=
        {
            GLX_RGBA,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER,
            None
        };
        visualInfo=glXChooseVisual(display, 0, visualAttributes);
        if (visualInfo==0)
        {
            MPMA::ErrorReport()<<"glXChooseVisual failed.\n";
            return false;
        }

        //set up the window
        Colormap colorMap=XCreateColormap(display, rootWindow, visualInfo->visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap=colorMap;
        swa.event_mask=StructureNotifyMask|FocusChangeMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask;

        window=XCreateWindow(display, rootWindow, 0, 0, state.Width, state.Height, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWColormap|CWEventMask, &swa);

        if (window)
            XMapWindow(display, window);
        else
        {
            MPMA::ErrorReport()<<"XCreateWindow failed.\n";
            return false;
        }

        //set us up to recieve a notifications for certain window manager things
        std::vector<Atom> atoms;

        atomDeleteWindow=XInternAtom(display, "WM_DELETE_WINDOW", false);
        if (atomDeleteWindow!=None)
            atoms.push_back(atomDeleteWindow);
        else
            MPMA::ErrorReport()<<"Failed to register WM_DELETE_WINDOW Atom.  App may not close properly if the user closes the window.\n";

        atomChangeState=XInternAtom(display, "WM_CHANGE_STATE", false);
        if (atomChangeState!=None)
            atoms.push_back(atomChangeState);
        else
            MPMA::ErrorReport()<<"Failed to register WM_CHANGE_STATE Atom.  App may not react to minimize/restore correctly.\n";

        if (!atoms.empty())
            XSetWMProtocols(display, window, &atoms[0], atoms.size());

        //get atoms needed for fullscreen
        atomNetWM=XInternAtom(display, "_NET_WM_STATE", false);
        if (atomNetWM==None)
            MPMA::ErrorReport()<<"Failed to register _NET_WM_STATE Atom.  App may not be able to switch to fullscreen.\n";

        atomNetWMFullscreen=XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);
        if (atomNetWMFullscreen==None)
            MPMA::ErrorReport()<<"Failed to register _NET_WM_STATE_FULLSCREEN Atom.  App will not be able to switch to fullscreen.\n";

        atomIcon=XInternAtom(display, "_NET_WM_ICON", false);
        if (atomIcon==None)
            MPMA::ErrorReport()<<"Failed to register _NET_WM_ICON Atom.  App will not be able to change icons.\n";

        //set up OpenGL
        context=glXCreateContext(display, visualInfo, 0, GL_TRUE);
        if (context)
            glXMakeCurrent(display, window, context);
        else
        {
            MPMA::ErrorReport()<<"glXCreateContext failed.\n";
            return false;
        }

        windowHasFocus=true;

        return true;
    }

    //callback for framework is shutting down
    void GFXFrameworkShutdownCallback()
    {
        GFX::ShutdownWindow();
    }
}

namespace GFX
{
    //Creates or updates the state of the applications window and sets up OpenGL.  Returns true if changing to the specified settings was successful, false if it failed.  This must be called at least once before OpenGL can be used.
    bool SetupWindow(const GraphicsSetup &newDesiredState, uint32 setupWindowFlags)
    {
        GraphicsSetup newState=newDesiredState;

        //sanitize input
        //TODO:

        //initial setup
        bool firstTime=false;
        if (!isInited)
        {
            firstTime=true;

            //hook our setup
            if (!isShutdownHooked)
            {
                isShutdownHooked=true;
                MPMA::Internal_AddShutdownCallback(GFXFrameworkShutdownCallback, 8000);
            }

            //make the initial window
            state=newState;
            if (!CreateWindow())
            {
                ShutdownWindow();
                return false;
            }

            GFX_INTERNAL::ExtensionInit();

            isInited=true;
        }

        //adjust name
        if (firstTime || state.Name!=newState.Name)
        {
            XStoreName(display, window, newState.Name.c_str());
        }

        //handle display FullScreen change
        //Note that XSendEvent happens over time due to animations, so we need to figure out and keep track of the intended sizes ourself.
        bool didSwitchToWindowed=false;
        if ((state.FullScreen!=newState.FullScreen || firstTime) && atomNetWM!=None && atomNetWMFullscreen!=None)
        {
            if (newState.FullScreen)
            {
                windowSizeBeforeFullscreen.Width=newState.Width;
                windowSizeBeforeFullscreen.Height=newState.Height;

                XEvent xev={0};
                xev.type=ClientMessage;
                xev.xclient.window=window;
                xev.xclient.message_type=atomNetWM;
                xev.xclient.format=32;
                xev.xclient.data.l[0]=1;
                xev.xclient.data.l[1]=atomNetWMFullscreen;
                XSendEvent(display, DefaultRootWindow(display), false, SubstructureNotifyMask, &xev);

                ScreenSize fullSize=GetCurrentScreenSize();
                newState.Width=fullSize.Width;
                newState.Height=fullSize.Height;
            }
            else if (!firstTime) //back to windowed
            {
                didSwitchToWindowed=true;

                if (setupWindowFlags&LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE)
                {
                    newState.Width=windowSizeBeforeFullscreen.Width;
                    newState.Height=windowSizeBeforeFullscreen.Height;
                }

                XEvent xev={0};
                xev.type=ClientMessage;
                xev.xclient.window=window;
                xev.xclient.message_type=atomNetWM;
                xev.xclient.format=32;
                xev.xclient.data.l[0]=0;
                xev.xclient.data.l[1]=atomNetWMFullscreen;
                XSendEvent(display, DefaultRootWindow(display), false, SubstructureNotifyMask, &xev);
            }
        }

        //adjust size
        if ((!newState.FullScreen && (state.Width!=newState.Width || state.Height!=newState.Height)) || (didSwitchToWindowed && !(setupWindowFlags&LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE)))
        {
            XWindowChanges change;
            change.width=newState.Width;
            change.height=newState.Height;
            XConfigureWindow(display, window, CWWidth|CWHeight, &change);

            ignoreSizeEvent=true;
        }

        //adjust resizability
        if (!newState.FullScreen && (firstTime || state.Resizable!=newState.Resizable))
        {
            XSizeHints *xsh=XAllocSizeHints();
            if (!newState.Resizable || newState.FullScreen) //fixed size
            {
                xsh->min_width=newState.Width;
                xsh->min_height=newState.Height;
                xsh->max_width=newState.Width;
                xsh->max_height=newState.Height;
            }
            else //user controls size
            {
                xsh->min_width=1;
                xsh->min_height=1;
                xsh->max_width=8000;
                xsh->max_height=8000;
            }
            xsh->flags=PMinSize|PMaxSize;
            XSetWMNormalHints(display, window, xsh);
            XFree(xsh);
        }

        //adjust iconic state
        if (firstTime || state.Minimized!=newState.Minimized)
        {
            if (newState.Minimized)
                XIconifyWindow(display, window, DefaultScreen(display));
            else
                XRaiseWindow(display, window);
        }

        //flush the x event queue and update once
        state=newState;
        UpdateWindow();

        ignoreSizeEvent=false;
        return true;
    }

    //Closes the window and shuts down the graphics system.
    void ShutdownWindow()
    {
        if (isInited)
        {
            if (GFX_INTERNAL::pShutdownCallbacks)
            {
                for (auto callback=GFX_INTERNAL::pShutdownCallbacks->begin(); callback!=GFX_INTERNAL::pShutdownCallbacks->end(); ++callback)
                {
                    (*callback)();
                }
            }
        }

        GFX_INTERNAL::ExtensionShutdown();

        //clean up OpenGL
        if (context!=0)
        {
            glXMakeCurrent(display, None, 0);
            glXDestroyContext(display, context);
            context=0;
        }

        //clean up X
        if (window!=0)
        {
            XDestroyWindow(display, window);
            window=0;
        }

        if (display!=0)
        {
            XFlush(display);
            XCloseDisplay(display);
            display=0;
        }

        atomDeleteWindow=None;
        atomChangeState=None;
        atomNetWM=None;
        atomNetWMFullscreen=None;
        isInited=false;
        isRequestedShutdown=false;
        windowHasFocus=false;
    }

    void RequestShutdown()
    {
        isRequestedShutdown=true;
    }

    //Retrieves the current state of the window. (or null pointer if not running)
    const GraphicsSetup* GetWindowState()
    {
        if (isInited && !isRequestedShutdown)
            return &state;
        else
            return 0;
    }

    //Returns the resolution of the current screen.
    ScreenSize GetCurrentScreenSize()
    {
        ScreenSize size;
        size.Width=1;
        size.Height=1;

        if (isInited)
        {
            size.Width=XDisplayWidth(display, DefaultScreen(display));
            size.Height=XDisplayHeight(display, DefaultScreen(display));
        }

        return size;
    }

    //Handles any window system events.  You should normally call this every frame.
    void UpdateWindow()
    {
        if (!isInited)
            return;

        //callback to everyone that we are about to update
        if (pUpdateCallbacks)
        {
            for (std::list<void (*)()>::iterator i=pUpdateCallbacks->begin(); i!=pUpdateCallbacks->end(); ++i)
                (*i)();
        }

        //see if anything happened
        while (XPending(display))
        {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type==ClientMessage) //window manager stuff
            {
                if (event.xclient.format==32)
                {
                    //if (atomDeleteWindow!=None && event.xclient.message_type==atomDeleteWindow) //user asked us to close -- why does this line not work?
                    if (atomDeleteWindow!=None && (Atom)event.xclient.data.l[0]==atomDeleteWindow) //user asked us to close
                    {
                        RequestShutdown();
                        break; //we're shutdown, don't try to process anything more
                    }
                    else if (atomChangeState!=None && event.xclient.message_type==atomChangeState) //some sorta state change
                    //else if (atomChangeState!=None && (Atom)event.xclient.data.l[0]==atomChangeState) //some sorta state change
                    {
                        if (event.xclient.data.l[0]==IconicState)
                            state.Minimized=true;
                        else //if... what?
                            state.Minimized=false;
                    }
                }
            }
            else if (event.type==ConfigureNotify) //resize
            {
                if (!ignoreSizeEvent)
                {
                    state.Width=event.xconfigure.width;
                    state.Height=event.xconfigure.height;

                    if (state.Width<1) state.Width=1;
                    if (state.Height<1) state.Height=1;
                }
            }
            else if (event.type==FocusIn) //gain focus
                windowHasFocus=true;
            else if (event.type==FocusOut) //lost focus
                windowHasFocus=false;

            //callback to everyone that wants to see events
            if (pEventCallbacks)
            {
                for (std::list<void (*)(XEvent*)>::iterator i=pEventCallbacks->begin(); i!=pEventCallbacks->end(); ++i)
                    (*i)(&event);
            }
        }

        //update backbuffer
        if (isInited)
        {
            glXSwapBuffers(display, window);
        }

        //callback to everyone that we finished an update
        if (GFX_INTERNAL::pAfterUpdateCallbacks)
        {
            for (std::list<void (*)()>::iterator i=GFX_INTERNAL::pAfterUpdateCallbacks->begin(); i!=GFX_INTERNAL::pAfterUpdateCallbacks->end(); ++i)
                (*i)();
        }
    }

    //Returns whether the window currently has the user's focus.
    bool WindowHasFocus()
    {
        if (!isInited)
            return false;

        return windowHasFocus && !state.Minimized;
    }

    void SetWindowIcon(const WindowIcon &icon)
    {
        if (icon.Height<=0 || icon.Width<=0)
        {
            MPMA::ErrorReport()<<"Cannot set window icon to a zero pixel image.\n";
            return;
        }

        if (icon.Height*icon.Width!=(int)icon.Pixels.size())
        {
            MPMA::ErrorReport()<<"Icon image for window does not contain the correct number of pixels.\n";
            return;
        }

        if (atomIcon==None || !window)
            return;

        std::vector<long> propertyData;
        propertyData.reserve(icon.Width*icon.Height+2);
        propertyData.push_back(icon.Width);
        propertyData.push_back(icon.Height);
        for (int i=0; i<icon.Width*icon.Height; ++i)
            propertyData.push_back(icon.Pixels[i]);

        XChangeProperty(display, window, atomIcon, XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)propertyData.data(), propertyData.size());
    }
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
