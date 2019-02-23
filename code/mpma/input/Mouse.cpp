//!\file Mouse.cpp Mouse input platform-common code.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "Mouse.h"

#ifdef MPMA_COMPILE_INPUT

#include "../base/DebugRouter.h"
#include "../gfxsetup/GFXSetup.h"

#include <algorithm>

namespace GFX_INTERNAL
{
    void AddUpdateCallback(void (*callback)());
}
using namespace GFX_INTERNAL;

//state of mouse
namespace INPUT_INTERNAL
{
    std::vector<uint8> currentMouseClicks;
    std::vector<uint8> previousMouseClicks;
    std::vector<uint8> newMouseclicks;

    INPUT::MOUSE::MousePixelPosition mousePos;
    std::vector<INPUT::MOUSE::MousePixelPosition> mouseTrail;

    int mouseWheelChange=0;
};
using namespace INPUT_INTERNAL;

namespace
{
    std::vector<INPUT::MOUSE::MouseScaledPosition> mouseScaledTrail;

    const std::string buttonNames[16]=
    {
        "??? Unknown Mouse Button ???",
        "Left Mouse Button",
        "Right Mouse Button",
        "Middle Mouse Button",
        "Ext Mouse Button 0",
        "Ext Mouse Button 1",
        "Ext Mouse Button 2",
        "Ext Mouse Button 3",
        "Ext Mouse Button 4",
        "Ext Mouse Button 5",
        "Ext Mouse Button 6",
        "Ext Mouse Button 7",
        "Ext Mouse Button 8",
        "Ext Mouse Button 9",
        "Mouse Wheel Up",
        "Mouse Wheel Down"
        
    };
    const unsigned int buttonNamesMax=16;

    void WindowUpdateCallback()
    {
        //move cur to previous
        previousMouseClicks.clear();
        previousMouseClicks.insert(previousMouseClicks.begin(), currentMouseClicks.begin(), currentMouseClicks.end());

        //wheel is special, it is always removed from current and re-added per-frame as needed
        currentMouseClicks.resize(std::remove(currentMouseClicks.begin(), currentMouseClicks.end(), INPUT::MOUSE::WHEEL_UP)-currentMouseClicks.begin());
        currentMouseClicks.resize(std::remove(currentMouseClicks.begin(), currentMouseClicks.end(), INPUT::MOUSE::WHEEL_DOWN)-currentMouseClicks.begin());

        //clear frame's previous buttons and motion
        newMouseclicks.clear();
        mouseTrail.clear();
        mouseScaledTrail.clear();
        mouseWheelChange=0;

        //if we lost focus, release all keys
        if (!GFX::WindowHasFocus())
        {
            currentMouseClicks.clear();
        }
    }

    class AutoInitMouse
    {
    public:
        //hookup init callbacks
        AutoInitMouse()
        {
            AddUpdateCallback(WindowUpdateCallback);
        }
    } autoInitMouse;
}

namespace INPUT
{
    namespace MOUSE
    {
        //Returns the friendly name for a button value
        const std::string& GetFriendlyName(uint8 button)
        {
            if (button==0 || button>=buttonNamesMax)
            {
                MPMA::ErrorReport()<<"Invalid value passed to Mouse GetFriendlyName: "<<(int)button<<"\n";
                return buttonNames[0];
            }
            else
                return buttonNames[button];
        }

        //Same as friendly name for mouse
        const std::string& GetPersistentIdentifier(uint8 key)
        {
            return GetFriendlyName(key);
        }

        //Finds a mouse button given a persistent identifier.  Returns 0 if not found.
        uint8 FindButton(const std::string &persistentIdentifier)
        {
            for (unsigned int i=0; i<buttonNamesMax; ++i)
            {
                if (buttonNames[i]==persistentIdentifier)
                    return (uint8)i;
            }

            return 0;
        }

        //Returns whether the specified mouse button is currently down
        bool IsButtonDown(uint8 button)
        {
            return std::find(currentMouseClicks.begin(), currentMouseClicks.end(), button)!=currentMouseClicks.end();
        }

        //Returns whether the specified mouse button was just pressed within the last frame
        bool IsButtonNewlyPressed(uint8 button)
        {
            return std::find(newMouseclicks.begin(), newMouseclicks.end(), button)!=newMouseclicks.end();
        }

        //Returns a list of mouse buttons that are currently pressed down
        const std::vector<uint8>& GetCurrentlyPressedButtons()
        {
            return currentMouseClicks;
        }

        //Returns a list of mouse buttons that were newly pressed within the last frame
        const std::vector<uint8>& GetNewlyPressedButtons()
        {
            return newMouseclicks;
        }

        //Returns the most recent scroll direction of the mouse wheel. -1 for up, 0 for none, 1 for down.
        int GetWheelDirection()
        {
            return mouseWheelChange;
        }

        //Returns the location of the mouse within the window, where X and Y are a value between 0 and 1.  This function is only usable in absolute mode, and always returns 0s in relative mode.
        MouseScaledPosition GetScaledPosition()
        {
            MouseScaledPosition msp;
            const GFX::GraphicsSetup* gfx=GFX::GetWindowState();
            if (gfx)
            {
                msp.X=(float)mousePos.X/gfx->Width;
                msp.Y=(float)mousePos.Y/gfx->Height;
            }
            else
            {
                msp.X=0;
                msp.Y=0;
            }

            return msp;
        }

        //Same as GetScaledPosition(), except this returns a list of all known locations that the server passed over since the last frame.  This function is only valid in absolute mode, and returns an empty list in relative mode.
        const std::vector<MouseScaledPosition>& GetScaledTrail()
        {
            if (mouseScaledTrail.empty() && !mouseTrail.empty()) //if we need to build the scaled list
            {
                const GFX::GraphicsSetup* gfx=GFX::GetWindowState();
                if (gfx)
                {
                    for (std::vector<MousePixelPosition>::iterator i=mouseTrail.begin(); i!=mouseTrail.end(); ++i)
                    {
                        MouseScaledPosition msp;
                        msp.X=(float)i->X/gfx->Width;
                        msp.Y=(float)i->Y/gfx->Height;
                        mouseScaledTrail.emplace_back(std::move(msp));
                    }
                }
            }

            return mouseScaledTrail;
        }

        //Returns the location of the mouse within the window, measured in pixels.  In absolute mode this returns the location of the cursor relative to the origin.  In relative mode this returns the number of pixels in each direction that the mouse moved since last frame.
        MousePixelPosition GetPixelPosition()
        {
            return mousePos;
        }

        //Same as GetPixelPosition, except this returns a list of all known locations that the server passed over since the last frame.  This function is only valid in absolute mode, and returns an empty list in relative mode.
        const std::vector<MousePixelPosition>& GetPixelTrail()
        {
            return mouseTrail;
        }

        void ClearState()
        {
            currentMouseClicks.clear();
            previousMouseClicks.clear();
            newMouseclicks.clear();
            mouseTrail.clear();
            mouseScaledTrail.clear();
        }
    }
}

bool mpmaForceReferenceToMouseCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_INPUT
