//Game Device implementation that is common between platforms
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#include "GameDevice.h"
#include "../base/MiscStuff.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

#include <cmath>
#include <algorithm>

namespace
{
    std::vector<const INPUT::GAME::Button*> currentlyPressedButtons;
    std::vector<const INPUT::GAME::Button*> newlyPressedButtons;
    std::vector<const INPUT::GAME::AxisSet*> currentlyPressedAxes;
    std::vector<const INPUT::GAME::AxisSet*> newlyPressedAxes;

    bool autoPoll=true;
    const std::string PIDButtonSeperator="::Button::";
    const std::string PIDAxisSetSeperator="::AxisSet::";
}

namespace INPUT_INTERNAL
{
    void ClearGlobalButtonState()
    {
        currentlyPressedButtons.clear();
        newlyPressedButtons.clear();
        currentlyPressedAxes.clear();
        newlyPressedAxes.clear();
    }

    void AutoPollOnUpdate()
    {
        INPUT::GAME::PollAllDevices();
    }
}

namespace INPUT
{
    namespace GAME
    {
        // -- AxisSet

        const std::string& AxisSet::GetPersistentIdentifier() const
        {
            return persistentIdentifier;
        }

        float AxisSet::GetXValue() const
        {
            return xValue;
        }

        float AxisSet::GetYValue() const
        {
            return yValue;
        }

        float AxisSet::GetZValue() const
        {
            return zValue;
        }

        AxisSet::AxisSet(INPUT_INTERNAL::PlatformAxesInformation *objectInfo, Device *device)
        {
            info=objectInfo;
            xValue=0;
            yValue=0;
            zValue=0;
            xDead=INPUT_GAMEDEVICE_DEFAULT_DEAD_ZONE;
            yDead=INPUT_GAMEDEVICE_DEFAULT_DEAD_ZONE;
            zDead=INPUT_GAMEDEVICE_DEFAULT_DEAD_ZONE;
            parentDevice=device;
            persistentIdentifier=parentDevice->GetPersistentIdentifier()+PIDAxisSetSeperator+GetCombinedName();
        }

        AxisSet::AxisSet(AxisSet &&o)
        {
            info=o.info;
            o.info=0;
            xValue=o.xValue;
            yValue=o.yValue;
            zValue=o.zValue;
            xDead=o.xDead;
            yDead=o.yDead;
            zDead=o.zDead;
            parentDevice=o.parentDevice;
            persistentIdentifier=o.persistentIdentifier;
        }

        void AxisSet::SetXDeadZone(float amount)
        {
            xDead=amount;
        }

        void AxisSet::SetYDeadZone(float amount)
        {
            yDead=amount;
        }

        void AxisSet::SetZDeadZone(float amount)
        {
            zDead=amount;
        }

        float AxisSet::GetXDeadZone() const
        {
            return xDead;
        }

        float AxisSet::GetYDeadZone() const
        {
            return yDead;
        }

        float AxisSet::GetZDeadZone() const
        {
            return zDead;
        }

        float AxisSet::ApplyDeadZone(float val, float dead)
        {
            if (std::abs(val)<dead)
                return 0.0f;

            //map dead through 1 to 0 through 1
            float adjustedVal=(std::abs(val)-dead)/(1.0f-dead);
            if (val<0)
                return -adjustedVal;
            else
                return adjustedVal;
        }

        const Device* AxisSet::GetDevice() const
        {
            return parentDevice;
        }


        // -- Button

        const std::string& Button::GetPersistentIdentifier() const
        {
            return persistentIdentifier;
        }

        bool Button::IsPressed() const
        {
            return isPressed;
        }

        const Device* Button::GetDevice() const
        {
            return parentDevice;
        }

        Button::Button(INPUT_INTERNAL::PlatformObjectInformation *objectInfo, Device *device)
        {
            info=objectInfo;
            isPressed=false;
            parentDevice=device;
            persistentIdentifier=parentDevice->GetPersistentIdentifier()+PIDButtonSeperator+GetName();
        }

        Button::Button(Button &&o)
        {
            info=o.info;
            o.info=0;
            isPressed=o.isPressed;
            parentDevice=o.parentDevice;
            persistentIdentifier=o.persistentIdentifier;
        }


        // -- Device

        const std::string& Device::GetName() const
        {
            return friendlyName;
        }

        const std::string& Device::GetPersistentIdentifier() const
        {
            return persistentIdentifier;
        }

        const std::list<AxisSet>& Device::GetAxes() const
        {
            return axes;
        }

        std::list<AxisSet>& Device::GetAxes()
        {
            return axes;
        }

        const std::list<Button>& Device::GetButtons() const
        {
            return buttons;
        }

