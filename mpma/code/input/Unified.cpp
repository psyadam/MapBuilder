//!\file Unified.cpp Unified input platform-common code.
//Luke Lenhart, 2008-2011
//See /docs/License.txt for details on how this code may be used.

#include "Unified.h"

#ifdef MPMA_COMPILE_INPUT

#include "Keyboard.h"
#include "Mouse.h"
#include "GameDevice.h"
#include "../Setup.h"
#include "../base/DebugRouter.h"
#include "../base/MiscStuff.h"
#include <cmath>
#include <map>

namespace GFX_INTERNAL
{
    void AddUpdateCallback(void (*callback)());
}
using namespace GFX_INTERNAL;

//locals
namespace
{
    const std::string badDevice="??? Unknown Device Type ???";
    const std::string badType="??? Bad Type ???";
    const std::string missingButton="(Missing Button)";
    const std::string missingAxis="(Missing Axis)";
    const std::string &keyboardAxisArrowsName="KeyboardArrows";
    const std::string &keyboardAxisLettersName="KeyboardLetters";
    const std::string &keyboardAxisNumpadName="KeyboardNumpad";
    
    const std::string PIDButtonKeyboardSeperator="UnifiedButton::Keyboard::";
    const std::string PIDButtonMouseSeperator="UnifiedButton::Mouse::";
    const std::string PIDButtonGameSeperator="UnifiedButton::Game::";
    const std::string PIDAxesKeyboardSeperator="UnifiedAxes::Keyboard::";
    const std::string PIDAxesGameSeperator="UnifiedAxes::Game::";

    const float DIAGONAL_VALUE=std::sqrt(2.0f)/2.0f;

    enum
    {
        KEYBOARD_ARROWS=0,
        KEYBOARD_LETTERS=1,
        KEYBOARD_NUMPAD=2
    };

    bool IsKeyUsedForAxis(uint8 key)
    {
        if (INPUT::KeyboardArrowsActsAsAxis && (key==INPUT::KEYBOARD::UP || key==INPUT::KEYBOARD::DOWN || key==INPUT::KEYBOARD::LEFT || key==INPUT::KEYBOARD::RIGHT))
            return true;

        if (INPUT::KeyboardLettersActsAsAxis && (key==INPUT::KEYBOARD::A || key==INPUT::KEYBOARD::O || key==INPUT::KEYBOARD::E || key==INPUT::KEYBOARD::COMMA || key==INPUT::KEYBOARD::S || key==INPUT::KEYBOARD::D || key==INPUT::KEYBOARD::W))
            return true;

        if (INPUT::KeyboardNumpadActsAsAxis && (key==INPUT::KEYBOARD::PAD8 || key==INPUT::KEYBOARD::PAD2 || key==INPUT::KEYBOARD::PAD4 || key==INPUT::KEYBOARD::PAD6))
            return true;

        return false;
    }

