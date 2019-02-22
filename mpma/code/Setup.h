//!\file Setup.h Initialization code for the framework.
//See /docs/License.txt for details on how this code may be used.

#pragma once

//! The MPMA Framework
namespace MPMA
{
    //!\brief You MUST declare this (and only 1 of it) at the start of the scope in which you want the framework to be used.
    //!It will handle all setup when constructed, and all cleanup required when destructed.
    //!Generally you will just declare this at the start of main().
    class InitAndShutdown
    {
    public:
        InitAndShutdown();
        ~InitAndShutdown();
    };

    //Internal framework use only: registers init and shutdown callbacks
    void Internal_AddInitCallback(void (*pfunc)(), int priority);
    void Internal_AddShutdownCallback(void (*pfunc)(), int priority);
}
