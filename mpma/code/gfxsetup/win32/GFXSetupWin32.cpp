//!\file GFXSetupWin32.cpp Window system and OpenGL setup
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../GFXSetup.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include "../../Setup.h"
#include "../../base/DebugRouter.h"
#include <windows.h>
#include <string>
#include <GL/gl.h>
#include <GL/glu.h>
#include <set>

//internal event hookups to other parts that need window and update events
namespace GFX_INTERNAL
{
    std::list<void (*)()> *pUpdateCallbacks=0;
    std::list<void (*)()> *pAfterUpdateCallbacks=0;
    std::list<bool (*)(HWND, UINT, WPARAM, LPARAM)> *pEventCallbacks=0;
    std::list<void (*)(HWND)> *pWindowCreateCallbacks=0;
    std::list<void (*)(HWND)> *pWindowDestroyCallbacks=0;
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
    void AddEventCallback(bool (*callback)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam))
    {
        //we do not free this, it is a one time ever alloc
        if (pEventCallbacks==0)
            pEventCallbacks=new std::list<bool (*)(HWND, UINT, WPARAM, LPARAM)>();

        pEventCallbacks->push_back(callback);
    }

    //internal use only - add a callback which is called when the window is created
    void AddWindowCreateCallback(void (*callback)(HWND))
    {
        //we do not free this, it is a one time ever alloc
        if (pWindowCreateCallbacks==0)
            pWindowCreateCallbacks=new std::list<void (*)(HWND)>();

        pWindowCreateCallbacks->push_back(callback);
    }

    //internal use only - add a callback which is called when the window is destroyed
    void AddWindowDestroyCallback(void (*callback)(HWND))
    {
        //we do not free this, it is a one time ever alloc
        if (pWindowDestroyCallbacks==0)
            pWindowDestroyCallbacks=new std::list<void (*)(HWND)>();

        pWindowDestroyCallbacks->push_back(callback);
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

namespace
{
    bool isInited=false;
    bool isRequestedShutdown=false;
    bool isShutdownHooked=false;
    GFX::GraphicsSetup state;

    HINSTANCE hInstance=0;
    std::string windowClassName;
    int regClassAltCounter=0;
    HWND window=0;
    HICON hIcon=0;

    HDC windowDC=0;
    HGLRC renderContext=0;

    bool isActiveWindow=false;

    int borderOffsetX=0;
    int borderOffsetY=0;
    bool ignoreResizeUpdateMessage=false;
    RECT windowRectBeforeFullscreen={0, 0, 800, 600};

    LRESULT APIENTRY WindowEventHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    //Make the initial window
    bool CreateWindowPlease()
    {
        //get the module handle for us
        if (hInstance==0)
            hInstance=GetModuleHandle(0);

        if (hInstance==0)
        {
            MPMA::ErrorReport()<<"GetModuleHandle failed.\n";
            return false;
        }

        //register the class
        if (windowClassName.empty())
            windowClassName=std::string("MPMA_")+state.Name;

        WNDCLASS winClass;
        winClass.style=CS_CLASSDC;
        winClass.lpfnWndProc=(WNDPROC)WindowEventHandler;
        winClass.cbClsExtra=0;
        winClass.cbWndExtra=0;
        winClass.hInstance=hInstance;
        winClass.hIcon=0;
        winClass.hCursor=LoadCursor(0, IDC_ARROW);
        winClass.hbrBackground=0;
        winClass.lpszMenuName=0;
        winClass.lpszClassName=windowClassName.c_str();

        if (!RegisterClass(&winClass))
        {
            MPMA::ErrorReport()<<"RegisterClass failed... trying an alternate name.  \n";
            windowClassName+="_"+MPMA::VaryString(++regClassAltCounter);
            winClass.lpszClassName=windowClassName.c_str();

            if (!RegisterClass(&winClass))
            {
                MPMA::ErrorReport()<<"RegisterClass failed: "<<(uint32_t)GetLastError()<<"\n";
                return false;
            }
        }

        //create the window
        window=CreateWindowEx(0, windowClassName.c_str(), state.Name.c_str(),
            WS_SYSMENU|WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT,
            state.Width, state.Height,
            0, 0, hInstance, 0);

        if (window==0)
        {
            MPMA::ErrorReport()<<"CreateWindowEx failed: "<<(uint32_t)GetLastError()<<"\n";
            return false;
        }

        //now show it
        ShowWindow(window, SW_SHOW);
        isActiveWindow=true;

        //we need a device context to use
        windowDC=GetDC(window);
        if (windowDC==0)
        {
            MPMA::ErrorReport()<<"GetDC failed.\n";
            return false;
        }

        int windowBPP=GetDeviceCaps(windowDC, BITSPIXEL);
        if (windowBPP!=32)
            MPMA::ErrorReport()<<"Display is not running in 32-bit color mode.  Setup may fail.\n";

        //Find a pixel format for OpenGL
        PIXELFORMATDESCRIPTOR pfd={0};
        pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion=1;
        pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
        pfd.iPixelType=PFD_TYPE_RGBA;
        pfd.cColorBits=32;
        pfd.cDepthBits=24;
        pfd.cStencilBits=8;
        pfd.iLayerType=PFD_MAIN_PLANE;

        unsigned int indPF=ChoosePixelFormat(windowDC, &pfd);
        if (indPF==0)
        {
            MPMA::ErrorReport()<<"Couldn't find a pixel format to use.\n";
            return false;
        }

        DescribePixelFormat(windowDC, indPF, sizeof(pfd), &pfd);

        //verify that the format that we got back is at least reasonable
        if (pfd.cColorBits<32)
            MPMA::ErrorReport()<<"Graphics Setup warning: Non-32 bit color mode is being used: "<<pfd.cColorBits<<" bits\n";

        if (pfd.cDepthBits<16)
            MPMA::ErrorReport()<<"Graphics Setup warning: Low quality depth buffer being used: "<<pfd.cDepthBits<<" bits\n";

        //set the chosen pixel format
        if (!SetPixelFormat(windowDC, indPF, &pfd))
        {
            MPMA::ErrorReport()<<"SetPixelFormat failed\n";
            return false;
        }

        //create the context and set it
        renderContext=wglCreateContext(windowDC);
        if (renderContext==0)
        {
            MPMA::ErrorReport()<<"wglCreateContext failed.\n";
            return false;
        }

        if (!wglMakeCurrent(windowDC, renderContext))
        {
            MPMA::ErrorReport()<<"wglMakeCurrent failed.\n";
            return false;
        }

        return true;
    }

    void SetIconImage(const GFX::WindowIcon &iconSource)
    {
        HICON hNewIcon=0;
        if (iconSource.Pixels.empty() && iconSource.ResourceIconId!=0)
        {
            hNewIcon=LoadIcon(hInstance, MAKEINTRESOURCE(iconSource.ResourceIconId));
            if (hNewIcon)
            {

            }
        }
        else
        {
            if (iconSource.Height<=0 || iconSource.Width<=0)
            {
                MPMA::ErrorReport()<<"Cannot set window icon to a zero pixel image.\n";
                return;
            }

            if (iconSource.Height*iconSource.Width!=(int)iconSource.Pixels.size())
            {
                MPMA::ErrorReport()<<"Icon image for window does not contain the correct number of pixels.\n";
                return;
            }

            //convert and store
            BITMAPV5HEADER bmi={0};
            bmi.bV5Size=sizeof(BITMAPV5HEADER);
            bmi.bV5Width=iconSource.Width;
            bmi.bV5Height=-iconSource.Height;
            bmi.bV5Planes=1;
            bmi.bV5BitCount=32;
            bmi.bV5Compression=BI_BITFIELDS;
            bmi.bV5AlphaMask=0xff000000;
            bmi.bV5RedMask=0x00ff0000;
            bmi.bV5GreenMask=0x0000ff00;
            bmi.bV5BlueMask=0x000000ff;

            HDC compatibleDC=CreateCompatibleDC(windowDC);
            uint32 *pixelBits=0;
            HBITMAP bitmap=CreateDIBSection(compatibleDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pixelBits, 0, 0);

            for (int i=0; i<iconSource.Width*iconSource.Height; ++i)
                pixelBits[i]=iconSource.Pixels[i];

            HBITMAP monoBitmap=CreateBitmap(iconSource.Width, iconSource.Height, 1, 1, nullptr);

            ICONINFO iconInfo={0};
            iconInfo.fIcon=true;
            iconInfo.hbmColor=bitmap;
            iconInfo.hbmMask=monoBitmap;

            hNewIcon=CreateIconIndirect(&iconInfo);

            //cleanup
            ReleaseDC(window, compatibleDC);
            DeleteObject(bitmap);
            DeleteObject(monoBitmap);
        }

        if (hNewIcon)
        {
            SetClassLongPtr(window, GCLP_HICON, (LONG_PTR)hNewIcon);
            SetClassLongPtr(window, GCLP_HICONSM, (LONG_PTR)hNewIcon);

            if (hIcon)
                DestroyIcon(hIcon);
            hIcon=hNewIcon;
        }
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
            if (!CreateWindowPlease())
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
            SetWindowText(window, newState.Name.c_str());
        }

        //handle display FullScreen change
        bool didSwitchToWindowed=false;
        if (state.FullScreen!=newState.FullScreen || firstTime)
        {
            if (newState.FullScreen)
            {
                GetWindowRect(window, &windowRectBeforeFullscreen);

                SetWindowLongPtr(window, GWL_EXSTYLE, (LONG_PTR)WS_EX_TOPMOST);
                SetWindowLongPtr(window, GWL_STYLE, (LONG_PTR)WS_POPUP);

                ShowWindow(window, SW_MAXIMIZE);
            }
            else if (!firstTime) //back to windowed
            {
                didSwitchToWindowed=true;

                SetWindowLongPtr(window, GWL_EXSTYLE, (LONG_PTR)0);
                SetWindowLongPtr(window, GWL_STYLE, (LONG_PTR)WS_TILEDWINDOW);

                ShowWindow(window, SW_RESTORE);

                SetWindowPos(window, HWND_TOP, windowRectBeforeFullscreen.left, windowRectBeforeFullscreen.top, windowRectBeforeFullscreen.right-windowRectBeforeFullscreen.left, windowRectBeforeFullscreen.bottom-windowRectBeforeFullscreen.top, 0);
            }

            //adjust our size to match the new reality
            if (newState.FullScreen || setupWindowFlags&LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE)
            {
                RECT clientRect;
                GetClientRect(window, &clientRect);
                newState.Width=clientRect.right-clientRect.left;
                newState.Height=clientRect.bottom-clientRect.top;
            }
        }

        //adjust resizability
        if (firstTime || state.Resizable!=newState.Resizable || state.FullScreen!=newState.FullScreen)
        {
            //change the window style
            LONG bitsToSet=0;
            LONG bitsToRemove=0;
            if (newState.FullScreen) //completely borderless
            {
                bitsToSet=WS_POPUP;
                bitsToRemove=WS_SYSMENU|WS_MINIMIZEBOX|WS_THICKFRAME|WS_MAXIMIZEBOX;
            }
            else if (!newState.Resizable) //fixed size
            {
                bitsToSet=WS_SYSMENU|WS_MINIMIZEBOX;
                bitsToRemove=WS_POPUP|WS_THICKFRAME|WS_MAXIMIZEBOX;
            }
            else //user controls size
            {
                bitsToSet=WS_SYSMENU|WS_MINIMIZEBOX|WS_THICKFRAME|WS_MAXIMIZEBOX;
                bitsToRemove=WS_POPUP;                
            }

            LONG styleCur=GetWindowLong(window, GWL_STYLE);
            LONG styleNew=styleCur|bitsToSet;
            styleNew&=~bitsToRemove;
            SetWindowLong(window, GWL_STYLE, styleNew);

            //force it to update
            SetWindowPos(window, window, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);
        }

        //adjust size (or reset it if we changed the style)
        if ((!newState.FullScreen && (firstTime || state.Width!=newState.Width || state.Height!=newState.Height || state.Resizable!=newState.Resizable || state.FullScreen!=newState.FullScreen)) || (didSwitchToWindowed && !(setupWindowFlags&LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE)))
        {
            ignoreResizeUpdateMessage=true;

            //get current placement
            WINDOWPLACEMENT wp={0};
            wp.length=sizeof(wp);
            GetWindowPlacement(window, &wp);

            //change it optimistically to the desired size
            wp.rcNormalPosition.right=wp.rcNormalPosition.left+newState.Width;
            wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+newState.Height;
            SetWindowPlacement(window, &wp);

            //windows borders and menu steal some of the space though, so see how big the inside window area actually became and compensate
            RECT clientRect={0};
            GetClientRect(window, &clientRect);

            borderOffsetX=newState.Width-(clientRect.right-clientRect.left);
            borderOffsetY=newState.Height-(clientRect.bottom-clientRect.top);
            wp.rcNormalPosition.right+=borderOffsetX;
            wp.rcNormalPosition.bottom+=borderOffsetY;
            SetWindowPlacement(window, &wp);
        }

        //adjust iconic state
        if ((firstTime && newState.Minimized) || (!firstTime && state.Minimized!=newState.Minimized))
        {
            //get current placement
            WINDOWPLACEMENT wp={0};
            wp.length=sizeof(wp);
            GetWindowPlacement(window, &wp);

            //change and set it
            if (newState.Minimized)
                wp.showCmd=SW_MINIMIZE;
            else
                wp.showCmd=SW_RESTORE;

            SetWindowPlacement(window, &wp);
        }

        //update once to flush out window's events
        state=newState;
        UpdateWindow();
        ignoreResizeUpdateMessage=false;

        //callbacks
        if (GFX_INTERNAL::pWindowCreateCallbacks)
        {
            for (auto callback=GFX_INTERNAL::pWindowCreateCallbacks->begin(); callback!=GFX_INTERNAL::pWindowCreateCallbacks->end(); ++callback)
            {
                (*callback)(window);
            }
        }

        return true;
    }

    //Closes the window and shuts down the graphics system.
    void ShutdownWindow()
    {
        //callbacks
        if (window)
        {
            if (GFX_INTERNAL::pWindowDestroyCallbacks)
            {
                for (auto callback=GFX_INTERNAL::pWindowDestroyCallbacks->begin(); callback!=GFX_INTERNAL::pWindowDestroyCallbacks->end(); ++callback)
                {
                    (*callback)(window);
                }
            }
        }

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

        //
        GFX_INTERNAL::ExtensionShutdown();

        if (renderContext!=0)
        {
            wglDeleteContext(renderContext);
            renderContext=0;
        }

        if (windowDC!=0)
        {
            ReleaseDC(window, windowDC);
            windowDC=0;
        }

        if (window!=0)
        {
            HWND tmp=window;
            window=0;
            DestroyWindow(tmp);
        }

        if (!windowClassName.empty())
        {
            if (!UnregisterClass(windowClassName.c_str(), hInstance))
                MPMA::ErrorReport()<<"UnregisterClass failed.\n";
            windowClassName.clear();
        }
        
        isActiveWindow=false;
        isInited=false;
        isRequestedShutdown=false;
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
            DEVMODE mode={0};
            mode.dmSize=sizeof(mode);
            EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode);

            size.Width=mode.dmPelsWidth;
            size.Height=mode.dmPelsHeight;
        }

        return size;
    }

    //Handles any window system events.  You should normally call this every frame.
    void UpdateWindow()
    {
        if (!isInited)
            return;

        //callback to everyone that we are about to update
        if (GFX_INTERNAL::pUpdateCallbacks)
        {
            for (std::list<void (*)()>::iterator i=GFX_INTERNAL::pUpdateCallbacks->begin(); i!=GFX_INTERNAL::pUpdateCallbacks->end(); ++i)
                (*i)();
        }

        //see if anything happened
        MSG msg={0};
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message!=WM_QUIT)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
                break;
        }

        //update backbuffer
        wglSwapLayerBuffers(windowDC, WGL_SWAP_MAIN_PLANE);

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

        return isActiveWindow && !state.Minimized;
    }

    void SetWindowIcon(const WindowIcon &icon)
    {
        SetIconImage(icon);
    }
}

