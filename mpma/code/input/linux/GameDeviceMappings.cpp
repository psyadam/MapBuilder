//Linux game device axis mapping list for devices whose axes are in weird orders
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#include "../GameDevice.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

namespace INPUT_INTERNAL
{
    std::vector<INPUT::GAME::Device::AxisSetMap> FindDefaultAxisMappings(const std::string &deviceName)
    {
        std::vector<INPUT::GAME::Device::AxisSetMap> mappings;

        if (deviceName=="Microsoft X-Box 360 pad")
        {
            INPUT::GAME::Device::AxisSetMap leftStick;
            leftStick.Name="Left Stick";
            leftStick.XNumber=0;
            leftStick.YNumber=1;
            mappings.push_back(leftStick);

            INPUT::GAME::Device::AxisSetMap rightStick;
            rightStick.Name="Right Stick";
            rightStick.XNumber=3;
            rightStick.YNumber=4;
            mappings.push_back(rightStick);

            INPUT::GAME::Device::AxisSetMap leftTrigger;
            leftTrigger.Name="Left Trigger";
            leftTrigger.ZNumber=2;
            mappings.push_back(leftTrigger);

            INPUT::GAME::Device::AxisSetMap rightTrigger;
            rightTrigger.Name="Right Trigger";
            rightTrigger.ZNumber=5;
            mappings.push_back(rightTrigger);
        }

        return mappings;
    }
}

#endif //defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)