        std::list<Button>& Device::GetButtons()
        {
            return buttons;
        }


        // -- Global

        void EnableAutoPoll(bool enable)
        {
            autoPoll=enable;
        }

        void PollAllDevices()
        {
            //poll all devices
            std::list<Device>& devices=GetDevices();
            for (auto d=devices.begin(); d!=devices.end(); ++d)
                d->Poll();

            //update the newly pressed states
            newlyPressedButtons.clear();
            newlyPressedAxes.clear();

            for (auto d=devices.begin(); d!=devices.end(); ++d)
            {
                const std::list<Button> &buttons=d->GetButtons();
                for (auto b=buttons.begin(); b!=buttons.end(); ++b)
                {
                    if (b->IsPressed())
                    {
                        if (std::find(currentlyPressedButtons.begin(), currentlyPressedButtons.end(), &*b)==currentlyPressedButtons.end())
                            newlyPressedButtons.push_back(&*b);
                    }
                }

                const std::list<AxisSet> &axes=d->GetAxes();
                for (auto a=axes.begin(); a!=axes.end(); ++a)
                {
                    if (a->GetXValue()!=0 || a->GetYValue()!=0 || a->GetZValue()!=0)
                    {
                        if (std::find(currentlyPressedAxes.begin(), currentlyPressedAxes.end(), &*a)==currentlyPressedAxes.end())
                            newlyPressedAxes.push_back(&*a);
                    }
                }
            }

            //update the currently pressed states
            currentlyPressedButtons.clear();
            currentlyPressedAxes.clear();

            for (auto d=devices.begin(); d!=devices.end(); ++d)
            {
                const std::list<Button> &buttons=d->GetButtons();
                for (auto b=buttons.begin(); b!=buttons.end(); ++b)
                {
                    if (b->IsPressed())
                    {
                        currentlyPressedButtons.push_back(&*b);
                    }
                }

                const std::list<AxisSet> &axes=d->GetAxes();
                for (auto a=axes.begin(); a!=axes.end(); ++a)
                {
                    if (a->GetXValue()!=0 || a->GetYValue()!=0 || a->GetZValue()!=0)
                    {
                        currentlyPressedAxes.push_back(&*a);
                    }
                }
            }
        }

        const std::vector<const Button*>& GetCurrentlyPressedButtons()
        {
            return currentlyPressedButtons;
        }

        const std::vector<const Button*>& GetNewlyPressedButtons()
        {
            return newlyPressedButtons;
        }

        const std::vector<const AxisSet*>& GetCurrentlyPressedAxes()
        {
            return currentlyPressedAxes;
        }

        const std::vector<const AxisSet*>& GetNewlyPressedAxes()
        {
            return newlyPressedAxes;
        }

        Device* FindDevice(const std::string &identifierToFind)
        {
            std::list<Device>& devices=GetDevices();
            for (auto d=devices.begin(); d!=devices.end(); ++d)
            {
                if (d->GetPersistentIdentifier()==identifierToFind)
                    return &*d;
            }

            return 0;
        }

        Button* FindButton(const std::string &identifierToFind)
        {
            std::string deviceId=identifierToFind;
            int ind=MISC::IndexOf(identifierToFind, PIDButtonSeperator);
            if (ind==-1)
                return 0;
            deviceId.resize(ind);
            ind+=(int)PIDButtonSeperator.length();
            std::string buttonId=&identifierToFind.c_str()[ind];

            Device *device=FindDevice(deviceId);
            if (!device)
                return 0;

            std::list<Button>& buttons=device->GetButtons();
            for (auto but=buttons.begin(); but!=buttons.end(); ++but)
            {
                if (but->GetPersistentIdentifier()==identifierToFind)
                    return &*but;
            }

            return 0;
        }

        AxisSet* FindAxisSet(const std::string &identifierToFind)
        {
            std::string deviceId=identifierToFind;
            int ind=MISC::IndexOf(identifierToFind, PIDAxisSetSeperator);
            if (ind==-1)
                return 0;
            deviceId.resize(ind);
            ind+=(int)PIDButtonSeperator.length();
            std::string axissetId=&identifierToFind.c_str()[ind];

            Device *device=FindDevice(deviceId);
            if (!device)
                return 0;

            std::list<AxisSet>& axes=device->GetAxes();
            for (auto as=axes.begin(); as!=axes.end(); ++as)
            {
                if (as->GetPersistentIdentifier()==identifierToFind)
                    return &*as;
            }

            return 0;
        }

        void ClearState()
        {
            INPUT_INTERNAL::ClearGlobalButtonState();
        }
    }
}

#endif //#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
