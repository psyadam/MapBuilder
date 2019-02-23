//!\file Keyboard.cpp Keyboard input platform-common code.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "Keyboard.h"

#ifdef MPMA_COMPILE_INPUT

#include "../Setup.h"
#include "../base/DebugRouter.h"
#include "../base/Memory.h"
#include "../gfxsetup/GFXSetup.h"

#include <map>
#include <vector>
#include <algorithm>

//window hookups
namespace GFX_INTERNAL
{
    void AddUpdateCallback(void (*callback)());
    void AddAfterUpdateCallback(void (*callback)());
}
using namespace GFX_INTERNAL;

namespace INPUT_INTERNAL
{
    //current state
    std::vector<uint8> accumulatedInput;
    std::vector<uint8> currentKeys;
    std::vector<uint8> previousKeys;
    std::vector<uint8> newlyPressedKeys;
    bool isCapturingTypedText=true;
    bool isCapturingKeyState=true;

    void AfterKeyboardWindowUpdate(); //called after keyboard has updated, after the state has been cleared and updated
}
using namespace INPUT_INTERNAL;

//shared keyboard stuff
namespace
{
    //key list
    const std::string unknownText="??? Unknown ???";
    std::map<uint8, std::string> *validKeyList=0;
    std::map<std::string, uint8> *keyIdToKeyValue=0;

    std::vector<uint8> emptyKeyList;

