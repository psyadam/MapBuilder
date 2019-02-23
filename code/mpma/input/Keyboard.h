//!\file Keyboard.h Keyboard input.  Note that all input relies on the window being set up first.
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
    //!Keyboard-specific input.
    namespace KEYBOARD
    {
        //normal typeables
        const uint8 A='A'; //!<A key
        const uint8 B='B'; //!<B key
        const uint8 C='C'; //!<C key
        const uint8 D='D'; //!<D key
        const uint8 E='E'; //!<E key
        const uint8 F='F'; //!<F key
        const uint8 G='G'; //!<G key
        const uint8 H='H'; //!<H key
        const uint8 I='I'; //!<I key
        const uint8 J='J'; //!<J key
        const uint8 K='K'; //!<K key
        const uint8 L='L'; //!<L key
        const uint8 M='M'; //!<M key
        const uint8 N='N'; //!<N key
        const uint8 O='O'; //!<O key
        const uint8 P='P'; //!<P key
        const uint8 Q='Q'; //!<Q key
        const uint8 R='R'; //!<R key
        const uint8 S='S'; //!<S key
        const uint8 T='T'; //!<T key
        const uint8 U='U'; //!<U key
        const uint8 V='V'; //!<V key
        const uint8 W='W'; //!<W key
        const uint8 X='X'; //!<X key
        const uint8 Y='Y'; //!<Y key
        const uint8 Z='Z'; //!<Z key
        const uint8 NUM0='0'; //!<0 key above the letters
        const uint8 NUM1='1'; //!<1 key above the letters
        const uint8 NUM2='2'; //!<2 key above the letters
        const uint8 NUM3='3'; //!<3 key above the letters
        const uint8 NUM4='4'; //!<4 key above the letters
        const uint8 NUM5='5'; //!<5 key above the letters
        const uint8 NUM6='6'; //!<6 key above the letters
        const uint8 NUM7='7'; //!<7 key above the letters
        const uint8 NUM8='8'; //!<8 key above the letters
        const uint8 NUM9='9'; //!<9 key above the letters
        const uint8 SPACE=' '; //!<space key
        const uint8 TILDA='`'; //!<~ key
        const uint8 LEFT_BRACKET='['; //!<[ key
        const uint8 RIGHT_BRACKET=']'; //!<] key
        const uint8 QUOTE='\''; //!<" key
        const uint8 COMMA=','; //!<, key
        const uint8 PERIOD='.'; //!<. key
        const uint8 SLASH='/'; //!</ key
        const uint8 BACKSLASH='\\'; //!<\ key
        const uint8 EQUALS='='; //!<= key
        const uint8 DASH='-'; //!<- key
        const uint8 COLON=';'; //!<; key

        //normal control keys
        const uint8 ENTER='\n'; //!<enter key
        const uint8 TAB='\t'; //!<tab key
        const uint8 BACKSPACE=130; //!<backspace key
        const uint8 ESCAPE=131; //!<escape key
        const uint8 UP=140; //!<up arrow key
        const uint8 DOWN=141; //!<down arrow key
        const uint8 LEFT=142; //!<left arrow key
        const uint8 RIGHT=143; //!<right arrow key
        const uint8 INSERT=150; //!<insert key
        const uint8 DELETE=151; //!<delete key
        const uint8 HOME=152; //!<home key
        const uint8 END=153; //!<end key
        const uint8 PAGE_UP=154; //!<page up key
        const uint8 PAGE_DOWN=155; //!<page down key
        const uint8 F1=161; //!<F1 function key
        const uint8 F2=162; //!<F2 function key
        const uint8 F3=163; //!<F3 function key
        const uint8 F4=164; //!<F4 function key
        const uint8 F5=165; //!<F5 function key
        const uint8 F6=166; //!<F6 function key
        const uint8 F7=167; //!<F7 function key
        const uint8 F8=168; //!<F8 function key
        const uint8 F9=169; //!<F9 function key
        const uint8 F10=170; //!<F10 function key
        const uint8 F11=171; //!<F11 function key
        const uint8 F12=172; //!<F12 function key

        //numpad keys (not all keyboards have)
        const uint8 PAD0=200; //!<0 key on numpad
        const uint8 PAD1=201; //!<1 key on numpad
        const uint8 PAD2=202; //!<2 key on numpad
        const uint8 PAD3=203; //!<3 key on numpad
        const uint8 PAD4=204; //!<4 key on numpad
        const uint8 PAD5=205; //!<5 key on numpad
        const uint8 PAD6=206; //!<6 key on numpad
        const uint8 PAD7=207; //!<7 key on numpad
        const uint8 PAD8=208; //!<8 key on numpad
        const uint8 PAD9=209; //!<9 key on numpad
        const uint8 PAD_SLASH=210; //!</ key on numpad
        const uint8 PAD_STAR=211; //!<* key on numpad
        const uint8 PAD_MINUS=212; //!<- key on numpad
        const uint8 PAD_PLUS=213; //!<+ key on numpad
        const uint8 PAD_DOT=214; //!<. key on numpad
        const uint8 PAD_ENTER=215; //!<enter key on numpad

        //modifier keys
        const uint8 LEFT_SHIFT=230; //!<left shift key
        const uint8 RIGHT_SHIFT=231; //!<right shift key
        const uint8 LEFT_CONTROL=232; //!<left control key
        const uint8 RIGHT_CONTROL=233; //!<right control key
        const uint8 LEFT_ALT=234; //!<left alt key - BE WARY that the desktop system may already make use of this key for other purposes
        const uint8 RIGHT_ALT=235; //!<right alt key - BE WARY that the desktop system may already make use of this key for other purposes

        //!Returns the friendly name for a key value
        const std::string& GetFriendlyName(uint8 key);

        //!Returns an string that can be used to uniquely identify the key again after the program has exited.
        const std::string& GetPersistentIdentifier(uint8 key);

        //!Finds a key given a persistent identifier.  Returns 0 if not found.
        uint8 FindKey(const std::string &persistentIdentifier);

        //!Returns whether the specified key is currently down
        bool IsKeyDown(uint8 key);

        //!Returns whether the specified key was newly pressed within the last frame
        bool IsKeyNewlyPressed(uint8 key);

        //!Returns a list of keys that are currently pressed down
        const std::vector<uint8>& GetCurrentlyPressedKeys();

        //!Returns a list of keys that were newly pressed within the last frame
        const std::vector<uint8>& GetNewlyPressedKeys();

        //!Updates a string with text typed by the user.  If the user had pressed backspace, the last character is removed.  Enter is also stored in the string as \n.  Returns true if the text was changed.
        bool UpdateTypedText(std::string &textToEdit);

        //!Sets the capture mode for keyboard input.  This controls whether typed text or key state is captured and made available to the app.  By default both are captured.
        void SetCaptureMode(bool captureTypedText, bool captureKeyState);

        //!Returns whether typed text is currently being captured
        bool IsCapturingText();

        //!Returns whether key state is currently being captured
        bool IsCapturingState();

        //!Clears the state for all keys for the current frame.
        void ClearState();
    }
}

#endif //#ifdef MPMA_COMPILE_INPUT
