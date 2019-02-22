//Wrap DirectInput calls to avoid directly polluting the global namespace of any of our real code with windows.h spam, since a lot of their names collide with ours.
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#include "WrapDInput.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

#include "../../base/DebugRouter.h"
#include "../../base/Vary.h"
#include "../../base/MiscStuff.h"
#include <algorithm>

#define INITGUID
#include <InitGuid.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace GFX_INTERNAL
{
    void AddWindowCreateCallback(void (*callback)(HWND));
    void AddWindowDestroyCallback(void (*callback)(HWND));
}

namespace INPUT_INTERNAL
{
    IDirectInput8 *pDI=0;
    bool hasSetWindowCallbacks=false;
    HWND hwnd=0;

    std::list<DeviceAndObjects*> allDevicesCreated;

    void CallbackWindowCreated(HWND h)
    {
        if (hwnd)
            return;

        hwnd=h;

        for (auto i=allDevicesCreated.begin(); i!=allDevicesCreated.end(); ++i)
        {
            HRESULT ret=(**i).pDev->SetCooperativeLevel(hwnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
            if (ret!=DI_OK)
            {
                MPMA::ErrorReport()<<"DirectInput Device SetCooperativeLevel (CallbackWindowCreated) failed: "<<ret<<"\n";
            }

            (**i).pDev->Unacquire();

            ret=(**i).pDev->Acquire();
            if (ret!=DI_OK)
                MPMA::ErrorReport()<<"DirectInput Device Acquire failed: "<<ret<<"\n";
        }
    }

    void CallbackWindowDestroyed(HWND h)
    {
        hwnd=0;

        for (auto i=allDevicesCreated.begin(); i!=allDevicesCreated.end(); ++i)
        {
            (**i).pDev->SetCooperativeLevel(0, 0); //needed?
            (**i).pDev->Unacquire();
        }
    }

    DeviceAndObjects::~DeviceAndObjects()
    {
        if (pCurrentStateData)
            delete3_array(pCurrentStateData);
        pCurrentStateData=0;
    }

    std::string MakeGuidString(const GUID &g)
    {
        return MPMA::Vary((uint32)g.Data1).AsHexString()+"-"+MPMA::Vary((uint32)g.Data2).AsHexString()+"-"+MPMA::Vary((uint32)g.Data3).AsHexString()+"-"+MPMA::Vary(*(uint64*)&g.Data4).AsHexString();
    }

    GUID GetGuidFromString(const std::string &s)
    {
        GUID g;
        g.Data1=0;
        g.Data2=0;
        g.Data3=0;
        *(uint64*)&g.Data4=0;

        std::vector<std::string> parts;
        MISC::ExplodeString(s, parts, "-");
        if (parts.size()!=4)
        {
            MPMA::ErrorReport()<<"Malformed guidstring passed to GetGuidFromString: "<<s<<"\n";
            return g;
        }

        g.Data1=(unsigned long)MPMA::Vary::FromHexString(parts[0]).AsInt();
        g.Data2=(unsigned short)MPMA::Vary::FromHexString(parts[1]).AsInt();
        g.Data3=(unsigned short)MPMA::Vary::FromHexString(parts[2]).AsInt();
        *(uint64*)&g.Data4=MPMA::Vary::FromHexString(parts[3]).AsInt();
        return g;
    }

    //resets the state of a DIJOYSTATE2 to be "nothing pressed"
    void ClearDeviceState(char *mem)
    {
        memset(mem, 0, sizeof(DIJOYSTATE2));

        DIJOYSTATE2 *pJoy=(DIJOYSTATE2*)mem;
        for (int i=0; i<4; ++i)
            pJoy->rgdwPOV[i]=0xffffffff; //center
    }

    // -

    bool DInput_Init()
    {
        if (pDI)
            return true;

        HRESULT ret=DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&pDI, 0);
        if (ret!=DI_OK || pDI==0)
        {
            MPMA::ErrorReport()<<"DirectInput8Create failed: "<<ret<<"\n";
            return false;
        }

        if (!hasSetWindowCallbacks)
        {
            hasSetWindowCallbacks=true;
            GFX_INTERNAL::AddWindowCreateCallback(CallbackWindowCreated);
            GFX_INTERNAL::AddWindowDestroyCallback(CallbackWindowDestroyed);
        }

        return true;
    }

