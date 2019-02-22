//!\file MouseLin32.cpp Mouse input for linux.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Mouse.h"

#ifdef MPMA_COMPILE_INPUT

#include "../../gfxsetup/GFXSetup.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <vector>
#include <algorithm>

//window hookups
namespace GFX_INTERNAL
{
    void AddEventCallback(void (*callback)(XEvent*));
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
    uint8 GetXButtonConstant(nuint xbut)
    {
        if (xbut==Button1) return INPUT::MOUSE::LEFT_BUTTON;
        if (xbut==Button2) return INPUT::MOUSE::MIDDLE_BUTTON;
        if (xbut==Button3) return INPUT::MOUSE::RIGHT_BUTTON;
        if (xbut==8) return INPUT::MOUSE::EXTENDED_BUTTON0; //TODO: Why am I getting 8 and 9 here? Those aren't valid constants for button even.
        if (xbut==9) return INPUT::MOUSE::EXTENDED_BUTTON1;
        return 0;
    }

    //called with events sent to the window
    void WindowEventCallback(XEvent *event)
    {
        if (!GFX::WindowHasFocus())
            return;

        if (event->type==MotionNotify) //motion
        {
            const GFX::GraphicsSetup* gfx=GFX::GetWindowState();
            if (gfx)
            {
                XMotionEvent *me=(XMotionEvent*)event;
                mousePos.X=me->x;
                mousePos.Y=gfx->Height-me->y;
                mouseTrail.push_back(mousePos);
            }
        }
        else if (event->type==ButtonPress) //button was pressed
        {
            XButtonPressedEvent *be=(XButtonPressedEvent*)event;

            //check for wheel scroll (also treated as a normal button)
            uint8 butVal;
            if (be->button==Button5)
            {
                ++mouseWheelChange;
                butVal=INPUT::MOUSE::WHEEL_DOWN;
            }
            else if (be->button==Button4)
            {
                --mouseWheelChange;
                butVal=INPUT::MOUSE::WHEEL_UP;
            }
            else
                butVal=GetXButtonConstant(be->button);

            if (butVal>=0 && butVal<=15)
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
        }
        else if (event->type==ButtonRelease) //button was released
        {
            XButtonReleasedEvent *be=(XButtonReleasedEvent*)event;
            uint8 butVal=GetXButtonConstant(be->button);
            if (butVal>=0 && butVal<13)
            {
                std::vector<uint8>::iterator c=std::find(currentMouseClicks.begin(), currentMouseClicks.end(), butVal);
                if (c!=currentMouseClicks.end())
                    currentMouseClicks.erase(c);
            }
        }
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
