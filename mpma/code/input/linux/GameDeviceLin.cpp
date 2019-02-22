//Linux-specific implementation of GameDevice
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#include "../GameDevice.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

#include "../../Setup.h"
#include "../../base/Memory.h"
#include "../../base/File.h"
#include "../../base/MiscStuff.h"
#include "../../base/DebugRouter.h"
#include "../../base/Vary.h"
#include "../../gfxsetup/GFXSetup.h"

#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cmath>

#include <iostream>

namespace INPUT_INTERNAL
{
    void ClearGlobalButtonState();
    void AutoPollOnUpdate();

    bool didRegisterUpdateCallback=false;
}

namespace GFX_INTERNAL
{
    void AddUpdateCallback(void (*callback)());
}

namespace INPUT_INTERNAL
{
    std::vector<INPUT::GAME::Device::AxisSetMap> FindDefaultAxisMappings(const std::string &deviceName);

    struct PlatformDeviceData
    {
        std::string Path;
        int AxisCount;
        int ButtonCount;
        int File;

        PlatformDeviceData()
        {
            AxisCount=0;
            ButtonCount=0;
            File=0;
        }

        PlatformDeviceData(PlatformDeviceData &&o)
        {
            AxisCount=o.AxisCount;
            ButtonCount=o.ButtonCount;
            File=o.File;
            o.File=0;
        }

        ~PlatformDeviceData()
        {
            if (File)
            {
                close(File);
                File=0;
            }
        }

    private:
        PlatformDeviceData(const PlatformDeviceData &o); //unused
    };

    struct PlatformObjectInformation
    {
        PlatformObjectInformation(int number, const std::string &name)
        {
            Index=number;
            Name=name;
        }

        int Index;
        std::string Name;
    };

    struct PlatformAxesInformation
    {
        PlatformAxesInformation(int xNumber, int yNumber, int zNumber, const std::string &name)
        {
            IndexX=xNumber;
            IndexY=yNumber;
            IndexZ=zNumber;
            Name=name;

            if (IndexX!=-1)
                NameX=Name+" X";
            if (IndexY!=-1)
                NameY=Name+" Y";
            if (IndexZ!=-1)
                NameZ=Name+" Z";
        }

        int IndexX, IndexY, IndexZ;
        std::string Name;
        std::string NameX, NameY, NameZ;
    };

    //
    std::list<INPUT::GAME::Device> gameDeviceList;

    void InitDeviceList()
    {
        //find all js* devices in /dev/input
        std::list<std::string> foundDevices;
        MPMA::Filename inputPath="/dev/input/";
        std::vector<std::string> fileList=inputPath.GetFiles();
        for (auto i=fileList.begin(); i!=fileList.end(); ++i)
        {
            if (MISC::StartsWith(*i, "js"))
                foundDevices.push_back("/dev/input/"+*i);
        }

        //if none were found, try the old location for it: /dev
        if (foundDevices.empty())
        {
            inputPath="/dev/";
            fileList=inputPath.GetFiles();
            for (auto i=fileList.begin(); i!=fileList.end(); ++i)
            {
                if (MISC::StartsWith(*i, "js"))
                    foundDevices.push_back("/dev/"+*i);
            }
        }

        //create devices to represent these
        for (auto i=foundDevices.begin(); i!=foundDevices.end(); ++i)
        {
            sint32 numAxes=0;
            sint32 numButtons=0;
            int fd=open(i->c_str(), O_RDONLY);
            if (fd==-1)
            {
                MPMA::ErrorReport()<<"Failed to open game device "<<*i<<"\n";
                continue;
            }

            char name[128]={0};
            ioctl(fd, JSIOCGNAME(127), name);
            ioctl(fd, JSIOCGAXES, &numAxes);
            ioctl(fd, JSIOCGBUTTONS, &numButtons);
            if (name[0]==0)
            {
                strcpy(name, "Unknown on ");
                strcat(name, i->c_str());
            }

            fcntl(fd, F_SETFL, O_NONBLOCK);

            PlatformDeviceData *pdi=new3(PlatformDeviceData);
            pdi->Path=*i;
            pdi->File=fd;
            pdi->AxisCount=numAxes;
            pdi->ButtonCount=numButtons;
            gameDeviceList.push_back(INPUT::GAME::Device(pdi, name, *i));
        }

        //register for auto poll
        if (!didRegisterUpdateCallback)
        {
            didRegisterUpdateCallback=true;
            GFX_INTERNAL::AddUpdateCallback(AutoPollOnUpdate);
        }
    }

    void FreeDeviceList()
    {
        ClearGlobalButtonState();
        gameDeviceList.clear();
    }

    class AutoInitGameInput
    {
    public:
        AutoInitGameInput()
        {
            MPMA::Internal_AddInitCallback(InitDeviceList, 8600);
            MPMA::Internal_AddShutdownCallback(FreeDeviceList, 8600);
        }
    } autoInitGameInput;
}

namespace INPUT
{
    namespace GAME
    {
        // -- AxisSet

        const std::string AxisSet::GetCombinedName() const
        {
            return info->Name;
        }

        bool AxisSet::IsXPresent() const
        {
            return info->IndexX!=-1;
        }

        bool AxisSet::IsYPresent() const
        {
            return info->IndexY!=-1;
        }

        bool AxisSet::IsZPresent() const
        {
            return info->IndexZ!=-1;
        }

        const std::string& AxisSet::GetXName() const
        {
            return info->NameX;
        }

        const std::string& AxisSet::GetYName() const
        {
            return info->NameY;
        }

        const std::string& AxisSet::GetZName() const
        {
            return info->NameZ;
        }

