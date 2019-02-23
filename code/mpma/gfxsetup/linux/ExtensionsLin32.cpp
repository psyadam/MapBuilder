//!\file ExtensionsLin32.cpp OpenGL Extension setup - Linux
//Luke Lenhart, 2009
//See /docs/License.txt for details on how this code may be used.

#include "../../Config.h"

#ifdef MPMA_COMPILE_GFXSETUP

#include "../Extensions.h"
#include <GL/glx.h>

namespace GFX
{
    //Returns a pointer to a OpenGL extension function with a specific name, or 0 if not found.
    VoidFuncPtrType GetExtensionFunction(const std::string &name)
    {
        return glXGetProcAddressARB((GLubyte*)name.c_str());
    }
}

#endif //#ifdef MPMA_COMPILE_GFXSETUP
