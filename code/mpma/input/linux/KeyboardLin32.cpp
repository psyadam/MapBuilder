//!\file KeyboardLin32.cpp Keyboard input for linux.
//Luke Lenhart, 2008
//See /docs/License.txt for details on how this code may be used.

#include "../Keyboard.h"

#ifdef MPMA_COMPILE_INPUT

#include "../../gfxsetup/GFXSetup.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <vector>
#include <map>
#include <algorithm>


//window hookups
namespace GFX_INTERNAL
{
    void AddEventCallback(void (*callback)(XEvent*));
}

namespace INPUT_INTERNAL
{
    extern std::vector<uint8> accumulatedInput;
    extern std::vector<uint8> currentKeys;
    extern std::vector<uint8> previousKeys;
    extern std::vector<uint8> newlyPressedKeys;
    extern bool isCapturingTypedText;
    extern bool isCapturingKeyState;

    //we don't need this
    void AfterKeyboardWindowUpdate()
    {
    }
}
using namespace INPUT_INTERNAL;

namespace
{
    std::map<int, uint8> mapXSymToKey;

    //called with events sent to the window
    void WindowEventCallback(XEvent *event)
    {
        if (!GFX::WindowHasFocus())
            return;

        if (event->type==KeyPress) //key was pressed
        {
            XKeyEvent *ke=(XKeyEvent*)event;
            char convertBuff[33]={0};
            KeySym sym;
            XLookupString(ke, convertBuff, 32, &sym, 0);

            if (isCapturingTypedText)
            {
                for (char *c=convertBuff; c!=&convertBuff[32] && *c; ++c)
                    accumulatedInput.push_back(*c);
            }

            if (isCapturingKeyState || isCapturingTypedText)
            {
                //see if we have a mapping for this key
                std::map<int, uint8>::iterator i=mapXSymToKey.find(sym);
                if (i!=mapXSymToKey.end())
                {
                    uint8 key=i->second;

                    if (isCapturingKeyState)
                    {
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

                    if (isCapturingTypedText && (key==INPUT::KEYBOARD::ENTER || key==INPUT::KEYBOARD::PAD_ENTER))
                        accumulatedInput.push_back((uint8)'\n');
                }
            }
        }
        else if (event->type==KeyRelease) //key was released
        {
            if (isCapturingKeyState)
            {
                XKeyEvent *ke=(XKeyEvent*)event;
                char convertBuff[9]={0};
                KeySym sym;
                XLookupString(ke, convertBuff, 8, &sym, 0);

                std::map<int, uint8>::iterator i=mapXSymToKey.find(sym);
                if (i!=mapXSymToKey.end())
                {
                    uint8 key=i->second;

                    std::vector<uint8>::iterator c=std::find(currentKeys.begin(), currentKeys.end(), key);
                    if (c!=currentKeys.end())
                        currentKeys.erase(c);
                }
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

            //build x key to our key mapping
            using namespace INPUT::KEYBOARD;

            mapXSymToKey.insert(std::pair<int, uint8>(XK_A, A));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_a, A));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_B, B));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_b, B));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_C, C));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_c, C));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_D, D));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_d, D));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_E, E));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_e, E));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F, F));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_f, F));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_G, G));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_g, G));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_H, H));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_h, H));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_I, I));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_i, I));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_J, J));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_j, J));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_K, K));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_k, K));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_L, L));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_l, L));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_M, M));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_m, M));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_N, N));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_n, N));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_O, O));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_o, O));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_P, P));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_p, P));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Q, Q));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_q, Q));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_R, R));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_r, R));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_S, S));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_s, S));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_T, T));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_t, T));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_U, U));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_u, U));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_V, V));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_v, V));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_W, W));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_w, W));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_X, X));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_x, X));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Y, Y));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_y, Y));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Z, Z));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_z, Z));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_0, NUM0));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_parenright, NUM0));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_1, NUM1));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_exclam, NUM1));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_2, NUM2));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_at, NUM2));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_3, NUM3));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_numbersign, NUM3));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_4, NUM4));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_dollar, NUM4));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_5, NUM5));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_percent, NUM5));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_6, NUM6));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_asciicircum, NUM6)); //is this the right one??
            mapXSymToKey.insert(std::pair<int, uint8>(XK_7, NUM7));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_ampersand, NUM7));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_8, NUM8));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_asterisk, NUM8));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_9, NUM9));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_parenleft, NUM9));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_space, SPACE));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_grave, TILDA));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_asciitilde, TILDA));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_bracketleft, LEFT_BRACKET));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_braceleft, LEFT_BRACKET));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_bracketright, RIGHT_BRACKET));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_braceright, RIGHT_BRACKET));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_apostrophe, QUOTE));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_quotedbl, QUOTE));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_comma, COMMA));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_less, COMMA));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_period, PERIOD));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_greater, PERIOD));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_slash, SLASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_question, SLASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_backslash, BACKSLASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_bar, BACKSLASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_equal, EQUALS));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_plus, EQUALS));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_minus, DASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_underscore, DASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_colon, COLON));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_semicolon, COLON));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Return, ENTER));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Tab, TAB));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_BackSpace, BACKSPACE));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Escape, ESCAPE));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Up, UP));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Down, DOWN));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Left, LEFT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Right, RIGHT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Insert, INSERT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Delete, DELETE));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Home, HOME));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_End, END));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Page_Up, PAGE_UP));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Page_Down, PAGE_DOWN));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F1, F1));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F2, F2));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F3, F3));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F4, F4));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F5, F5));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F6, F6));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F7, F7));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F8, F8));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F9, F9));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F10, F10));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F11, F11));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_F12, F12));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_0, PAD0));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Insert, PAD0));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_1, PAD1));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_End, PAD1));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_2, PAD2));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Down, PAD2));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_3, PAD3));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Page_Down, PAD3));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_4, PAD4));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Left, PAD4));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_5, PAD5));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Space, PAD5));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_6, PAD6));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Right, PAD6));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_7, PAD7));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Home, PAD7));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_8, PAD8));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Up, PAD8));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_9, PAD9));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Up, PAD9));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Divide, PAD_SLASH));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Multiply, PAD_STAR));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Subtract, PAD_MINUS));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Add, PAD_PLUS));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Decimal, PAD_DOT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Delete, PAD_DOT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_KP_Enter, PAD_ENTER));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Shift_L, LEFT_SHIFT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Shift_R, RIGHT_SHIFT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Control_L, LEFT_CONTROL));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Control_R, RIGHT_CONTROL));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Alt_L, LEFT_ALT));
            mapXSymToKey.insert(std::pair<int, uint8>(XK_Alt_R, RIGHT_ALT));
        }
    } autoSetup;
}

bool mpmaForceReferenceToKeyboardPlatformSpecificCPP=false; //work around a problem using MPMA as a static library

#endif //#ifdef MPMA_COMPILE_INPUT
