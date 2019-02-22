//!\file KeyboardWin32.cpp Keyboard input for windows.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Keyboard.h"

#ifdef MPMA_COMPILE_INPUT

#include "../../base/win32/alt_windows.h"
#include "../../gfxsetup/GFXSetup.h"

#include <vector>
#include <map>
#include <algorithm>

//window hookups
namespace GFX_INTERNAL
{
    void AddEventCallback(bool (*callback)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
}

namespace
{
    //maps a vk to our key code
    std::map<nuint, uint8> mapVKToKey;

    //list of "special keys" that we have to poll every frame
    std::vector<std::pair<nuint, uint8> > specialVKs;
}

namespace INPUT_INTERNAL
{
    extern std::vector<uint8> accumulatedInput;
    extern std::vector<uint8> currentKeys;
    extern std::vector<uint8> previousKeys;
    extern std::vector<uint8> newlyPressedKeys;
    extern bool isCapturingTypedText;
    extern bool isCapturingKeyState;

    //we update special keys here that windows messages don't work for
    void AfterKeyboardWindowUpdate()
    {
        //clear the special keys out of the current list
        for (std::vector<std::pair<nuint, uint8> >::const_iterator sk=specialVKs.begin(); sk!=specialVKs.end(); ++sk)
        {
            std::vector<uint8>::iterator c=std::find(currentKeys.begin(), currentKeys.end(), sk->second);
            if (c!=currentKeys.end())
                currentKeys.erase(c);
        }

        //capture the pressed key set for the special keys
        unsigned char keys[256];
        if (isCapturingKeyState && GetKeyboardState(keys))
        {
            for (std::vector<std::pair<nuint, uint8> >::const_iterator sk=specialVKs.begin(); sk!=specialVKs.end(); ++sk)
            {
                if (keys[sk->first]&0x80) //key is pressed down
                {
                    currentKeys.push_back(sk->second);

                    //if it wasn't previously pressed, then also add it to the newly pressed set
                    if (std::find(previousKeys.begin(), previousKeys.end(), sk->second)==previousKeys.end()) //not in previous set
                        newlyPressedKeys.push_back(sk->second);
                }
            }
        }
    }
}
using namespace INPUT_INTERNAL;

namespace
{
    //converts the wPamam+lParam to a virtual key that treats the numpad the same regardless of the numlock state
    nuint GetRealVirtualKey(UINT wParam, UINT lParam)
    {
        bool extended=(lParam&(1<<24))!=0;
        nuint vk=wParam;

        //handle special numpad cases
        if (vk>=VK_NUMPAD0 && vk<=VK_NUMPAD9) return vk;
        if (!extended)
        {
            if (vk==VK_UP) return VK_NUMPAD8;
            if (vk==VK_DOWN) return VK_NUMPAD2;
            if (vk==VK_LEFT) return VK_NUMPAD4;
            if (vk==VK_RIGHT) return VK_NUMPAD6;
            if (vk==VK_HOME) return VK_NUMPAD7;
            if (vk==VK_PRIOR) return VK_NUMPAD9;
            if (vk==VK_END) return VK_NUMPAD1;
            if (vk==VK_NEXT) return VK_NUMPAD3;
            if (vk==VK_INSERT) return VK_NUMPAD0;
            if (vk==VK_CLEAR) return VK_NUMPAD5;
            if (vk==VK_DELETE) return VK_DECIMAL;
        }
        if (extended && vk==VK_RETURN) return VK_SEPARATOR;
        
        //else it's a normal key
        return vk;
    }

    //called with events sent to the window
    bool WindowEventCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        hwnd;
        if (!GFX::WindowHasFocus())
            return false;

