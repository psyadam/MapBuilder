//Windows-specific implementation of GameDevice
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#include "../../Setup.h"
#include "../GameDevice.h"
#include "../../base/Memory.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

#include "WrapDInput.h"

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
    const float PI2=6.2831853071f;

    struct PlatformDeviceData
    {
        PlatformDeviceData(INPUT_INTERNAL::DeviceAndObjects *pdevice)
        {
            pDev=pdevice;
        }

        ~PlatformDeviceData()
        {
            if (pDev)
                DInput_Device_Release(pDev);
            pDev=0;
        }

        INPUT_INTERNAL::DeviceAndObjects *pDev;
    };

    struct PlatformObjectInformation
    {
        PlatformObjectInformation(DeviceObject *pdo)
        {
            pDevObj=pdo;
        }

        ~PlatformObjectInformation()
        {
            //we only have a pointer to something persistant, we don't own this pointer so don't free
            pDevObj=0;
        }

        DeviceObject *pDevObj;
    };

    struct PlatformAxesInformation
    {
        PlatformAxesInformation(DeviceAxisSet *pdo, INPUT_INTERNAL::Hat *hat)
        {
            pDevObj=pdo;
            pHat=hat;
        }

        ~PlatformAxesInformation()
        {
            //we only have a pointer to something persistant, we don't own this pointer so don't free
            pDevObj=0;
            pHat=0;
        }

        DeviceAxisSet *pDevObj;
        INPUT_INTERNAL::Hat *pHat;
    };

    //
    std::list<INPUT::GAME::Device> gameDeviceList;

    void InitDeviceList()
    {
        if (DInput_Init())
        {
            std::list<DeviceInfo> devinfo=DInput_EnumGameDevices();
            for (auto i=devinfo.begin(); i!=devinfo.end(); ++i)
            {
                INPUT_INTERNAL::DeviceAndObjects *pDev=DInput_CreateDevice(*i);
                if (pDev)
                    gameDeviceList.emplace_back(new3(PlatformDeviceData(pDev)), i->InstanceName, i->PersistantIdentifier);
            }
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
            if (info->pHat)
                return info->pHat->GetName();

            std::string s;
            if (IsXPresent())
                s+=GetXName();

            if (IsYPresent())
            {
                if (s.length()>0)
                    s+=" / ";
                s+=GetYName();
            }

            if (IsZPresent())
            {
                if (s.length()>0)
                    s+=" / ";
                s+=GetZName();
            }

            return s;
        }

        bool AxisSet::IsXPresent() const
        {
            if (info->pDevObj && info->pDevObj->X.Offset!=-1)
                return true;
            else if (info->pHat)
                return true;
            else
                return false;
        }

        bool AxisSet::IsYPresent() const
        {
            if (info->pDevObj && info->pDevObj->Y.Offset!=-1)
                return true;
            else if (info->pHat)
                return true;
            else
                return false;
        }

        bool AxisSet::IsZPresent() const
        {
            if (info->pDevObj && info->pDevObj->Z.Offset!=-1)
                return true;
            else
                return false;
        }

        const std::string& AxisSet::GetXName() const
        {
            if (info->pDevObj)
                return info->pDevObj->X.Name;
            else
                return info->pHat->GetXName();
        }

        const std::string& AxisSet::GetYName() const
        {
            if (info->pDevObj)
                return info->pDevObj->Y.Name;
            else
                return info->pHat->GetYName();
        }

        const std::string& AxisSet::GetZName() const
        {
            if (info->pDevObj)
                return info->pDevObj->Z.Name;
            else
                return info->pHat->GetZName();
        }

        AxisSet::~AxisSet()
        {
            if (info)
                delete3(info);
            info=0;
        }

        void AxisSet::SetState(char *rawdata)
        {
            if (info->pHat) //hat will be set directly by Poll
                return;

            if (IsXPresent())
            {
                sint32 valX=*(uint32*)(rawdata+info->pDevObj->X.Offset); //0-0xffff
                xValue=(valX-0x7fff)/(float)0x7fff;
                if (xValue>1.0f)
                    xValue=1.0f;
                xValue=ApplyDeadZone(xValue, xDead);
            }

            if (IsYPresent())
            {
                sint32 valY=*(uint32*)(rawdata+info->pDevObj->Y.Offset); //0-0xffff
                yValue=(valY-0x7fff)/(float)0x7fff;
                if (yValue>1.0f)
                    yValue=1.0f;
                yValue=ApplyDeadZone(yValue, yDead);
            }

            if (IsZPresent())
            {
                sint32 valZ=*(uint32*)(rawdata+info->pDevObj->Z.Offset); //0-0xffff
                zValue=(valZ-0x7fff)/(float)0x7fff;
                if (zValue>1.0f)
                    zValue=1.0f;
                zValue=ApplyDeadZone(zValue, zDead);
            }
        }


        // -- Button

        const std::string& Button::GetName() const
        {
            return info->pDevObj->Name;
        }

        Button::~Button()
        {
            if (info)
                delete3(info);
            info=0;
        }

        void Button::SetState(char *rawdata)
        {
            if (*(rawdata+info->pDevObj->Offset)&0x80)
                isPressed=true;
            else
                isPressed=false;
        }

        // -- Hat
    } //namespace GAME
} //namespace INPUT
namespace INPUT_INTERNAL
{
    const std::string& Hat::GetName() const
    {
        return info->pDevObj->Name;
    }

