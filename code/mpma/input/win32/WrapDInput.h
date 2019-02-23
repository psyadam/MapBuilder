//Wrap DirectInput calls to avoid directly polluting the global namespace of any of our real code with windows.h spam, since a lot of their names collide with ours.
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../../Config.h"
#include "../../base/Types.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

#include <list>
#include <string>

struct IDirectInputDevice8A;

namespace INPUT_INTERNAL
{
    bool DInput_Init();

    struct DeviceInfo
    {
        std::string DeviceName;
        std::string InstanceName;
        std::string PersistantIdentifier;
    };

    struct DeviceObject
    {
        inline DeviceObject(): Offset(-1) {}

        int Offset;
        std::string Name;
    };

    struct DeviceAxisSet
    {
        DeviceObject X;
        DeviceObject Y;
        DeviceObject Z;
    };

    struct DeviceAndObjects
    {
        inline DeviceAndObjects(): pCurrentStateData(0) {}
        ~DeviceAndObjects();

        DeviceInfo Info;
        IDirectInputDevice8A *pDev;

        std::list<DeviceObject> Buttons;
        std::list<DeviceObject> Hats;
        std::list<DeviceAxisSet> Axes;

        char *pCurrentStateData;
    };

    std::list<DeviceInfo> DInput_EnumGameDevices();
    DeviceAndObjects* DInput_CreateDevice(const DeviceInfo &device);
    void DInput_Device_Release(DeviceAndObjects *pWrappedDevice);
    bool DInput_Device_Poll(DeviceAndObjects* pWrappedDevice);
}

#endif //#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
