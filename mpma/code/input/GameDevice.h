//!\file GameDevice.h Input for joysticks and gamepads.  Note that all input relies on the window being set up first.
//Luke Lenhart, 2011
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)

#include "../base/Types.h"

#include <string>
#include <list>
#include <vector>

namespace INPUT
{
    namespace GAME
    {
        class Device;
    }
}

namespace INPUT_INTERNAL
{
    struct PlatformDeviceData;
    struct PlatformObjectInformation;
    struct PlatformAxesInformation;

#if defined(_WIN32) || defined(_WIN64)
    //windows treats hats differently, so we need to track them internally separately
    class Hat
    {
    public:
        const std::string& GetName() const;
        const std::string& GetXName() const;
        const std::string& GetYName() const;
        const std::string& GetZName() const;
        bool IsPressed() const;
        float GetRotation() const; //0 to 2*PI

        Hat(INPUT_INTERNAL::PlatformObjectInformation *objectInfo);
        Hat(Hat &&o);
        ~Hat();

    private:
        INPUT_INTERNAL::PlatformObjectInformation *info;
        std::string nameX;
        std::string nameY;
        std::string nameZ;
        bool isPressed;
        float value;

        Hat(const Hat &o); //not implemented
        void SetState(char *rawdata);

        friend class INPUT::GAME::Device;
    };
#endif
}

namespace INPUT
{
    //!Game devices
    namespace GAME
    {
        //!Represents a set of related axes on a device.
        class AxisSet
        {
        public:
            //!Returns a string representing the friendly name of all of the axes in this set.
            const std::string GetCombinedName() const;
            //!Returns an string that can be used to uniquely identify the axis set again after the program has exited.
            const std::string& GetPersistentIdentifier() const;

            bool IsXPresent() const; //!<Returns true if this axis set contains an x axis
            bool IsYPresent() const; //!<Returns true if this axis set contains an y axis
            bool IsZPresent() const; //!<Returns true if this axis set contains an z axis
            const std::string& GetXName() const; //!<Returns the name of the x axis
            const std::string& GetYName() const; //!<Returns the name of the y axis
            const std::string& GetZName() const; //!<Returns the name of the z axis
            float GetXValue() const; //!<Returns the current value of the x axis, from -1 to 1
            float GetYValue() const; //!<Returns the current value of the y axis, from -1 to 1
            float GetZValue() const; //!<Returns the current value of the z axis, from -1 to 1

            void SetXDeadZone(float amount); //!<Sets the amount of dead zone for the x axis, from 0 to 1
            void SetYDeadZone(float amount); //!<Sets the amount of dead zone for the y axis, from 0 to 1
            void SetZDeadZone(float amount); //!<Sets the amount of dead zone for the z axis, from 0 to 1
            float GetXDeadZone() const; //!<Gets the current dead zone for the x axis, from 0 to 1
            float GetYDeadZone() const; //!<Gets the current dead zone for the y axis, from 0 to 1
            float GetZDeadZone() const; //!<Gets the current dead zone for the z axis, from 0 to 1

            //!Returns the device this button belongs to.
            const Device* GetDevice() const;

            AxisSet(INPUT_INTERNAL::PlatformAxesInformation *objectInfo, Device *device);
            AxisSet(AxisSet &&o);
            ~AxisSet();

        private:
            std::string persistentIdentifier;
            INPUT_INTERNAL::PlatformAxesInformation *info;
            float xValue, yValue, zValue;
            float xDead, yDead, zDead;
            Device *parentDevice;

            AxisSet(const AxisSet &o); //not implemented
            void SetState(char *rawdata);
            void NormalizeMaximum();

            float ApplyDeadZone(float val, float dead);

            friend class Device;
        };

        //!Represents a button on a device.
        class Button
        {
        public:
            //!Returns the friendly name of the button.
            const std::string& GetName() const;
            //!Returns an string that can be used to uniquely identify the button again after the program has exited.
            const std::string& GetPersistentIdentifier() const;
            //!Returns whether the button is currently pressed.
            bool IsPressed() const;
            //!Returns the device this button belongs to.
            const Device* GetDevice() const;

            //the application is not expected to use these directly
            Button(INPUT_INTERNAL::PlatformObjectInformation *objectInfo, Device *device);
            Button(Button &&o);
            ~Button();

