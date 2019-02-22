//tests that the input
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "base/Timer.h"
#include "base/MiscStuff.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "input/GameDevice.h"
#include "input/Unified.h"
#include "gfxsetup/GFXSetup.h"
#include "gfxsetup/GL.h"
#include "geo/Geo.h"
#include <iostream>
#include <cmath>

#ifdef DECLARE_TESTS_CODE
class InputKeyboardMouseTest: public UnitTest
{
public:
    bool Run()
    {
        GFX::GraphicsSetup setup;
        setup.Width=200;
        setup.Height=200;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        //loop until window is closed
        INPUT::MOUSE::MouseScaledPosition lastMousePos;
        lastMousePos.X=-1; lastMousePos.Y=-1;

        MPMA::Timer timer;
        std::string userText;
        for (;;)
        {
            MPMA::Sleep(100);

            //do some default stuff
            GFX::UpdateWindow();

            const GFX::GraphicsSetup *state=GFX::GetWindowState();
            if (state==0)
                break;

            //clear
            glViewport(0, 0, state->Width, state->Height);

            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            //echo pressed keys
            const std::vector<INPUT::UnifiedButton*> &newKeys=INPUT::GetNewlyPressedButtons();
            if (!newKeys.empty())
            {
                std::cout<<"Newly Pressed: ";
                for (std::vector<INPUT::UnifiedButton*>::const_iterator i=newKeys.begin(); i!=newKeys.end(); ++i)
                    std::cout<<" "<<(**i).GetFriendlyName();
                std::cout<<std::endl;
            }

            const std::vector<INPUT::UnifiedButton*> &curKeys=INPUT::GetCurrentlyPressedButtons();
            if (!curKeys.empty())
            {
                std::cout<<"Currently Pressed: ";
                for (std::vector<INPUT::UnifiedButton*>::const_iterator i=curKeys.begin(); i!=curKeys.end(); ++i)
                    std::cout<<" "<<(**i).GetFriendlyName();
                std::cout<<std::endl;
            }

            //echo out typed text
            if (INPUT::KEYBOARD::UpdateTypedText(userText))
                std::cout<<"Typed text: "<<userText<<"\n";

            //echo mouse coords and scroll
            INPUT::MOUSE::MouseScaledPosition mousePos=INPUT::MOUSE::GetScaledPosition();
            if (mousePos!=lastMousePos)
            {
                lastMousePos=mousePos;

                std::cout<<"Mouse move: "<<mousePos.X<<", "<<mousePos.Y<<std::endl;
            }

            const std::vector<INPUT::MOUSE::MouseScaledPosition>& mouseTrail=INPUT::MOUSE::GetScaledTrail();
            if (mouseTrail.size()>0)
                std::cout<<"Mouse trail count: "<<mouseTrail.size()<<std::endl;

            int mouseWheel=INPUT::MOUSE::GetWheelDirection();
            if (mouseWheel!=0)
                std::cout<<"Mouse wheel: "<<mouseWheel<<std::endl;
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(InputKeyboardMouseTest);


#if defined(ENABLE_GAME_DEVICE_INPUT)

#ifdef DECLARE_TESTS_CODE
class InputGameTest: public UnitTest
{
public:
    bool Run()
    {
        //print out game device list
        for (auto d=INPUT::GAME::GetDevices().begin(); d!=INPUT::GAME::GetDevices().end(); ++d)
        {
            std::cout<<"Game Device Found: "<<d->GetName()<<std::endl;

            std::cout<<"Buttons:\n";
            for (auto b=d->GetButtons().begin(); b!=d->GetButtons().end(); ++b)
                std::cout<<b->GetName()<<"\n";
            std::cout<<std::endl;

            std::cout<<"Axes:\n";
            for (auto a=d->GetAxes().begin(); a!=d->GetAxes().end(); ++a)
                std::cout<<a->GetCombinedName()<<"\n";
            std::cout<<std::endl;

            std::cout<<std::endl;
        }

        //
        GFX::GraphicsSetup setup;
        setup.Width=200;
        setup.Height=200;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        //loop until window is closed
        std::string userText;
        for (;;)
        {
            MPMA::Sleep(250);

            //do some default stuff
            GFX::UpdateWindow();

            const GFX::GraphicsSetup *state=GFX::GetWindowState();
            if (state==0)
                break;

            //clear
            glViewport(0, 0, state->Width, state->Height);

            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            //print devices states
            for (auto d=INPUT::GAME::GetDevices().begin(); d!=INPUT::GAME::GetDevices().end(); ++d)
            {
                for (auto b=d->GetButtons().begin(); b!=d->GetButtons().end(); ++b)
                {
                    if (b->IsPressed())
                        std::cout<<d->GetName()<<" - "<<b->GetName()<<std::endl;
                }

                for (auto a=d->GetAxes().begin(); a!=d->GetAxes().end(); ++a)
                {
                    if (a->IsXPresent() && a->GetXValue()!=0)
                        std::cout<<d->GetName()<<" - "<<a->GetXName()<<": "<<a->GetXValue()<<std::endl;
                    if (a->IsYPresent() && a->GetYValue()!=0)
                        std::cout<<d->GetName()<<" - "<<a->GetYName()<<": "<<a->GetYValue()<<std::endl;
                    if (a->IsZPresent() && a->GetZValue()!=0)
                        std::cout<<d->GetName()<<" - "<<a->GetZName()<<": "<<a->GetZValue()<<std::endl;
                }
            }
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(InputGameTest);
#endif //defined(ENABLE_GAME_DEVICE_INPUT)

#ifdef DECLARE_TESTS_CODE
class InputUnifiedTest: public UnitTest
{
public:
    bool Run()
    {
        GFX::GraphicsSetup setup;
        setup.Width=250;
        setup.Height=250;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        INPUT::KeyboardLettersActsAsAxis=true;

        //loop until window is closed
        std::string userText;
        for (;;)
        {
            MPMA::Sleep(100);

            //do some default stuff
            GFX::UpdateWindow();

            const GFX::GraphicsSetup *state=GFX::GetWindowState();
            if (state==0)
                break;

            //clear
            glViewport(0, 0, state->Width, state->Height);

            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            //print out button states
            const std::vector<INPUT::UnifiedButton*>& newButtons=INPUT::GetNewlyPressedButtons();
            const std::vector<INPUT::UnifiedButton*>& currentButtons=INPUT::GetCurrentlyPressedButtons();

            for (auto b=newButtons.begin(); b!=newButtons.end(); ++b)
                std::cout<<"New Button: "<<(**b).GetFriendlyName()<<"\n"<<(**b).GetPersistentIdentifier()<<"\n";

            for (auto b=currentButtons.begin(); b!=currentButtons.end(); ++b)
                std::cout<<"Current Button: "<<(**b).GetFriendlyName()<<"\n";

            //print out the axis states
            const std::vector<INPUT::UnifiedAxisSet*> newAxes=INPUT::GetNewlyPressedAxes();
            const std::vector<INPUT::UnifiedAxisSet*> currentAxes=INPUT::GetCurrentlyPressedAxes();

            for (auto a=newAxes.begin(); a!=newAxes.end(); ++a)
                std::cout<<"New Axis: "<<(**a).GetFriendlyName()<<": "<<(**a).GetXValue()<<", "<<(**a).GetYValue()<<", "<<(**a).GetZValue()<<"\n"<<(**a).GetPersistentIdentifier()<<"\n";
            for (auto a=currentAxes.begin(); a!=currentAxes.end(); ++a)
                std::cout<<"Current Axis: "<<(**a).GetFriendlyName()<<": "<<(**a).GetXValue()<<", "<<(**a).GetYValue()<<", "<<(**a).GetZValue()<<"\n";

            //verify persistent IDs work
            for (auto b=newButtons.begin(); b!=newButtons.end(); ++b)
            {
                INPUT::UnifiedButton *foundButton=INPUT::FindUnifiedButton((**b).GetPersistentIdentifier());
                if (!foundButton)
                {
                    std::cout<<"Could not find unified button for just-pressed button.\n";
                    std::cout<<"Name="<<(**b).GetFriendlyName()<<"\n";
                    std::cout<<"Id="<<(**b).GetPersistentIdentifier()<<"\n";
                    ok=false;
                }
                else
                {
                    if (foundButton!=*b) //even the pointer should be the same now
                    {
                        std::cout<<"Pointer for found unified axis differs from original pointer\n";
                        ok=false;
                    }

                    if (foundButton->GetPersistentIdentifier()!=(**b).GetPersistentIdentifier())
                    {
                        std::cout<<"ID for found unified button differs from original ID\n";
                        std::cout<<"Original ID: "<<(**b).GetPersistentIdentifier()<<"\n";
                        std::cout<<"Found ID: "<<foundButton->GetPersistentIdentifier()<<"\n";
                        ok=false;
                    }

                    if (foundButton->GetFriendlyName()!=(**b).GetFriendlyName())
                    {
                        std::cout<<"Name for found unified button differs from original Name\n";
                        std::cout<<"Original Name: "<<(**b).GetPersistentIdentifier()<<"\n";
                        std::cout<<"Found Name: "<<foundButton->GetPersistentIdentifier()<<"\n";
                        ok=false;
                    }

                    if (foundButton->IsPressed()!=(**b).IsPressed())
                    {
                        std::cout<<"IsPressed state for found unified button differs from original IsPressed state for button: "<<(**b).GetFriendlyName()<<"\n";
                        std::cout<<"Original State: "<<(**b).IsPressed()<<"\n";
                        std::cout<<"Found State: "<<foundButton->IsPressed()<<"\n";
                        ok=false;
                    }
                }
            }

            for (auto a=newAxes.begin(); a!=newAxes.end(); ++a)
            {
                INPUT::UnifiedAxisSet *foundAxis=INPUT::FindUnifiedAxisSet((**a).GetPersistentIdentifier());
                if (!foundAxis)
                {
                    std::cout<<"Could not find unified axis for just-pressed axis.\n";
                    std::cout<<"Name="<<(**a).GetFriendlyName()<<"\n";
                    std::cout<<"Id="<<foundAxis->GetPersistentIdentifier()<<"\n";
                    ok=false;
                }
                else
                {
                    if (foundAxis!=*a) //even the pointer should be the same now
                    {
                        std::cout<<"Pointer for found unified axis differs from original pointer\n";
                        ok=false;
                    }

                    if (foundAxis->GetPersistentIdentifier()!=(**a).GetPersistentIdentifier())
                    {
                        std::cout<<"ID for found unified axis differs from original ID\n";
                        std::cout<<"Original ID: "<<(**a).GetPersistentIdentifier()<<"\n";
                        std::cout<<"Found ID: "<<foundAxis->GetPersistentIdentifier()<<"\n";
                        ok=false;
                    }

                    if (foundAxis->GetFriendlyName()!=(**a).GetFriendlyName())
                    {
                        std::cout<<"Name for found unified axis differs from original Name\n";
                        std::cout<<"Original Name: "<<(**a).GetPersistentIdentifier()<<"\n";
                        std::cout<<"Found Name: "<<foundAxis->GetPersistentIdentifier()<<"\n";
                        ok=false;
                    }

                    if (foundAxis->GetXValue()!=(**a).GetXValue())
                    {
                        std::cout<<"GetXValue state for found unified axis differs from original GetXValue state for axis: "<<(**a).GetFriendlyName()<<"\n";
                        std::cout<<"Original State: "<<(**a).GetXValue()<<"\n";
                        std::cout<<"Found State: "<<foundAxis->GetXValue()<<"\n";
                        ok=false;
                    }

                    if (foundAxis->GetYValue()!=(**a).GetYValue())
                    {
                        std::cout<<"GetYValue state for found unified axis differs from original GetYValue state for axis: "<<(**a).GetFriendlyName()<<"\n";
                        std::cout<<"Original State: "<<(**a).GetYValue()<<"\n";
                        std::cout<<"Found State: "<<foundAxis->GetYValue()<<"\n";
                        ok=false;
                    }

                    if (foundAxis->GetZValue()!=(**a).GetZValue())
                    {
                        std::cout<<"GetZValue state for found unified axis differs from original GetZValue state for axis: "<<(**a).GetFriendlyName()<<"\n";
                        std::cout<<"Original State: "<<(**a).GetZValue()<<"\n";
                        std::cout<<"Found State: "<<foundAxis->GetZValue()<<"\n";
                        ok=false;
                    }
                }
            }
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(InputUnifiedTest);
