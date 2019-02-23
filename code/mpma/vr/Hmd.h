//!\file Hmd.h Wrapper for interacting with head-mounted displays.
//written by Luke Lenhart, 2013
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_VR

#include <list>
#include "../geo/Geo.h"

namespace VR
{
    //!Parameters that control setting up rendering for a scene
    struct SceneParameters
    {
        inline SceneParameters()
        {
            NearClippingPlane=1.0f;
            FarClippingPlane=1000.0f;
            MeterToWorldSpaceRatio=1.0f;
        }

        //!Near clipping plane
        float NearClippingPlane;
        //!Far clipping plane
        float FarClippingPlane;

        //!Ratio of "1 meter" to "1 world space unit used for rendering".  For example if 4 units of world were equivalent to 1 meter, this would be 0.25f.
        float MeterToWorldSpaceRatio;
    };

    //!Details needed to render the current scene pass
    struct RenderPassParameters
    {
        //!Projection transformation to use
        GEO::Matrix4 ProjectionTransform;
        //!View transform for head tracking and eye displacement, to be combined with the normal model-view transform
        GEO::Matrix4 ViewTransform;
        //!Viewport to use
        int ViewportLeft, ViewportBottom, ViewportWidth, ViewportHeight;
    };

    //!Represents a specific head-mounted display device.
    class HmdDevice
    {
    public:
        HmdDevice();
        virtual ~HmdDevice();

        //!The name of the display device corresponding to the hmd device or empty if there was a problem (such as it not being connected).
        inline const std::string& GetDisplayName() const {return displayName;}
        //!A friendly name of the device itself
        inline const std::string& GetFriendlyName() const {return friendlyName;}

        //!Actual x resolution of the display covering both eyes.  Note that this may be different than the active viewport.
        virtual int GetResolutionX() const=0;
        //!Actual y resolution of the display covering both eyes.  Note that this may be different than the active viewport.
        virtual int GetResolutionY() const=0;

        //!Resets the baseline view such that the currently-facing direction results in no rotation being applied from the hmd.
        virtual void ResetOrientation()=0;

        //!Returns the number of rendering passes required (1 for mono, 2 for stereoscopic)
        virtual int GetRenderingPasses()=0;

        //!Call this once for each rendering pass required, as obtained by GetRenderingPasses().  For example if GetRenderingPasses() returns 2, you will call it with pass=0 and render the scene, then call it with pass=1 and render the scene again.
        //!This method may change the render target to an internal texture if rendering for the active device requires a post-render transformation.
        //!This will also set the viewport, scissor, and projection matrix to values relavent to the eye that corresponds to the rendering pass.
        //!The returned information should be used when performing all rendering for the current pass.
        virtual const RenderPassParameters& BeginRenderPass(int pass, const SceneParameters &sceneParameters=SceneParameters())=0;

        //!Call this only once per frame, after calling BeginRenderPass one or more times.
        //!If the active device required post-render transformation, this may render an internal texture to the actual backbuffer.
        virtual void FinalizeRenderPasses()=0;

    protected:
        std::string friendlyName;
        std::string displayName;
        RenderPassParameters currentRenderPass;

        void ApplyCurrentRenderPassParameters();
        void DisableFancyStuff();
    };

    //!Set this to a device returned from VR::GetDevices to change the device that the methods in this file apply to.  If no hmd are present, this defaults to a normal mono full-viewport pseudo-device.
    extern HmdDevice *ActiveDevice;

    //!Returns a list of all detected devices.
    std::list<HmdDevice*> GetDevices();

    //!For the Oculus Rift HMD, this controls the scale of the offscreen texture used for distortion.  Higher means a larger render target but reduces pixellation near the center of the view.
    extern float RiftOffscreenTextureScale;
}

#endif