        AxisSet::~AxisSet()
        {
            if (info)
                delete3(info);
            info=0;
        }

        void AxisSet::SetState(char *rawdata)
        {
            //Poll sets state directly for axis, this isn't used
        }

        void AxisSet::NormalizeMaximum()
        {
            //pair x and y, treat z separately
            float xyLen=std::sqrt(xValue*xValue+yValue*yValue);
            if (xyLen>1.0f)
            {
                xValue/=xyLen;
                yValue/=xyLen;
            }

            if (zValue<-1.0f)
                zValue=-1.0f;
            if (zValue>1.0f)
                zValue=1.0f;
        }


        // -- Button

        const std::string& Button::GetName() const
        {
            return info->Name;
        }

        Button::~Button()
        {
            if (info)
                delete3(info);
            info=0;
        }

        void Button::SetState(char *rawdata)
        {
            sint16 val=*(sint16*)rawdata;

            if (val)
                isPressed=true;
            else
                isPressed=false;
        }


        // -- Device

        void Device::Poll()
        {
            int ret=0;
            while (ret!=-1)
            {
                js_event jse={0};
                ret=read(data->File, &jse, sizeof(jse));
                if (ret!=-1)
                {
                    if (jse.type==JS_EVENT_BUTTON)
                    {
                        for (auto b=buttons.begin(); b!=buttons.end(); ++b)
                        {
                            if (b->info->Index==jse.number)
                                b->SetState((char*)&jse.value);
                        }
                    }
                    else if (jse.type==JS_EVENT_AXIS)
                    {
                        for (auto a=axes.begin(); a!=axes.end(); ++a)
                        {
                            if (a->info->IndexX==jse.number)
                                a->xValue=a->ApplyDeadZone(jse.value/32767.0f, a->xDead);
                            if (a->info->IndexY==jse.number)
                                a->yValue=a->ApplyDeadZone(jse.value/32767.0f, a->yDead);
                            if (a->info->IndexZ==jse.number)
                                a->zValue=a->ApplyDeadZone(jse.value/32767.0f, a->zDead);
                        }
                    }
                }
            }

            for (auto a=axes.begin(); a!=axes.end(); ++a)
                a->NormalizeMaximum();

            //if we don't have focus, clear all state
            if (!GFX::WindowHasFocus())
            {
                sint16 butState=0;
                for (auto b=buttons.begin(); b!=buttons.end(); ++b)
                    b->SetState((char*)&butState);

                for (auto a=axes.begin(); a!=axes.end(); ++a)
                {
                    a->xValue=0;
                    a->yValue=0;
                    a->zValue=0;
                }
            }
        }

        Device::Device(INPUT_INTERNAL::PlatformDeviceData *platformData, const std::string &name, const std::string &persistentId)
        {
            data=platformData;
            friendlyName=name;
            persistentIdentifier=persistentId;

            //buttons
            for (int i=0; i<platformData->ButtonCount; ++i)
            {
                MPMA::Vary name="Button ";
                name+=i;

                buttons.push_back(Button(new3(INPUT_INTERNAL::PlatformObjectInformation(i, name)), this));
            }

            //axes
            SetAxitMaps(INPUT_INTERNAL::FindDefaultAxisMappings(name));
        }

        Device::Device(Device &&o)
        {
            data=o.data;
            o.data=0;
            friendlyName=o.friendlyName;
            persistentIdentifier=o.persistentIdentifier;

            axes=std::move(o.axes);
            buttons=std::move(o.buttons);
            axisMaps=std::move(o.axisMaps);
        }

        Device::~Device()
        {
            buttons.clear();
            axes.clear();

            if (data)
                delete3(data);
            data=0;
        }

        const std::vector<Device::AxisSetMap>& Device::GetAxitMaps() const
        {
            return axisMaps;
        }

        void Device::SetAxitMaps(const std::vector<Device::AxisSetMap> &maps)
        {
            axisMaps=maps;

            if (axisMaps.empty()) //use a default guess if none were provided
            {
                for (int i=0; i<data->AxisCount/2; ++i) //axis comes in pairs of x/y
                {
                    MPMA::Vary name="AxisSet ";
                    name+=i;

                    AxisSetMap axisXY;
                    axisXY.XNumber=i*2;
                    axisXY.YNumber=i*2+1;
                    axisXY.Name=(std::string)name;

                    axisMaps.push_back(axisXY);
                }

                if (data->AxisCount%2==1) //assign any straggler to z
                {
                    AxisSetMap axisZ;
                    axisZ.ZNumber=data->AxisCount-1;
                    axisZ.Name="Axis Z";

                    axisMaps.push_back(axisZ);
                }
            }

            axes.clear();
            for (auto i=axisMaps.begin(); i!=axisMaps.end(); ++i)
            {
                if (i->XNumber>=data->AxisCount)
                    i->XNumber=-1;
                if (i->YNumber>=data->AxisCount)
                    i->YNumber=-1;
                if (i->ZNumber>=data->AxisCount)
                    i->ZNumber=-1;
                if (i->Name.empty())
                    i->Name="Unnamed AxisSet";

                axes.push_back(AxisSet(new3(INPUT_INTERNAL::PlatformAxesInformation(i->XNumber, i->YNumber, i->ZNumber, i->Name)), this));
            }
        }

        int Device::GetAxisCount() const
        {
            return data->AxisCount;
        }

        std::list<Device>& GetDevices()
        {
            return INPUT_INTERNAL::gameDeviceList;
        }
    }
}

bool mpmaForceReferenceToGameDeviceWinCPP=false;

#endif //#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
