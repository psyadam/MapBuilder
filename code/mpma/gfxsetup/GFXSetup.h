//!\file GFXSetup.h Window system and OpenGL setup
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include "../base/Types.h"

#include <string>
#include <vector>

//!Graphics
namespace GFX
{
    //!Represents a the size of a window or screen.
    struct ScreenSize
    {
        //!Horizontal pixels contained within the window.
        int Width=0;

        //!Vertical pixels contained within the window.
        int Height=0;
    };

    struct WindowIcon
    {
        //!Width of the icon image.
        int Width=0;

        //!Height of the icon image.
        int Height=0;

        //!Pixels that make up the icon starting from the upper left corner.  There should be Width*Height entries stored in here.  Each pixel is 32-bit in ARGB format.  Note that not all platform support alpha in icons, so you should be careful to provide appropriate color information for any transparent pixels.
        std::vector<uint32> Pixels;

#if defined(_WIN32) || defined(_WIN64)
        //!If Pixels is empty and this is non-zero, this icon will be pulled from the app's resource section and used.  Only applies to Windows.
        int ResourceIconId=0;
#endif

    };

    //!Represents the state of graphics system and its related window.  Everything in here can be both changed or read from.
    struct GraphicsSetup: public ScreenSize
    {
        //!The name of the window.
        std::string Name="App";

        //!Whether the window takes over the whole screen.  If set to true, Width and Height will be adjusted to the current resolution and changes to Width and Height will have no effect.
        bool FullScreen=false;

        //!Whether the window can be resized by the user.  This is ignored if FullScreen is true.
        bool Resizable=true;

        //!Whether the window is currently minimized.
        bool Minimized=false;

        //TODO: stencil buffer and vsync options

        inline GraphicsSetup()
        {
            Width=800;
            Height=600;
        }
    };

    //Flags that can be passed to SetupWindow
    const uint32 LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE=1; //!<When leaving fullscreen mode, this specifies that the original window size prior to entering fullscreen mode should be used rather than the values stored in newDesiredState.
    
    //!Default flags for SetupWindow
    const uint32 DEFAULT_SETUPWINDOW_FLAGS=LEAVE_FULLSCREEN_USE_ORIGINAL_SIZE;
    
    //!Creates or updates the state of the applications window and sets up OpenGL.  Returns true if changing to the specified settings was successful, false if it failed.  This must be called at least once before OpenGL can be used.  Only the thread that called this to create the window may make OpenGL calls.
    bool SetupWindow(const GraphicsSetup &newDesiredState, uint32 setupWindowFlags=DEFAULT_SETUPWINDOW_FLAGS);

    //!Closes the window and shuts down the graphics system.  This must be called from the same thread that called SetupWindow to create the initial window.
    void ShutdownWindow();

    //!Retrieves the current state of the window, or 0 if the window has been closed.
    const GraphicsSetup* GetWindowState();

    //!Handles any window system events, and exposes the backbuffer.  You should normally call this every frame.  You should check GetWindowState after calling this to verify that the window still exists.  This must be called from the same thread that called SetupWindow to create the initial window.
    void UpdateWindow();

    //!Returns whether the window currently has the user's focus.
    bool WindowHasFocus();

    //!Returns the resolution of the current screen.  This may only called if a window created.
    ScreenSize GetCurrentScreenSize();

    //!Sets the icon for the window.
    void SetWindowIcon(const WindowIcon &icon);
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