    int GetKeyboardXAxisState(nuint data)
    {
        if (data==KEYBOARD_ARROWS && INPUT::KeyboardArrowsActsAsAxis)
        {
            if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::LEFT))
                return -1;
            else if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::RIGHT))
                return 1;
        }
        else if (data==KEYBOARD_LETTERS && INPUT::KeyboardLettersActsAsAxis)
        {
            if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::A))
                return -1;
            else if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::E) || INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::D))
                return 1;
        }
        else if (data==KEYBOARD_NUMPAD && INPUT::KeyboardNumpadActsAsAxis)
        {
            if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::PAD4))
                return -1;
            else if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::PAD6))
                return 1;
        }

        return 0;
    }

    int GetKeyboardYAxisState(nuint data)
    {
        if (data==KEYBOARD_ARROWS && INPUT::KeyboardArrowsActsAsAxis)
        {
            if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::UP))
                return -1;
            else if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::DOWN))
                return 1;
        }
        else if (data==KEYBOARD_LETTERS && INPUT::KeyboardLettersActsAsAxis)
        {
            if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::COMMA) || INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::W))
                return -1;
            else if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::O) || INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::S))
                return 1;
        }
        else if (data==KEYBOARD_NUMPAD && INPUT::KeyboardNumpadActsAsAxis)
        {
            if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::PAD8))
                return -1;
            else if (INPUT::KEYBOARD::IsKeyDown(INPUT::KEYBOARD::PAD2))
                return 1;
        }

        return 0;
    }

    bool isCurrentlyPressedValid=false;
    std::vector<INPUT::UnifiedButton*> currentlyPressed;
    bool isNewlyPressedValid=false;
    std::vector<INPUT::UnifiedButton*> newlyPressed;

    //called when a frame update starts, wipe our button states
    void WindowUpdateCallback()
    {
        isCurrentlyPressedValid=false;
        isNewlyPressedValid=false;

        currentlyPressed.clear();
        newlyPressed.clear();
    }

    //cache to map real buttons and axes to unified versions
    std::map<nuint, INPUT::UnifiedButton> buttonCache;
    std::map<nuint, INPUT::UnifiedAxisSet> axisCache;

    nuint NinjaHash(INPUT::DeviceType type, nuint data)
    {
        int shiftBits=28;
        if (sizeof(nuint)!=4)
            shiftBits=60;
        nuint value=data^(type<<shiftBits);

        return value;
    }

    INPUT::UnifiedButton* GetButtonFromCache(INPUT::DeviceType type, nuint data)
    {
        nuint key=NinjaHash(type, data);
        std::map<nuint, INPUT::UnifiedButton>::iterator i=buttonCache.find(key);
        if (i!=buttonCache.end())
        {
            return &i->second;
        }
        else
        {
            INPUT::UnifiedButton newButton(type, data);
            buttonCache.insert(std::pair<nuint, INPUT::UnifiedButton>(key, newButton));
            i=buttonCache.find(key);
            return &i->second;
        }
    }

    INPUT::UnifiedButton* GetButtonFromCache(INPUT::DeviceType type, nuint *dataPtr)
    {
        return GetButtonFromCache(type, (nuint)dataPtr);
    }

    INPUT::UnifiedAxisSet* GetAxisFromCache(INPUT::DeviceType type, nuint data)
    {
        nuint key=NinjaHash(type, data);
        std::map<nuint, INPUT::UnifiedAxisSet>::iterator i=axisCache.find(key);
        if (i!=axisCache.end())
        {
            return &i->second;
        }
        else
        {
            INPUT::UnifiedAxisSet newAxes(type, data);
            axisCache.insert(std::pair<nuint, INPUT::UnifiedAxisSet>(key, newAxes));
            i=axisCache.find(key);
            return &i->second;
        }
    }

    INPUT::UnifiedAxisSet* GetAxisFromCache(INPUT::DeviceType type, nuint *dataPtr)
    {
        return GetAxisFromCache(type, (nuint)dataPtr);
    }

    void CleanupCache()
    {
        buttonCache.clear();
        axisCache.clear();
    }

    //hookup callbacks
    class AutoInitInput
    {
    public:
        AutoInitInput()
        {
            AddUpdateCallback(WindowUpdateCallback);
            MPMA::Internal_AddShutdownCallback(CleanupCache, 8700);
        }
    } autoInitInput;
}

namespace INPUT
{
    bool KeyboardLettersActsAsAxis=false;
    bool KeyboardNumpadActsAsAxis=true;
    bool KeyboardArrowsActsAsAxis=true;