    //init stuff
    void KeyboardInitialize()
    {
        using namespace INPUT::KEYBOARD;
        typedef std::map<uint8, std::string> greatMapType1;
        validKeyList=new3(greatMapType1());
        typedef std::map<std::string, uint8> greatMapType2;
        keyIdToKeyValue=new3(greatMapType2());

        //populate the valid key list
        validKeyList->insert(std::pair<uint8, std::string>(A, "A"));
        validKeyList->insert(std::pair<uint8, std::string>(B, "B"));
        validKeyList->insert(std::pair<uint8, std::string>(C, "C"));
        validKeyList->insert(std::pair<uint8, std::string>(D, "D"));
        validKeyList->insert(std::pair<uint8, std::string>(E, "E"));
        validKeyList->insert(std::pair<uint8, std::string>(F, "F"));
        validKeyList->insert(std::pair<uint8, std::string>(G, "G"));
        validKeyList->insert(std::pair<uint8, std::string>(H, "H"));
        validKeyList->insert(std::pair<uint8, std::string>(I, "I"));
        validKeyList->insert(std::pair<uint8, std::string>(J, "J"));
        validKeyList->insert(std::pair<uint8, std::string>(K, "K"));
        validKeyList->insert(std::pair<uint8, std::string>(L, "L"));
        validKeyList->insert(std::pair<uint8, std::string>(M, "M"));
        validKeyList->insert(std::pair<uint8, std::string>(N, "N"));
        validKeyList->insert(std::pair<uint8, std::string>(O, "O"));
        validKeyList->insert(std::pair<uint8, std::string>(P, "P"));
        validKeyList->insert(std::pair<uint8, std::string>(Q, "Q"));
        validKeyList->insert(std::pair<uint8, std::string>(R, "R"));
        validKeyList->insert(std::pair<uint8, std::string>(S, "S"));
        validKeyList->insert(std::pair<uint8, std::string>(T, "T"));
        validKeyList->insert(std::pair<uint8, std::string>(U, "U"));
        validKeyList->insert(std::pair<uint8, std::string>(V, "V"));
        validKeyList->insert(std::pair<uint8, std::string>(W, "W"));
        validKeyList->insert(std::pair<uint8, std::string>(X, "X"));
        validKeyList->insert(std::pair<uint8, std::string>(Y, "Y"));
        validKeyList->insert(std::pair<uint8, std::string>(Z, "Z"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM0, "0"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM1, "1"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM2, "2"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM3, "3"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM4, "4"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM5, "5"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM6, "6"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM7, "7"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM8, "8"));
        validKeyList->insert(std::pair<uint8, std::string>(NUM9, "9"));
        validKeyList->insert(std::pair<uint8, std::string>(SPACE, "Space"));
        validKeyList->insert(std::pair<uint8, std::string>(TILDA, "~"));
        validKeyList->insert(std::pair<uint8, std::string>(LEFT_BRACKET, "["));
        validKeyList->insert(std::pair<uint8, std::string>(RIGHT_BRACKET, "]"));
        validKeyList->insert(std::pair<uint8, std::string>(QUOTE, "'"));
        validKeyList->insert(std::pair<uint8, std::string>(COMMA, ", "));
        validKeyList->insert(std::pair<uint8, std::string>(PERIOD, "."));
        validKeyList->insert(std::pair<uint8, std::string>(SLASH, "/"));
        validKeyList->insert(std::pair<uint8, std::string>(BACKSLASH, "\\"));
        validKeyList->insert(std::pair<uint8, std::string>(EQUALS, "="));
        validKeyList->insert(std::pair<uint8, std::string>(DASH, "-"));
        validKeyList->insert(std::pair<uint8, std::string>(COLON, ";"));
        validKeyList->insert(std::pair<uint8, std::string>(ENTER, "Enter"));
        validKeyList->insert(std::pair<uint8, std::string>(TAB, "Tab"));
        validKeyList->insert(std::pair<uint8, std::string>(BACKSPACE, "Backspace"));
        validKeyList->insert(std::pair<uint8, std::string>(ESCAPE, "Escape"));
        validKeyList->insert(std::pair<uint8, std::string>(UP, "Up"));
        validKeyList->insert(std::pair<uint8, std::string>(DOWN, "Down"));
        validKeyList->insert(std::pair<uint8, std::string>(LEFT, "Left"));
        validKeyList->insert(std::pair<uint8, std::string>(RIGHT, "Right"));
        validKeyList->insert(std::pair<uint8, std::string>(INSERT, "Insert"));
        validKeyList->insert(std::pair<uint8, std::string>(DELETE, "Delete"));
        validKeyList->insert(std::pair<uint8, std::string>(HOME, "Home"));
        validKeyList->insert(std::pair<uint8, std::string>(END, "End"));
        validKeyList->insert(std::pair<uint8, std::string>(PAGE_UP, "Page Up"));
        validKeyList->insert(std::pair<uint8, std::string>(PAGE_DOWN, "Page Down"));
        validKeyList->insert(std::pair<uint8, std::string>(F1, "F1"));
        validKeyList->insert(std::pair<uint8, std::string>(F2, "F2"));
        validKeyList->insert(std::pair<uint8, std::string>(F3, "F3"));
        validKeyList->insert(std::pair<uint8, std::string>(F4, "F4"));
        validKeyList->insert(std::pair<uint8, std::string>(F5, "F5"));
        validKeyList->insert(std::pair<uint8, std::string>(F6, "F6"));
        validKeyList->insert(std::pair<uint8, std::string>(F7, "F7"));
        validKeyList->insert(std::pair<uint8, std::string>(F8, "F8"));
        validKeyList->insert(std::pair<uint8, std::string>(F9, "F9"));
        validKeyList->insert(std::pair<uint8, std::string>(F10, "F10"));
        validKeyList->insert(std::pair<uint8, std::string>(F11, "F11"));
        validKeyList->insert(std::pair<uint8, std::string>(F12, "F12"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD0, "Numpad 0"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD1, "Numpad 1"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD2, "Numpad 2"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD3, "Numpad 3"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD4, "Numpad 4"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD5, "Numpad 5"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD6, "Numpad 6"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD7, "Numpad 7"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD8, "Numpad 8"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD9, "Numpad 9"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD_SLASH, "Numpad /"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD_STAR, "Numpad *"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD_MINUS, "Numpad -"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD_PLUS, "Numpad +"));
        validKeyList->insert(std::pair<uint8, std::string>(PAD_DOT, "Numpad ."));
        validKeyList->insert(std::pair<uint8, std::string>(PAD_ENTER, "Numpad Enter"));
        validKeyList->insert(std::pair<uint8, std::string>(LEFT_SHIFT, "Left Shift"));
        validKeyList->insert(std::pair<uint8, std::string>(RIGHT_SHIFT, "Right Shift"));
        validKeyList->insert(std::pair<uint8, std::string>(LEFT_CONTROL, "Left Control"));
        validKeyList->insert(std::pair<uint8, std::string>(RIGHT_CONTROL, "Right Control"));
        validKeyList->insert(std::pair<uint8, std::string>(LEFT_ALT, "Left Alt"));
        validKeyList->insert(std::pair<uint8, std::string>(RIGHT_ALT, "Right Alt"));

        //populate the inverse mapping
        for (auto k=validKeyList->begin(); k!=validKeyList->end(); ++k)
        {
            keyIdToKeyValue->insert(std::pair<std::string, uint8>(k->second, k->first));
        }
    }

    void KeyboardShutdown()
    {
        delete3(validKeyList);
        validKeyList=0;
        delete3(keyIdToKeyValue);
        keyIdToKeyValue=0;
    }

    void WindowUpdateCallback()
    {
        //move cur to previous
        previousKeys.clear();
        previousKeys.insert(previousKeys.begin(), currentKeys.begin(), currentKeys.end());

        //clear frame's previous keys
        accumulatedInput.clear();
        newlyPressedKeys.clear();

        //if we lost focus, release all keys
        if (!GFX::WindowHasFocus())
            currentKeys.clear();

        //update
        AfterKeyboardWindowUpdate();
    }

