//!\file Unified.h Unified input for keyboard+mouse+gamepad.  Note that all input relies on the window being set up first.
//Luke Lenhart, 2008-2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_INPUT

#include "../base/Types.h"

#include <string>
#include <vector>

//!Input.
namespace INPUT
{
    //Note that all input relies on the window being set up first.

    //!The device that a button represents
    enum DeviceType
    {
        KEYBOARD_DEVICE=1,
        MOUSE_DEVICE,
        GAME_DEVICE
    };

    // -- buttons

    //!Represents an input button from an arbitrary device
    struct UnifiedButton
    {
        //!The source device of the button

        DeviceType Type;
        //Data specific to the device for this button.  The application should not try to interpret this value.
        union
        {
            nuint *DataPtr;
            nuint Data;
        };

        //ctor
        inline UnifiedButton(DeviceType type, nuint data)
        {
            Type=type;
            Data=data;
        }

        //ctor
        inline UnifiedButton(DeviceType type, nuint *dataPtr)
        {
            Type=type;
            DataPtr=dataPtr;
        }

        inline bool operator==(const UnifiedButton &o) const //!<==
                { return Type==o.Type && Data==o.Data; }
        inline bool operator!=(const UnifiedButton &o) const //!<!=
                { return !(*this==o); }

        //!Returns whether the button is currently pressed.
        bool IsPressed() const;

        //!Retrieves a user-friendly name for the button.
        const std::string& GetFriendlyName() const;

        //!Returns an string that can be used to uniquely identify the button again after the program has exited.
        const std::string GetPersistentIdentifier() const;
    };


    //!Returns a list of buttons that are currently pressed down
    const std::vector<UnifiedButton*>& GetCurrentlyPressedButtons();

    //!Returns a list of buttons that were pressed since the last frame
    const std::vector<UnifiedButton*>& GetNewlyPressedButtons();

    //!Finds a unified button from a persistent identifier returned from UnifiedButton::GetPersistentIdentifier.  Returns 0 if not found.
    UnifiedButton* FindUnifiedButton(const std::string &persistentIdendifier);

    namespace MOUSE
    {
        //!Returns a unified button that represents a specific mouse button
        UnifiedButton* GetUnifiedButton(uint8 button);
    }

    namespace KEYBOARD
    {
        //!Returns a unified button that represents a specific key
        UnifiedButton* GetUnifiedButton(uint8 key);
    }

    namespace GAME
    {
        class Button;

        //!Returns a unified button that represents a specific game device button
        UnifiedButton* GetUnifiedButton(Button *button);
    }

    // -- axes

    //!If set to true, letters on the keyboard commonly used for movement (asdfoe,) will be treated as another axis.  The default is false.
    extern bool KeyboardLettersActsAsAxis;

    //!If set to true, the numpad directions will be treated as another axis.  The default is true.
    extern bool KeyboardNumpadActsAsAxis;

    //!If set to true, the arrow directions will be treated as another axis.  The default is true.
    extern bool KeyboardArrowsActsAsAxis;

    //!Uniquely identifies an axis contraption.  Any given device type may have more than one set of axes.
    struct UnifiedAxisSet
    {
        //!The source device of the axis.
        DeviceType Type;

        //An identifier for the axis.  The application should not try to interpret this value.
        union
        {
            nuint *DataPtr;
            nuint Data;
        };

        //ctor
        inline UnifiedAxisSet(DeviceType type, nuint data)
        {
            Type=type;
            Data=data;
        }

        //ctor
        inline UnifiedAxisSet(DeviceType type, nuint *dataPtr)
        {
            Type=type;
            DataPtr=dataPtr;
        }

        inline bool operator==(const UnifiedButton &o) const //!<==
                { return Type==o.Type && Data==o.Data; }
        inline bool operator!=(const UnifiedButton &o) const //!<!=
                { return !(*this==o); }

        //!Returns the value of the X axis, from -1 to 1
        float GetXValue() const;

        //!Returns the value of the Y axis, from -1 to 1
        float GetYValue() const;

        //!Returns the value of the Z axis, from -1 to 1
        float GetZValue() const;

        //!Retrieves a user-friendly name for the button.
        const std::string GetFriendlyName() const;

        //!Returns an string that can be used to uniquely identify the axis set again after the program has exited.
        const std::string GetPersistentIdentifier() const;
    };

    namespace KEYBOARD
    {
        //!Returns a unified axis set for the keyboard directional arrows
        UnifiedAxisSet* GetUnifiedAxisSetArrows();

        //!Returns a unified axis set for the keyboard letters
        UnifiedAxisSet* GetUnifiedAxisSetLetters();

        //!Returns a unified axis set for the keyboard numpad
        UnifiedAxisSet* GetUnifiedAxisSetNumpad();
    }

    namespace GAME
    {
        class AxisSet;

        //!Returns a unified axis set that represents a specific game device axis set
        UnifiedAxisSet* GetUnifiedAxisSet(AxisSet *axes);
    }

    //!Returns a list of all axis sets that were pressed since the last frame.
    const std::vector<UnifiedAxisSet*> GetNewlyPressedAxes();

    //!Returns a list of all axis sets that are currently pressed.
    const std::vector<UnifiedAxisSet*> GetCurrentlyPressedAxes();

    //!Finds a unified button from a persistent identifier returned from UnifiedAxisSet::GetPersistentIdentifier.  Returns 0 if not found.
    UnifiedAxisSet* FindUnifiedAxisSet(const std::string &persistentIdendifier);

    //!Clears the state for buttons and keys for the current frame, for all device types.
    void ClearState();
}

#endif //#ifdef MPMA_COMPILE_INPUT