    //Returns whether the button is currently pressed pressed
    bool UnifiedButton::IsPressed() const
    {
        if (Type==KEYBOARD_DEVICE)
            return KEYBOARD::IsKeyDown((uint8)Data);
        else if (Type==MOUSE_DEVICE)
            return MOUSE::IsButtonDown((uint8)Data);
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return ((INPUT::GAME::Button*)DataPtr)->IsPressed();
            else
#endif
                return false;
        }
        else
        {
            MPMA::ErrorReport()<<"Unknown device type in UnifiedButton::IsPressed.\n";
            return false;
        }
    }

    //Retrieves a user-friendly name for the button
    const std::string& UnifiedButton::GetFriendlyName() const
    {
        if (Type==KEYBOARD_DEVICE)
            return KEYBOARD::GetFriendlyName((uint8)Data);
        else if (Type==MOUSE_DEVICE)
            return MOUSE::GetFriendlyName((uint8)Data);
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return ((INPUT::GAME::Button*)DataPtr)->GetName();
            else
                return missingButton;
#else
            return badType;
#endif
        }
        else
        {
            MPMA::ErrorReport()<<"Unknown device type in UnifiedButton::GetFriendlyName.\n";
            return badDevice;
        }
    }

    const std::string UnifiedButton::GetPersistentIdentifier() const
    {
        if (Type==KEYBOARD_DEVICE)
            return PIDButtonKeyboardSeperator+KEYBOARD::GetPersistentIdentifier((uint8)Data);
        else if (Type==MOUSE_DEVICE)
            return PIDButtonMouseSeperator+MOUSE::GetPersistentIdentifier((uint8)Data);
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return PIDButtonGameSeperator+((INPUT::GAME::Button*)DataPtr)->GetPersistentIdentifier();
            else
#endif
                return badType;
        }
        else
        {
            MPMA::ErrorReport()<<"Unknown device type in UnifiedButton::GetPersistentIdentifier.\n";
            return badDevice;
        }
    }

    //Returns a list of buttons that are currently pressed down
    const std::vector<UnifiedButton*>& GetCurrentlyPressedButtons()
    {
        if (isCurrentlyPressedValid)
            return currentlyPressed;

        //rebuild the current list from all devices
        isCurrentlyPressedValid=true;
        currentlyPressed.clear();

        const std::vector<uint8> &kbKeys=INPUT::KEYBOARD::GetCurrentlyPressedKeys();
        for (std::vector<uint8>::const_iterator i=kbKeys.begin(); i!=kbKeys.end(); ++i)
        {
            if (!IsKeyUsedForAxis(*i))
                currentlyPressed.emplace_back(GetButtonFromCache(KEYBOARD_DEVICE, *i));
        }

        const std::vector<uint8> &mouseButtons=INPUT::MOUSE::GetCurrentlyPressedButtons();
        for (std::vector<uint8>::const_iterator i=mouseButtons.begin(); i!=mouseButtons.end(); ++i)
            currentlyPressed.emplace_back(GetButtonFromCache(MOUSE_DEVICE, *i));

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
        const std::vector<const INPUT::GAME::Button*>& gameButtons=INPUT::GAME::GetCurrentlyPressedButtons();
        for (auto i=gameButtons.begin(); i!=gameButtons.end(); ++i)
            currentlyPressed.emplace_back(GetButtonFromCache(GAME_DEVICE, (nuint*)*i));
#endif

        return currentlyPressed;
    }

    //Returns a list of buttons that were pressed since the last frame
    const std::vector<UnifiedButton*>& GetNewlyPressedButtons()
    {
        if (isNewlyPressedValid)
            return newlyPressed;

        //rebuild the current list from all devices
        isNewlyPressedValid=true;
        newlyPressed.clear();

        const std::vector<uint8> &kbKeys=INPUT::KEYBOARD::GetNewlyPressedKeys();
        for (std::vector<uint8>::const_iterator i=kbKeys.begin(); i!=kbKeys.end(); ++i)
        {
            if (!IsKeyUsedForAxis(*i))
                newlyPressed.emplace_back(GetButtonFromCache(KEYBOARD_DEVICE, *i));
        }

        const std::vector<uint8> &mouseButtons=INPUT::MOUSE::GetNewlyPressedButtons();
        for (std::vector<uint8>::const_iterator i=mouseButtons.begin(); i!=mouseButtons.end(); ++i)
            newlyPressed.emplace_back(GetButtonFromCache(MOUSE_DEVICE, *i));

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
        const std::vector<const INPUT::GAME::Button*>& gameButtons=INPUT::GAME::GetNewlyPressedButtons();
        for (auto i=gameButtons.begin(); i!=gameButtons.end(); ++i)
            newlyPressed.emplace_back(GetButtonFromCache(GAME_DEVICE, (nuint*)*i));
#endif

        return newlyPressed;
    }

    UnifiedButton* FindUnifiedButton(const std::string &persistentIdendifier)
    {
        if (MISC::StartsWith(persistentIdendifier, PIDButtonKeyboardSeperator))
        {
            std::string buttonId;
            buttonId.insert(buttonId.end(), persistentIdendifier.begin()+PIDButtonKeyboardSeperator.size(), persistentIdendifier.end());
            uint8 kbButton=INPUT::KEYBOARD::FindKey(buttonId);
            return GetButtonFromCache(KEYBOARD_DEVICE, kbButton);
        }
        else if (MISC::StartsWith(persistentIdendifier, PIDButtonMouseSeperator))
        {
            std::string buttonId;
            buttonId.insert(buttonId.end(), persistentIdendifier.begin()+PIDButtonMouseSeperator.size(), persistentIdendifier.end());
            uint8 mouseButton=INPUT::MOUSE::FindButton(buttonId);
            return GetButtonFromCache(MOUSE_DEVICE, mouseButton);
        }
        else if (MISC::StartsWith(persistentIdendifier, PIDButtonGameSeperator))
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            std::string buttonId;
            buttonId.insert(buttonId.end(), persistentIdendifier.begin()+PIDButtonGameSeperator.size(), persistentIdendifier.end());
            INPUT::GAME::Button *gameButton=INPUT::GAME::FindButton(buttonId);
            return GetButtonFromCache(GAME_DEVICE, (nuint*)gameButton);
#else
            return 0;
#endif
        }
        else
        {
            MPMA::ErrorReport()<<"Bad identifier passed to FindUnifiedButton.\n";
            return 0;
        }
    }

    //Returns the value of the X axis, from -1 to 1
    float UnifiedAxisSet::GetXValue() const
    {
        if (Type==KEYBOARD_DEVICE)
        {
            int xstate=GetKeyboardXAxisState(Data);
            int ystate=GetKeyboardYAxisState(Data);
            float modifier=1.0f;
            if (xstate && ystate)
                modifier=DIAGONAL_VALUE;

            return xstate*modifier;
        }
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return ((INPUT::GAME::AxisSet*)DataPtr)->GetXValue();
            else
#endif
                return 0;
        }
        else
            return 0;
    }

    //Returns the value of the Y axis, from -1 to 1
    float UnifiedAxisSet::GetYValue() const
    {
        if (Type==KEYBOARD_DEVICE)
        {
            int xstate=GetKeyboardXAxisState(Data);
            int ystate=GetKeyboardYAxisState(Data);
            float modifier=1.0f;
            if (xstate && ystate)
                modifier=DIAGONAL_VALUE;

            return ystate*modifier;
        }
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return ((INPUT::GAME::AxisSet*)DataPtr)->GetYValue();
            else
#endif
                return 0;
        }
        else
            return 0;
    }

    //Returns the value of the Z axis, from -1 to 1
    float UnifiedAxisSet::GetZValue() const
    {
        if (Type==KEYBOARD_DEVICE)
        {
            return 0;
        }
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return ((INPUT::GAME::AxisSet*)DataPtr)->GetZValue();
            else
#endif
                return 0;
        }
        else
            return 0;
    }

    //Retrieves a user-friendly name for the button.
    const std::string UnifiedAxisSet::GetFriendlyName() const
    {
        if (Type==KEYBOARD_DEVICE)
        {
            if (Data==KEYBOARD_ARROWS)
                return keyboardAxisArrowsName;
            else if (Data==KEYBOARD_LETTERS)
                return keyboardAxisLettersName;
            else if (Data==KEYBOARD_NUMPAD)
                return keyboardAxisNumpadName;
            else
                return badType;
        }
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return ((INPUT::GAME::AxisSet*)DataPtr)->GetCombinedName();
            else
                return missingAxis;
#else
            return badType;
#endif
        }
        else
        {
            MPMA::ErrorReport()<<"Unknown device type in UnifiedAxisSet::GetFriendlyName.\n";
            return badDevice;
        }
    }

    const std::string UnifiedAxisSet::GetPersistentIdentifier() const
    {
        if (Type==KEYBOARD_DEVICE)
        {
            if (Data==KEYBOARD_ARROWS)
                return PIDAxesKeyboardSeperator+keyboardAxisArrowsName;
            else if (Data==KEYBOARD_LETTERS)
                return PIDAxesKeyboardSeperator+keyboardAxisLettersName;
            else if (Data==KEYBOARD_NUMPAD)
                return PIDAxesKeyboardSeperator+keyboardAxisNumpadName;
            else
                return badType;
        }
        else if (Type==GAME_DEVICE)
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            if (DataPtr)
                return PIDAxesGameSeperator+((INPUT::GAME::AxisSet*)DataPtr)->GetPersistentIdentifier();
            else
#endif
                return badType;
        }
        else
        {
            MPMA::ErrorReport()<<"Unknown device type in UnifiedAxisSet::GetPersistentIdentifier.\n";
            return badDevice;
        }
    }

    //Returns a list of all axis sets that were pressed since the last frame.
    const std::vector<UnifiedAxisSet*> GetNewlyPressedAxes()
    {
        std::vector<UnifiedAxisSet*> axes;

        //add keyboard
        bool isKbArrowAxesPressed=false;
        bool isKbLettersAxesPressed=false;
        bool isKbNumpadAxesPressed=false;

        const std::vector<uint8> &kbKeys=INPUT::KEYBOARD::GetNewlyPressedKeys();
        for (auto kbk=kbKeys.begin(); kbk!=kbKeys.end(); ++kbk)
        {
            if (*kbk==INPUT::KEYBOARD::LEFT || *kbk==INPUT::KEYBOARD::RIGHT || *kbk==INPUT::KEYBOARD::UP || *kbk==INPUT::KEYBOARD::DOWN)
                isKbArrowAxesPressed=true;
            else if (*kbk==INPUT::KEYBOARD::A || *kbk==INPUT::KEYBOARD::O || *kbk==INPUT::KEYBOARD::E || *kbk==INPUT::KEYBOARD::COMMA || *kbk==INPUT::KEYBOARD::S || *kbk==INPUT::KEYBOARD::D || *kbk==INPUT::KEYBOARD::W)
                isKbLettersAxesPressed=true;
            else if (*kbk==INPUT::KEYBOARD::PAD4 || *kbk==INPUT::KEYBOARD::PAD6 || *kbk==INPUT::KEYBOARD::PAD8 || *kbk==INPUT::KEYBOARD::PAD2)
                isKbNumpadAxesPressed=true;
        }

        if (isKbArrowAxesPressed && KeyboardArrowsActsAsAxis)
            axes.emplace_back(GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_ARROWS));

        if (isKbLettersAxesPressed && KeyboardLettersActsAsAxis)
            axes.emplace_back(GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_LETTERS));

        if (isKbNumpadAxesPressed && KeyboardNumpadActsAsAxis)
            axes.emplace_back(GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_NUMPAD));

        //add game
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
        const std::vector<const INPUT::GAME::AxisSet*> &gpAxes=INPUT::GAME::GetNewlyPressedAxes();
        for (auto gpa=gpAxes.begin(); gpa!=gpAxes.end(); ++gpa)
            axes.emplace_back(GetAxisFromCache(GAME_DEVICE, (nuint*)*gpa));