    BOOL EnumGameDevicesCallback(DIDEVICEINSTANCE *pDev, void *pVoidList)
    {
        std::list<DeviceInfo> &deviceList=*(std::list<DeviceInfo>*)pVoidList;

        DeviceInfo info;
        info.DeviceName=pDev->tszProductName;
        info.InstanceName=pDev->tszInstanceName;
        info.PersistantIdentifier=MakeGuidString(pDev->guidInstance);

        deviceList.emplace_back(std::move(info));
        return DIENUM_CONTINUE;
    }

    std::list<DeviceInfo> DInput_EnumGameDevices()
    {
        if (!pDI)
            return std::list<DeviceInfo>();

        std::list<DeviceInfo> deviceList;
        HRESULT ret=pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)EnumGameDevicesCallback, &deviceList, DIEDFL_ATTACHEDONLY);
        if (ret!=DI_OK)
            MPMA::ErrorReport()<<"DirectInput8 EnumDevices failed: "<<ret<<"\n";

        return deviceList;
    }

    DeviceAndObjects* DInput_CreateDevice(const DeviceInfo &device)
    {
        if (!pDI)
            return 0;

        //create
        GUID g=GetGuidFromString(device.PersistantIdentifier);
        IDirectInputDevice8 *pDev=0;
        HRESULT ret=pDI->CreateDevice(g, &pDev, 0);
        if (ret!=DI_OK || !pDev)
        {
            MPMA::ErrorReport()<<"DirectInput8 CreateDevice failed: "<<ret<<"\n";
            return 0;
        }

        //set format
        ret=pDev->SetDataFormat(&c_dfDIJoystick2);
        if (ret!=DI_OK)
        {
            MPMA::ErrorReport()<<"DirectInput8 Device SetDataFormat failed: "<<ret<<"\n";
            pDev->Release();
            return 0;
        }

        //walk device objects
        DeviceAndObjects *pWrappedDevice=new3(DeviceAndObjects);
        pWrappedDevice->Info=device;
        pWrappedDevice->pDev=pDev;
        pWrappedDevice->pCurrentStateData=new3_array(char, sizeof(DIJOYSTATE2));
        ClearDeviceState(pWrappedDevice->pCurrentStateData);

        DIDEVICEOBJECTINSTANCE oi={0};
        oi.dwSize=sizeof(DIDEVICEOBJECTINSTANCE);

        int offset=0;
        for (int axis=0; axis<2; ++axis) //first two axis sets
        {
            bool valid=false;
            DeviceAxisSet axisSet;
            ret=pDev->GetObjectInfo(&oi, offset+0, DIPH_BYOFFSET);
            if (ret==DI_OK)
            {
                valid=true;
                axisSet.X.Name=oi.tszName;
                axisSet.X.Offset=offset;
            }

            ret=pDev->GetObjectInfo(&oi, offset+4, DIPH_BYOFFSET);
            if (ret==DI_OK)
            {
                valid=true;
                axisSet.Y.Name=oi.tszName;
                axisSet.Y.Offset=offset+4;
            }

            ret=pDev->GetObjectInfo(&oi, offset+8, DIPH_BYOFFSET);
            if (ret==DI_OK)
            {
                valid=true;
                axisSet.Z.Name=oi.tszName;
                axisSet.Z.Offset=offset+8;
            }

            if (valid)
                pWrappedDevice->Axes.emplace_back(std::move(axisSet));

            offset+=12;
        }

        offset+=8; //skip 2 sliders

        for (int pov=0; pov<4; ++pov) //4 hats
        {
            ret=pDev->GetObjectInfo(&oi, offset, DIPH_BYOFFSET);
            if (ret==DI_OK)
            {
                DeviceObject devObj;
                devObj.Name=oi.tszName;
                devObj.Offset=offset;
                pWrappedDevice->Hats.emplace_back(std::move(devObj));
            }

            offset+=4;
        }

        for (int but=0; but<128; ++but) //128 buttons
        {
            ret=pDev->GetObjectInfo(&oi, offset, DIPH_BYOFFSET);
            if (ret==DI_OK)
            {
                DeviceObject devObj;
                devObj.Name=oi.tszName;
                devObj.Offset=offset;
                pWrappedDevice->Buttons.emplace_back(std::move(devObj));
            }

            offset+=1;
        }

        for (int lastThreeAxisSliderSets=0; lastThreeAxisSliderSets<3; ++lastThreeAxisSliderSets) //there are now 3 more sets of: 3 axis + 3 axis + 2 slider
        {
            for (int axis=0; axis<2; ++axis) //two axis sets
            {
                bool valid=false;
                DeviceAxisSet axisSet;
                ret=pDev->GetObjectInfo(&oi, offset+0, DIPH_BYOFFSET);
                if (ret==DI_OK)
                {
                    valid=true;
                    axisSet.X.Name=oi.tszName;
                    axisSet.X.Offset=offset;
                }

                ret=pDev->GetObjectInfo(&oi, offset+4, DIPH_BYOFFSET);
                if (ret==DI_OK)
                {
                    valid=true;
                    axisSet.Y.Name=oi.tszName;
                    axisSet.Y.Offset=offset+4;
                }

                ret=pDev->GetObjectInfo(&oi, offset+8, DIPH_BYOFFSET);
                if (ret==DI_OK)
                {
                    valid=true;
                    axisSet.Z.Name=oi.tszName;
                    axisSet.Z.Offset=offset+8;
                }

                if (valid)
                    pWrappedDevice->Axes.emplace_back(std::move(axisSet));

                offset+=12;
            }

            offset+=8; //skip 2 sliders
        }

        if (hwnd!=0)
        {
            pWrappedDevice->pDev->SetCooperativeLevel(hwnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
            if (ret!=DI_OK)
            {
                MPMA::ErrorReport()<<"DirectInput Device SetCooperativeLevel (DInput_CreateDevice) failed: "<<ret<<"\n";
            }
        }

        allDevicesCreated.push_back(pWrappedDevice);
        DInput_Device_Poll(pWrappedDevice);
        return pWrappedDevice;
    }

    void DInput_Device_Release(DeviceAndObjects *pWrappedDevice)
    {
        if (!pWrappedDevice)
            return;

        auto foundDevice=std::find(allDevicesCreated.begin(), allDevicesCreated.end(), pWrappedDevice);
        if (foundDevice!=allDevicesCreated.end())
            allDevicesCreated.erase(foundDevice);

        if (pWrappedDevice->pDev)
            pWrappedDevice->pDev->Release();
        pWrappedDevice->pDev=0;

        delete3(pWrappedDevice);
    }

    bool DInput_Device_Poll(DeviceAndObjects* pWrappedDevice)
    {
        if (pWrappedDevice && hwnd)
        {
            HRESULT ret=pWrappedDevice->pDev->Poll();
            if (ret==DIERR_INPUTLOST || ret==DIERR_NOTACQUIRED)
            {
                pWrappedDevice->pDev->Unacquire();
                pWrappedDevice->pDev->Acquire();

                ret=pWrappedDevice->pDev->Poll();
            }

            if (!(ret==DIERR_INPUTLOST || ret==DIERR_NOTACQUIRED))
            {
                //update the stored DIJOYSTATE2 copy
                ret=pWrappedDevice->pDev->GetDeviceState(sizeof(DIJOYSTATE2), pWrappedDevice->pCurrentStateData);
                if (ret==DI_OK)
                    return true;
            }
        }

        ClearDeviceState(pWrappedDevice->pCurrentStateData);
        return false;
    }
}

#endif //#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