        private:
            std::string persistentIdentifier;
            INPUT_INTERNAL::PlatformObjectInformation *info;
            bool isPressed;
            Device *parentDevice;

            Button(const Button &o); //not implemented
            void SetState(char *rawdata);

            friend class Device;
        };

        //!Represents a game device, such as a joystick or gamepad.
        class Device
        {
        public:
            //!Returns the friendly name of the device.
            const std::string& GetName() const;
            //!Returns an string that can be used to uniquely identify the device again after the program has exited.
            const std::string& GetPersistentIdentifier() const;

            //Updates the data in all axes and buttons.  The application should not call this directly but should instead call PollAllDevices, which will also update global pressed states.
            void Poll();

            //!Returns all of the axes of a device.
            const std::list<AxisSet>& GetAxes() const;
            //!Returns all of the axes of a device.
            std::list<AxisSet>& GetAxes();
            //!Returns all of the buttons of a device.
            const std::list<Button>& GetButtons() const;
            //!Returns all of the buttons of a device.
            std::list<Button>& GetButtons();

            //the application is not expected to use these directly
            Device(INPUT_INTERNAL::PlatformDeviceData *platformData, const std::string &name, const std::string &persistentId);
            Device(Device &&o);
            ~Device();

#if defined(linux)
            //linux joy api doesn't provide relations between a device's axes.  MPMA will autodetect based on a known list, but this API allows apps to override the default layout for other devices
            struct AxisSetMap
            {
                inline AxisSetMap(): XNumber(-1), YNumber(-1), ZNumber(-1) {}

                std::string Name;
                int XNumber;
                int YNumber;
                int ZNumber;
            };

            //retrieves the current axis map
            const std::vector<AxisSetMap>& GetAxitMaps() const;

            //replaces the current axes map.  note that this frees all existing Axes on the device, so callers should beware any pointers they might have.
            void SetAxitMaps(const std::vector<AxisSetMap> &maps);

            //retrieves the number of device axes
            int GetAxisCount() const;
#endif

        private:
            std::string friendlyName;
            std::string persistentIdentifier;
            INPUT_INTERNAL::PlatformDeviceData *data;
            std::list<AxisSet> axes;
            std::list<Button> buttons;

#if defined(_WIN32) || defined(_WIN64)
            //hats are tracked internally separately on windows
            std::list<INPUT_INTERNAL::Hat> hats;
#elif defined(linux)
            std::vector<AxisSetMap> axisMaps;
#endif

            Device(const Device &o); //do not use
        };


        //!Returns all devices.  Note that the pointers to the axes/buttons of the devices are always valid and constant while MPMA is in an initialized state.
        std::list<Device>& GetDevices();

        //!Enables or disables auto-polling of all devices each frame.  The default is to auto-poll, in which case it is never neccesary to manually call PollAllDevices.
        void EnableAutoPoll(bool enable);

        //!Updates the data for all axes in buttons in all Devices.  This also updates the list of currently and newly pressed buttons and axes.
        void PollAllDevices();

        //!Returns a list of buttons that are currently pressed.
        const std::vector<const Button*>& GetCurrentlyPressedButtons();

        //!Returns a list of buttons that were newly pressed since the last call to PollAllDevices.
        const std::vector<const Button*>& GetNewlyPressedButtons();

        //!Returns a list of axis sets that are currently pressed.
        const std::vector<const AxisSet*>& GetCurrentlyPressedAxes();

        //!Returns a list of axis sets that were newly pressed since the last call to PollAllDevices.
        const std::vector<const AxisSet*>& GetNewlyPressedAxes();

        //!Finds a device given its persistent identifier.  Returns 0 if not found.
        Device* FindDevice(const std::string &identifierToFind);

        //!Finds a button given its persistent identifier.  Returns 0 if not found.
        Button* FindButton(const std::string &identifierToFind);

        //!Finds an axis set given its persistent identifier.  Returns 0 if not found.
        AxisSet* FindAxisSet(const std::string &identifierToFind);

        //!Clears the state for all keys and buttons that have occured since the last poll.
        void ClearState();
    }
}

#endif //#if defined(MPMA_COMPILE_INPUT) && defined(ENABLE_GAME_DEVICE_INPUT)