#endif

        return axes;
    }

    //Returns a list of all axis sets that are currently pressed.
    const std::vector<UnifiedAxisSet*> GetCurrentlyPressedAxes()
    {
        std::vector<UnifiedAxisSet*> axes;

        //add keyboard
        bool isKbArrowAxesPressed=false;
        bool isKbLettersAxesPressed=false;
        bool isKbNumpadAxesPressed=false;

        const std::vector<uint8> &kbKeys=INPUT::KEYBOARD::GetCurrentlyPressedKeys();
        for (auto kbk=kbKeys.begin(); kbk!=kbKeys.end(); ++kbk)
        {
            if (*kbk==INPUT::KEYBOARD::LEFT || *kbk==INPUT::KEYBOARD::RIGHT || *kbk==INPUT::KEYBOARD::UP || *kbk==INPUT::KEYBOARD::DOWN)
                isKbArrowAxesPressed=true;
            else if (*kbk==INPUT::KEYBOARD::A || *kbk==INPUT::KEYBOARD::O || *kbk==INPUT::KEYBOARD::E || *kbk==INPUT::KEYBOARD::COMMA || *kbk==INPUT::KEYBOARD::S || *kbk==INPUT::KEYBOARD::D || *kbk==INPUT::KEYBOARD::W)
                isKbLettersAxesPressed=true;
            else if (*kbk==INPUT::KEYBOARD::PAD4 || *kbk==INPUT::KEYBOARD::PAD6 || *kbk==INPUT::KEYBOARD::PAD8 || *kbk==INPUT::KEYBOARD::PAD2)
                isKbNumpadAxesPressed=true;
        }

        if (isKbArrowAxesPressed && KeyboardArrowsActsAsAxis)
            axes.emplace_back(GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_ARROWS));

        if (isKbLettersAxesPressed && KeyboardLettersActsAsAxis)
            axes.emplace_back(GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_LETTERS));

        if (isKbNumpadAxesPressed && KeyboardNumpadActsAsAxis)
            axes.emplace_back(GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_NUMPAD));

        //add game
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
        const std::vector<const INPUT::GAME::AxisSet*> &gpAxes=INPUT::GAME::GetCurrentlyPressedAxes();
        for (auto gpa=gpAxes.begin(); gpa!=gpAxes.end(); ++gpa)
            axes.emplace_back(GetAxisFromCache(GAME_DEVICE, (nuint*)*gpa));
