//!\file Mouse.h Mouse input.  Note that all input relies on the window being set up first.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_INPUT

#include "../base/Types.h"
#include "Unified.h"

#include <vector>

namespace INPUT
{
    //!Mouse-specific input.
    namespace MOUSE
    {
        const uint8 LEFT_BUTTON=1; //!<Left mouse button
        const uint8 RIGHT_BUTTON=2; //!<Right mouse button
        const uint8 MIDDLE_BUTTON=3; //!<Middle mouse button
        const uint8 EXTENDED_BUTTON0=4; //!<Extended mouse button 0
        const uint8 EXTENDED_BUTTON1=5; //!<Extended mouse button 1
        const uint8 EXTENDED_BUTTON2=6; //!<Extended mouse button 2
        const uint8 EXTENDED_BUTTON3=7; //!<Extended mouse button 3
        const uint8 EXTENDED_BUTTON4=8; //!<Extended mouse button 4
        const uint8 EXTENDED_BUTTON5=9; //!<Extended mouse button 5
        const uint8 EXTENDED_BUTTON6=10; //!<Extended mouse button 6
        const uint8 EXTENDED_BUTTON7=11; //!<Extended mouse button 7
        const uint8 EXTENDED_BUTTON8=12; //!<Extended mouse button 8
        const uint8 EXTENDED_BUTTON9=13; //!<Extended mouse button 9
        const uint8 WHEEL_UP=14; //!<Mouse wheel moved up
        const uint8 WHEEL_DOWN=15; //!<Mouse wheel moved down

        //!Returns the friendly name for a mouse button
        const std::string& GetFriendlyName(uint8 button);

        //!Returns an string that can be used to uniquely identify the mouse button again after the program has exited.
        const std::string& GetPersistentIdentifier(uint8 key);

        //!Finds a mouse button given a persistent identifier.  Returns 0 if not found.
        uint8 FindButton(const std::string &persistentIdentifier);

        //!Returns whether the specified mouse button is currently down
        bool IsButtonDown(uint8 button);

        //!Returns whether the specified mouse button was just pressed within the last frame
        bool IsButtonNewlyPressed(uint8 button);

        //!Returns a list of mouse buttons that are currently pressed down
        const std::vector<uint8>& GetCurrentlyPressedButtons();

        //!Returns a list of mouse buttons that were newly pressed within the last frame
        const std::vector<uint8>& GetNewlyPressedButtons();

        //!Returns the most recent scroll direction of the mouse wheel, counted in number of wheel ticks.  The value can be negative or positive, or 0 if no movement occured.
        int GetWheelDirection();

        //!In absolute mode, this represents the location of the mouse cursor, measured in pixels from the origin, which is the lower left corner of the window.  In relative mode this represents the number of pixels the mouse moved in each direction since the last frame.
        struct MousePixelPosition
        {
            sint16 X;
            sint16 Y;

            inline bool operator==(const MousePixelPosition &o) const //!<==
                { return X==o.X && Y==o.Y; }
            inline bool operator!=(const MousePixelPosition &o) const //!<!=
                { return !(*this==o); }
        };

        //!Represents the location of the mouse cursor, measured from the origin, which is the lower left corner of the window.  Both X and Y are a number between 0 and 1.  This is only used in absolute mode.
        struct MouseScaledPosition
        {
            float X;
            float Y;

            inline bool operator==(const MouseScaledPosition &o) const //!<==
                { return X==o.X && Y==o.Y; }
            inline bool operator!=(const MouseScaledPosition &o) const //!<!=
                { return !(*this==o); }
        };

        //!Returns the location of the mouse within the window, where X and Y are a value between 0 and 1.  This function is only usable in absolute mode, and always returns 0s in relative mode.
        MouseScaledPosition GetScaledPosition();

        //!Same as GetScaledPosition(), except this returns a list of all known locations that the server passed over since the last frame
        const std::vector<MouseScaledPosition>& GetScaledTrail();

        //!Returns the location of the mouse within the window, measured in pixels.  In absolute mode this returns the location of the cursor relative to the origin.  In relative mode this returns the number of pixels in each direction that the mouse moved since last frame.
        MousePixelPosition GetPixelPosition();

        //!Same as GetPixelPosition, except this returns a list of all known locations that the server passed over since the last frame.  This function is only valid in absolute mode, and returns an empty list in relative mode.
        const std::vector<MousePixelPosition>& GetPixelTrail();

        //!Clears the state for buttons and movement for the current frame.
        void ClearState();

        //TODO: Mouse mode (os control in absolute mode, or we capture and provide relative mode data) (we are always absolute for now)
        //TODO: Cursor image control
    }
}

#endif //#ifdef MPMA_COMPILE_INPUT
