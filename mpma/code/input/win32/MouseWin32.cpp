//!\file MouseWin32.cpp Mouse input for windows.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Mouse.h"

#ifdef MPMA_COMPILE_INPUT

#include "../../gfxsetup/GFXSetup.h"
#include "../../base/win32/alt_windows.h"

#include <vector>
#include <algorithm>

//window hookups
namespace GFX_INTERNAL
{
    void AddEventCallback(bool (*callback)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
}

namespace INPUT_INTERNAL
{
    extern std::vector<uint8> currentMouseClicks;
    extern std::vector<uint8> previousMouseClicks;
    extern std::vector<uint8> newMouseclicks;

    extern INPUT::MOUSE::MousePixelPosition mousePos;
    extern std::vector<INPUT::MOUSE::MousePixelPosition> mouseTrail;

    extern int mouseWheelChange;
}
using namespace INPUT_INTERNAL;

namespace
{
    uint8 GetWindowsButtonConstant(UINT message, UINT wParam)
    {
        if (message==WM_LBUTTONDOWN || message==WM_LBUTTONUP) return INPUT::MOUSE::LEFT_BUTTON;
        if (message==WM_MBUTTONDOWN || message==WM_MBUTTONUP) return INPUT::MOUSE::MIDDLE_BUTTON;
        if (message==WM_RBUTTONDOWN || message==WM_RBUTTONUP) return INPUT::MOUSE::RIGHT_BUTTON;
        if (message==WM_XBUTTONDOWN || message==WM_XBUTTONUP)
        {
            int xbut=(uint16)(wParam>>16);
            if (xbut==XBUTTON1) return INPUT::MOUSE::EXTENDED_BUTTON0;
            if (xbut==XBUTTON2) return INPUT::MOUSE::EXTENDED_BUTTON1;
        }

        return 0;
    }

    //called with events sent to the window
    bool WindowEventCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (!GFX::WindowHasFocus())
            return false;

        if (message==WM_MOUSEMOVE)
        {
            const GFX::GraphicsSetup* gfx=GFX::GetWindowState();
            if (gfx)
            {
                mousePos.X=(sint16)lParam;
                mousePos.Y=(sint16)(gfx->Height-((uint16)(lParam>>16)));
                mouseTrail.push_back(mousePos);
                return true;
            }
        }
        else if (message==WM_LBUTTONDOWN || message==WM_MBUTTONDOWN || message==WM_RBUTTONDOWN || message==WM_XBUTTONDOWN) //button was pressed
        {
            //capture mouse so we get input even if it leaves the window
            SetCapture(hwnd);

            //handle buttons
            uint8 butVal=GetWindowsButtonConstant(message, (UINT)wParam);
            if (butVal>=0 && butVal<13)
            {
                //count the position that this happened also
                const GFX::GraphicsSetup* gfx=GFX::GetWindowState();
                if (gfx)
                {
                    mousePos.X=(sint16)lParam;
                    mousePos.Y=(sint16)(gfx->Height-((uint16)(lParam>>16)));
                    mouseTrail.push_back(mousePos);
                }

                //handle the click
                bool isInCur=std::find(currentMouseClicks.begin(), currentMouseClicks.end(), butVal)!=currentMouseClicks.end();
                bool isInPrev=std::find(previousMouseClicks.begin(), previousMouseClicks.end(), butVal)!=previousMouseClicks.end();

                if (!isInCur)
                {
                    if (!isInPrev)
                    {
                        if (std::find(newMouseclicks.begin(), newMouseclicks.end(), butVal)==newMouseclicks.end())
                            newMouseclicks.push_back(butVal);
                    }

                    currentMouseClicks.push_back(butVal);
                }

                return true;
            }
        }
        else if (message==WM_LBUTTONUP || message==WM_MBUTTONUP || message==WM_RBUTTONUP || message==WM_XBUTTONUP) //button was released
        {
            //release mouse
            ReleaseCapture();

            //count the position that this happened also
            const GFX::GraphicsSetup* gfx=GFX::GetWindowState();
            if (gfx)
            {
                mousePos.X=(sint16)lParam;
                mousePos.Y=(sint16)(gfx->Height-((uint16)(lParam>>16)));
                mouseTrail.push_back(mousePos);
            }

            //handle the unclick
            uint8 butVal=GetWindowsButtonConstant(message, (UINT)wParam);
            if (butVal>=0 && butVal<13)
            {
                std::vector<uint8>::iterator c=std::find(currentMouseClicks.begin(), currentMouseClicks.end(), butVal);
                if (c!=currentMouseClicks.end())
                    currentMouseClicks.erase(c);

                return true;
            }
        }
        else if (message==WM_MOUSEWHEEL)
        {
            //direction accumulation
            int dir=-((sint16)(wParam>>16))/120;
            mouseWheelChange+=dir;

            //also as a button
            uint8 butVal=0;
            if (dir<0)
            {
                butVal=INPUT::MOUSE::WHEEL_UP;
            }
            else if (dir>0)
            {
                butVal=INPUT::MOUSE::WHEEL_DOWN;
            }

            if (butVal)
            {
                bool isInCur=std::find(currentMouseClicks.begin(), currentMouseClicks.end(), butVal)!=currentMouseClicks.end();
                bool isInPrev=std::find(previousMouseClicks.begin(), previousMouseClicks.end(), butVal)!=previousMouseClicks.end();

                if (!isInCur)
                {
                    if (!isInPrev)
                    {
                        if (std::find(newMouseclicks.begin(), newMouseclicks.end(), butVal)==newMouseclicks.end())
                            newMouseclicks.push_back(butVal);
                    }

                    currentMouseClicks.push_back(butVal);
                }
            }

            return true;
        }

        return false;
    }

    class AutoSetup
    {
    public:

        AutoSetup()
        {
            //hookup callbacks
            GFX_INTERNAL::AddEventCallback(WindowEventCallback);
        }
    } autoSetup;
}

bool mpmaForceReferenceToMousePlatformSpecificCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_INPUT