#endif

        return axes;
    }

    //Finds a unified button from a persistent identifier returned from UnifiedAxisSet::GetPersistentIdentifier.  Returns 0 if not found.
    UnifiedAxisSet* FindUnifiedAxisSet(const std::string &persistentIdendifier)
    {
        if (MISC::StartsWith(persistentIdendifier, PIDAxesKeyboardSeperator))
        {
            std::string axisId;
            axisId.insert(axisId.end(), persistentIdendifier.begin()+PIDAxesKeyboardSeperator.size(), persistentIdendifier.end());

            if (axisId==keyboardAxisArrowsName)
                return GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_ARROWS);
            else if (axisId==keyboardAxisLettersName)
                return GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_LETTERS);
            else if (axisId==keyboardAxisNumpadName)
                return GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_NUMPAD);
            else
            {
                MPMA::ErrorReport()<<"Bad keyboard identifier passed to FindUnifiedAxisSet.\n";
                return 0;
            }
        }
        else if (MISC::StartsWith(persistentIdendifier, PIDAxesGameSeperator))
        {
#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
            std::string axisId;
            axisId.insert(axisId.end(), persistentIdendifier.begin()+PIDAxesGameSeperator.size(), persistentIdendifier.end());
            INPUT::GAME::AxisSet *gameAxes=INPUT::GAME::FindAxisSet(axisId);
            return GetAxisFromCache(GAME_DEVICE, (nuint*)gameAxes);
#else
            return 0;
#endif
        }
        else
        {
            MPMA::ErrorReport()<<"Bad identifier passed to FindUnifiedAxisSet.\n";
            return 0;
        }
    }

    void ClearState()
    {
        KEYBOARD::ClearState();
        MOUSE::ClearState();
#if defined(ENABLE_GAME_DEVICE_INPUT)
        GAME::ClearState();
#endif
    }

    //GetUnified functions from Keyboard
    namespace KEYBOARD
    {
        UnifiedButton* GetUnifiedButton(uint8 key)
        {
            return GetButtonFromCache(KEYBOARD_DEVICE, key);
        }

        UnifiedAxisSet* GetUnifiedAxisSetArrows()
        {
            return GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_ARROWS);
        }

        UnifiedAxisSet* GetUnifiedAxisSetLetters()
        {
            return GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_LETTERS);
        }

        UnifiedAxisSet* GetUnifiedAxisSetNumpad()
        {
            return GetAxisFromCache(KEYBOARD_DEVICE, KEYBOARD_NUMPAD);
        }
    }


    //GetUnified functions from Mouse
    namespace MOUSE
    {
        UnifiedButton* GetUnifiedButton(uint8 button)
        {
            return GetButtonFromCache(MOUSE_DEVICE, button);
        }
    }

    //GetUnified functions from Game
    namespace GAME
    {
        UnifiedButton* GetUnifiedButton(Button *button)
        {
            return GetButtonFromCache(GAME_DEVICE, (nuint*)button);
        }

        UnifiedAxisSet* GetUnifiedAxisSet(AxisSet *axes)
        {
            return GetAxisFromCache(GAME_DEVICE, (nuint*)axes);
        }
    }
} //namespace INPUT

bool mpmaForceReferenceToUnifiedCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_INPUT