    const std::string& Hat::GetXName() const
    {
        return nameX;
    }

    const std::string& Hat::GetYName() const
    {
        return nameY;
    }

    const std::string& Hat::GetZName() const
    {
        return nameZ;
    }

    bool Hat::IsPressed() const
    {
        return isPressed;
    }

    float Hat::GetRotation() const
    {
        return value;
    }

    Hat::Hat(INPUT_INTERNAL::PlatformObjectInformation *objectInfo)
    {
        info=objectInfo;
        isPressed=false;
        value=0;
        nameX=info->pDevObj->Name+" X";
        nameY=info->pDevObj->Name+" Y";
    }

    Hat::Hat(Hat &&o)
    {
        info=o.info;
        o.info=0;
        isPressed=o.isPressed;
        value=o.value;
        nameX=o.nameX;
        nameY=o.nameY;
        nameZ=o.nameZ;
    }

    Hat::~Hat()
    {
        if (info)
            delete3(info);
        info=0;
    }

    void Hat::SetState(char *rawdata)
    {
        uint32 val=*(uint32*)(rawdata+info->pDevObj->Offset);

        if ((val&0xffff)!=0xffff)
        {
            isPressed=true;
            value=val/100.0f/360.0f*INPUT_INTERNAL::PI2;
        }
        else
        {
            isPressed=false;
            value=0;
        }
    }
} //namespace iNPUT_INTERNAL
namespace INPUT
{
    namespace GAME
    {
        // -- Device

        void Device::Poll()
        {
            if (DInput_Device_Poll(data->pDev))
            {
                //update button states
                for (auto b=buttons.begin(); b!=buttons.end(); ++b)
                    b->SetState(data->pDev->pCurrentStateData);

                //update axes
                for (auto a=axes.begin(); a!=axes.end(); ++a)
                    a->SetState(data->pDev->pCurrentStateData);

                //update hats
                for (auto h=hats.begin(); h!=hats.end(); ++h)
                    h->SetState(data->pDev->pCurrentStateData);

                //update axes that are hats
                for (auto a=axes.begin(); a!=axes.end(); ++a)
                {
                    if (a->info->pHat)
                    {
                        a->xValue=0;
                        a->yValue=0;
                        a->zValue=0;
                        if (a->info->pHat->IsPressed())
                        {
                            float angle=a->info->pHat->GetRotation();
                            a->xValue=std::sin(angle);
                            a->yValue=std::cos(angle);
                        }
                    }
                }
            }
        }

        Device::Device(INPUT_INTERNAL::PlatformDeviceData *platformData, const std::string &name, const std::string &persistentId)
        {
            data=platformData;
            friendlyName=name;
            persistentIdentifier=persistentId;

            //buttons
            for (auto i=platformData->pDev->Buttons.begin(); i!=platformData->pDev->Buttons.end(); ++i)
            {
                buttons.emplace_back(new3(INPUT_INTERNAL::PlatformObjectInformation(&*i)), this);
            }

            //axes
            for (auto i=platformData->pDev->Axes.begin(); i!=platformData->pDev->Axes.end(); ++i)
            {
                axes.emplace_back(new3(INPUT_INTERNAL::PlatformAxesInformation(&*i, 0)), this);
            }

            //hats
            for (auto i=platformData->pDev->Hats.begin(); i!=platformData->pDev->Hats.end(); ++i)
            {
                hats.emplace_back(new3(INPUT_INTERNAL::PlatformObjectInformation(&*i)));
            }

            //map hats to axes
            for (auto i=hats.begin(); i!=hats.end(); ++i)
            {
                axes.emplace_back(new3(INPUT_INTERNAL::PlatformAxesInformation(0, &*i)), this);
            }
        }

        Device::Device(Device &&o)
        {
            data=o.data;
            o.data=0;
            friendlyName=o.friendlyName;
            persistentIdentifier=o.persistentIdentifier;

            axes=std::move(o.axes);
            buttons=std::move(o.buttons);
            hats=std::move(o.hats);
        }

        Device::~Device()
        {
            buttons.clear();
            axes.clear();
            hats.clear();

            if (data)
                delete3(data);
            data=0;
        }

        std::list<Device>& GetDevices()
        {
            return INPUT_INTERNAL::gameDeviceList;
        }
    }
}

bool mpmaForceReferenceToGameDeviceWinCPP=false;

#endif //#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