        if (message==WM_CHAR)
        {
            if (isCapturingTypedText)
            {
                if (wParam==VK_RETURN)
                    accumulatedInput.push_back((uint8)'\n');
                else
                    accumulatedInput.push_back((uint8)wParam);
                return true;
            }
        }
        else if (message==WM_KEYDOWN || message==WM_SYSKEYDOWN)
        {
            if (isCapturingKeyState)
            {
                nuint vkey=GetRealVirtualKey((UINT)wParam, (UINT)lParam);
                std::map<nuint, uint8>::iterator i=mapVKToKey.find(vkey);
                if (i!=mapVKToKey.end())
                {
                    uint8 key=i->second;

                    bool isInCur=std::find(currentKeys.begin(), currentKeys.end(), key)!=currentKeys.end();
                    bool isInPrev=std::find(previousKeys.begin(), previousKeys.end(), key)!=previousKeys.end();

                    if (!isInCur)
                    {
                        if (!isInPrev)
                        {
                            if (std::find(newlyPressedKeys.begin(), newlyPressedKeys.end(), key)==newlyPressedKeys.end())
                                newlyPressedKeys.push_back(key);
                        }

                        currentKeys.push_back(key);
                    }
                }
            }
        }
        else if (message==WM_KEYUP || message==WM_SYSKEYUP)
        {
            if (isCapturingKeyState)
            {
                nuint vkey=GetRealVirtualKey((UINT)wParam, (UINT)lParam);
                std::map<nuint, uint8>::iterator i=mapVKToKey.find(vkey);
                if (i!=mapVKToKey.end())
                {
                    uint8 key=i->second;

                    std::vector<uint8>::iterator c=std::find(currentKeys.begin(), currentKeys.end(), key);
                    if (c!=currentKeys.end())
                        currentKeys.erase(c);
                }
            }
        }

        return false;
    }

    class AutoSetupKeyboard
    {
    public:
        
        AutoSetupKeyboard()
        {
            //hookup callbacks
            GFX_INTERNAL::AddEventCallback(WindowEventCallback);

            //build VK to our key mapping
            using namespace INPUT::KEYBOARD;

            for (char c='A'; c<='Z'; ++c)
                mapVKToKey.insert(std::pair<nuint, uint8>(c, c));

            for (char n='0'; n<='9'; ++n)
                mapVKToKey.insert(std::pair<nuint, uint8>(n, n));
            
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_SPACE, SPACE));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_3, TILDA));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_4, LEFT_BRACKET));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_6, RIGHT_BRACKET));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_7, QUOTE));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_COMMA, COMMA));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_PERIOD, PERIOD));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_2, SLASH));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_5, BACKSLASH));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_PLUS, EQUALS));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_MINUS, DASH));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_OEM_1, COLON));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_RETURN, ENTER));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_TAB, TAB));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_BACK, BACKSPACE));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_ESCAPE, ESCAPE));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_UP, UP));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_DOWN, DOWN));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_LEFT, LEFT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_RIGHT, RIGHT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_INSERT, INSERT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_DELETE, DELETE));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_HOME, HOME));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_END, END));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_PRIOR, PAGE_UP));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_NEXT, PAGE_DOWN));

            for (uint8 f=0; f<12; ++f)
                mapVKToKey.insert(std::pair<nuint, uint8>(VK_F1+f, F1+f));

            for (uint8 n=0; n<10; ++n)
                mapVKToKey.insert(std::pair<nuint, uint8>(VK_NUMPAD0+n, PAD0+n));

            mapVKToKey.insert(std::pair<nuint, uint8>(VK_DIVIDE, PAD_SLASH));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_MULTIPLY, PAD_STAR));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_SUBTRACT, PAD_MINUS));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_ADD, PAD_PLUS));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_DECIMAL, PAD_DOT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_SEPARATOR, PAD_ENTER));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_LSHIFT, LEFT_SHIFT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_RSHIFT, RIGHT_SHIFT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_LCONTROL, LEFT_CONTROL));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_RCONTROL, RIGHT_CONTROL));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_LMENU, LEFT_ALT));
            mapVKToKey.insert(std::pair<nuint, uint8>(VK_RMENU, RIGHT_ALT));

            //build the list of "special keys" that need manually polled each frame
            specialVKs.push_back(std::pair<nuint, uint8>(VK_F10, mapVKToKey[VK_F10]));
            specialVKs.push_back(std::pair<nuint, uint8>(VK_LSHIFT, mapVKToKey[VK_LSHIFT]));
            specialVKs.push_back(std::pair<nuint, uint8>(VK_RSHIFT, mapVKToKey[VK_RSHIFT]));
            specialVKs.push_back(std::pair<nuint, uint8>(VK_LCONTROL, mapVKToKey[VK_LCONTROL]));
            specialVKs.push_back(std::pair<nuint, uint8>(VK_RCONTROL, mapVKToKey[VK_RCONTROL]));
            specialVKs.push_back(std::pair<nuint, uint8>(VK_LMENU, mapVKToKey[VK_LMENU]));
            specialVKs.push_back(std::pair<nuint, uint8>(VK_RMENU, mapVKToKey[VK_RMENU]));
        }
    } autoSetupKeyboard;
}

bool mpmaForceReferenceToKeyboardPlatformSpecificCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_INPUT