    void AfterWindowUpdateCallback()
    {
#ifdef INPUT_ALT_ENTER_TOGGLES_FULLSCREEN
        if (std::find(currentKeys.begin(), currentKeys.end(), INPUT::KEYBOARD::LEFT_ALT)!=currentKeys.end() || std::find(currentKeys.begin(), currentKeys.end(), INPUT::KEYBOARD::RIGHT_ALT)!=currentKeys.end())
        {
            if (std::find(newlyPressedKeys.begin(), newlyPressedKeys.end(), INPUT::KEYBOARD::ENTER)!=newlyPressedKeys.end())
            {
                const GFX::GraphicsSetup *prevSetup=GFX::GetWindowState();
                if (prevSetup && prevSetup->Resizable)
                {
                    GFX::GraphicsSetup curSetup=*prevSetup;
                    curSetup.FullScreen=!curSetup.FullScreen;
                    GFX::SetupWindow(curSetup);
                }
            }
        }
#endif
    }

    class AutoInitKeyboard
    {
    public:
        //hookup init callbacks
        AutoInitKeyboard()
        {
            MPMA::Internal_AddInitCallback(KeyboardInitialize,8500);
            MPMA::Internal_AddShutdownCallback(KeyboardShutdown,8500);

            AddUpdateCallback(WindowUpdateCallback);
            AddAfterUpdateCallback(AfterWindowUpdateCallback);
        }
    } autoInitKeyboard;
}

namespace INPUT
{
    namespace KEYBOARD
    {

        //Returns the friendly name for a key value
        const std::string& GetFriendlyName(uint8 key)
        {
            std::map<uint8, std::string>::iterator i=validKeyList->find(key);
            if (i==validKeyList->end())
            {
                MPMA::ErrorReport()<<"Unknown value passed to Keyboard's GetFriendlyName: "+(int)key<<"\n";
                return unknownText;
            }
            else
                return i->second;
        }

        //For keyboard it's just the key name
        const std::string& GetPersistentIdentifier(uint8 key)
        {
            return GetFriendlyName(key);
        }

        //!Finds a key given a persistent identifier.  Returns 0 if not found;
        uint8 FindKey(const std::string &persistentIdentifier)
        {
            std::map<std::string, uint8>::iterator i=keyIdToKeyValue->find(persistentIdentifier);
            if (i==keyIdToKeyValue->end())
            {
                MPMA::ErrorReport()<<"Invalid value passed to Keyboard's FindKey: "<<persistentIdentifier<<"\n";
                return 0;
            }
            else
                return i->second;
        }

        //Returns whether the specified key is currently down
        bool IsKeyDown(uint8 key)
        {
            if (!isCapturingKeyState)
                return false;

            return std::find(currentKeys.begin(), currentKeys.end(), key)!=currentKeys.end();
        }

        //Returns whether the specified key was newly pressed within the last frame
        bool IsKeyNewlyPressed(uint8 key)
        {
            if (!isCapturingKeyState)
                return false;

            return std::find(newlyPressedKeys.begin(), newlyPressedKeys.end(), key)!=newlyPressedKeys.end();
        }

        //Returns a list of keys that are currently pressed down
        const std::vector<uint8>& GetCurrentlyPressedKeys()
        {
            if (!isCapturingKeyState)
                return emptyKeyList;

            return currentKeys;
        }

        //Returns a list of keys that were newly pressed within the last frame
        const std::vector<uint8>& GetNewlyPressedKeys()
        {
            if (!isCapturingKeyState)
                return emptyKeyList;

            return newlyPressedKeys;
        }

        //Updates a string with text typed by the user.  If the user had pressed backspace, the last character is removed.  Returns true if the text was changed.
        bool UpdateTypedText(std::string &textToEdit)
        {
            if (!isCapturingTypedText)
                return false;

            bool changed=false;
            for (std::vector<uint8>::const_iterator i=accumulatedInput.begin(); i!=accumulatedInput.end(); ++i)
            {
                if ((*i>=32 && *i<=127) || *i=='\n')
                {
                    textToEdit.push_back(*i);
                    changed=true;
                }
                else if (*i==8) //backspace
                {
                    if (!textToEdit.empty())
                    {
                        textToEdit.resize(textToEdit.size()-1);
                        changed=true;
                    }
                }
            }

            return changed;
        }

        //Sets the capture mode for keyboard input.
        void SetCaptureMode(bool captureTypedText, bool captureKeyState)
        {
            isCapturingTypedText=captureTypedText;
            isCapturingKeyState=captureKeyState;

            //clear the keys if we turned any sets off
            if (!isCapturingTypedText)
                accumulatedInput.clear();

            if (!isCapturingKeyState)
            {
                currentKeys.clear();
                newlyPressedKeys.clear();
            }
        }

        //Returns whether typed text is currently being captured
        bool IsCapturingText()
        {
            return isCapturingTypedText;
        }

        //Returns whether key state is currently being captured
        bool IsCapturingState()
        {
            return isCapturingKeyState;
        }

        void ClearState()
        {
            accumulatedInput.clear();
            currentKeys.clear();
            previousKeys.clear();
            newlyPressedKeys.clear();
        }
    }
}

bool mpmaForceReferenceToKeyboardCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_INPUT
