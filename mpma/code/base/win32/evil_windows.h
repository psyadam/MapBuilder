//counter some of windows.h's evilnesses and include the file
//Luke Lenhart, 2005-2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

#define VC_EXTRALEAN

#include <windows.h>

//fight the evilness!
#undef DrawText //grr
#undef GetObject //oof
#undef min //ugh
#undef max //ack
#undef CreateDirectory //omg
#undef SetPort //wow

//we don't want anyone accidently using these windows.h things either
#undef DWORD
#undef WORD
#undef BYTE
#undef BOOL
