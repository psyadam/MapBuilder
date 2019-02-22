//some common "things" that windows.h would normally declare that we need, but would rather not pull in windows.h which pollutes the namespace badly.

#pragma once

//some types and pseudo types

#ifndef APIENTRY
    #define APIENTRY __stdcall
#endif

#ifndef CALLBACK
    #define CALLBACK __stdcall
#endif

#ifndef WINAPI
    #define WINAPI __stdcall
#endif

#ifndef DECLSPEC_IMPORT
    #define DECLSPEC_IMPORT __declspec(dllimport)
#endif

#ifndef WINUSERAPI
    #define WINUSERAPI DECLSPEC_IMPORT
#endif

#ifndef WINGDIAPI
	#define WINGDIAPI DECLSPEC_IMPORT
#endif

#ifndef _WINDEF_
    struct HWND__ { int unused; }; typedef struct HWND__ *HWND;
#endif

#if defined(_WIN64)
    typedef __int64 INT_PTR, *PINT_PTR;
    typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
    typedef int INT_PTR, *PINT_PTR;
    typedef unsigned int UINT_PTR, *PUINT_PTR;

    typedef long LONG_PTR, *PLONG_PTR;
    typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#ifndef _WPARAM_DEFINED
    #define _WPARAM_DEFINED
    typedef UINT_PTR WPARAM;
#endif // _WPARAM_DEFINED

#ifndef _LPARAM_DEFINED
    #define _LPARAM_DEFINED
    typedef LONG_PTR LPARAM;
#endif // !_LPARAM_DEFINED

typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef BYTE *PBYTE;
typedef long LONG;

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef TRUE
    #define TRUE 1
#endif

//functions
extern "C" WINUSERAPI BOOL WINAPI GetKeyboardState(PBYTE lpKeyState);

extern "C" WINUSERAPI HWND WINAPI SetCapture(HWND hWnd);

extern "C" WINUSERAPI BOOL WINAPI ReleaseCapture();

//message constants
#ifndef WM_CHAR
    #define WM_CHAR 0x102
    #define WM_KEYDOWN 0x100
    #define WM_SYSKEYDOWN 0x0104
    #define WM_KEYUP 0x101
    #define WM_SYSKEYUP 0x0105
    #define WM_MOUSEMOVE 0x200
    #define WM_LBUTTONDOWN 0x201
    #define WM_LBUTTONUP 0x202
    #define WM_MBUTTONDOWN 0x207
    #define WM_MBUTTONUP 0x208
    #define WM_RBUTTONDOWN 0x204
    #define WM_RBUTTONUP 0x205
    #define WM_XBUTTONDOWN 0x20B
    #define WM_XBUTTONUP 0x20C
    #define WM_MOUSEWHEEL 0x20A
    #define WM_SYSCOMMAND 0x0112

    #define SC_KEYMENU 0xF100
#endif

//key constants
#ifndef VK_BACK
    #define VK_BACK 0x08
    #define VK_TAB 0x09
    #define VK_CLEAR 0x0C
    #define VK_RETURN 0x0D
    #define VK_ESCAPE 0x1B
    #define VK_SPACE 0x20
    #define VK_PRIOR 0x21
    #define VK_NEXT 0x22
    #define VK_END 0x23
    #define VK_HOME 0x24
    #define VK_LEFT 0x25
    #define VK_UP 0x26
    #define VK_RIGHT 0x27
    #define VK_DOWN 0x28
    #define VK_INSERT 0x2D
    #define VK_DELETE 0x2E
    #define VK_NUMPAD0 0x60
    #define VK_NUMPAD1 0x61
    #define VK_NUMPAD2 0x62
    #define VK_NUMPAD3 0x63 
    #define VK_NUMPAD4 0x64
    #define VK_NUMPAD5 0x65
    #define VK_NUMPAD6 0x66
    #define VK_NUMPAD7 0x67
    #define VK_NUMPAD8 0x68
    #define VK_NUMPAD9 0x69
    #define VK_MULTIPLY 0x6A
    #define VK_ADD 0x6B
    #define VK_SEPARATOR 0x6C
    #define VK_SUBTRACT 0x6D
    #define VK_DECIMAL 0x6E
    #define VK_DIVIDE 0x6F
    #define VK_F1 0x70
    #define VK_F2 0x71
    #define VK_F3 0x72
    #define VK_F4 0x73
    #define VK_F5 0x74
    #define VK_F6 0x75
    #define VK_F7 0x76
    #define VK_F8 0x77
    #define VK_F9 0x78
    #define VK_F10 0x79
    #define VK_F11 0x7A
    #define VK_F12 0x7B
    #define VK_LSHIFT 0xA0
    #define VK_RSHIFT 0xA1
    #define VK_LCONTROL 0xA2
    #define VK_RCONTROL 0xA3
    #define VK_LMENU 0xA4
    #define VK_RMENU 0xA5
    #define VK_OEM_PLUS 0xBB
    #define VK_OEM_COMMA 0xBC
    #define VK_OEM_MINUS 0xBD
    #define VK_OEM_PERIOD 0xBE
    #define VK_OEM_1 0xBA
    #define VK_OEM_2 0xBF
    #define VK_OEM_3 0xC0
    #define VK_OEM_4 0xDB
    #define VK_OEM_5 0xDC
    #define VK_OEM_6 0xDD
    #define VK_OEM_7 0xDE
    #define VK_NUMPAD0 0x60
    #define VK_NUMPAD1 0x61
    #define VK_NUMPAD2 0x62
    #define VK_NUMPAD3 0x63 
    #define VK_NUMPAD4 0x64
    #define VK_NUMPAD5 0x65
    #define VK_NUMPAD6 0x66
    #define VK_NUMPAD7 0x67
    #define VK_NUMPAD8 0x68
    #define VK_NUMPAD9 0x69

    #define XBUTTON1 1
    #define XBUTTON2 2
#endif