namespace
{
    LRESULT CALLBACK WindowEventHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        bool handled=false;
        switch (message)
        {
        //happens on maximize/restore and user resize and some other times
        case WM_SIZE:
            if (!ignoreResizeUpdateMessage)
            {
                int wid=LOWORD(lParam);
                int hei=HIWORD(lParam);
                state.Width=wid; //-borderOffsetX;
                state.Height=hei; //-borderOffsetY;

                if (state.Width<1) state.Width=1;
                if (state.Height<1) state.Height=1;
            }            
            return 0;
        //switch to or away from the window
        case WM_ACTIVATE:
            if (LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam)==WA_CLICKACTIVE)
                isActiveWindow=true;
            else if (LOWORD(wParam)==WA_INACTIVE)
                isActiveWindow=false;
            else
                break;
            return 0;
        //tell them nothing needs painted anymore
        case WM_PAINT:
            ValidateRect(hwnd, 0);
            return 0;
        //system commands
        case WM_SYSCOMMAND:
            //block the ones that are annoying for games
            if (wParam==SC_CONTEXTHELP || wParam==SC_HOTKEY || wParam==SC_HSCROLL || wParam==SC_KEYMENU || wParam==SC_MONITORPOWER || wParam==SC_MOUSEMENU || wParam==SC_SCREENSAVE || wParam==SC_VSCROLL)
            {
                handled=true;
                break;
            }
            //minimize/restore changes we'll take note of, but not block
            if (wParam==SC_MINIMIZE)
                state.Minimized=true;
            if (wParam==SC_RESTORE)
                state.Minimized=false;
            break;
        //user is closing the app
        case WM_DESTROY:
            GFX::RequestShutdown();
            PostQuitMessage(0);
            return 0;
        }

        //callback to everyone that wants to see events
        if (GFX_INTERNAL::pEventCallbacks)
        {
            for (std::list<bool (*)(HWND, UINT, WPARAM, LPARAM)>::iterator i=GFX_INTERNAL::pEventCallbacks->begin(); i!=GFX_INTERNAL::pEventCallbacks->end(); ++i)
                handled=(*i)(hwnd, message, wParam, lParam) || handled;
        }

        if (handled)
            return 0;

        //use default action
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
