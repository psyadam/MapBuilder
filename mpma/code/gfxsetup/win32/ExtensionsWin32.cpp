//!\file ExtensionsWin32.cpp OpenGL Extension setup - Windows
//Luke Lenhart, 2009
//See /docs/License.txt for details on how this code may be used.

#include "../../Config.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include <Windows.h>
#include "../Extensions.h"

namespace GFX
{
    //Returns a pointer to a OpenGL extension function with a specific name, or 0 if not found.
    VoidFuncPtrType GetExtensionFunction(const std::string &name)
    {
        return (VoidFuncPtrType)wglGetProcAddress(name.c_str());
    }
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
