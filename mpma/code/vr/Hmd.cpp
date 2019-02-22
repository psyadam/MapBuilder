//Wrapper for interacting with head-mounted displays.
//written by Luke Lenhart, 2013
//See /docs/License.txt for details on how this code may be used.

#include "Hmd.h"

#ifdef MPMA_COMPILE_VR

#include <OVR.h>
#include "../Setup.h"
#include "../gfxsetup/GFXSetup.h"
#include "../gfxsetup/GL.h"
#include "../gfx/Framebuffer.h"
#include "../gfx/Shader.h"
#include "../gfx/Vertex.h"

namespace GFX_INTERNAL
{
    void AddWindowShutdownCallback(void (*callback)());
}

namespace
{
    std::string riftDistortionVsCode=
        "#version 120\n"
        "void main()"
        "{"
            "gl_TexCoord[0]=gl_MultiTexCoord0;"
            "gl_Position=ftransform();"
        "}";

    std::string riftDistortionFsCode=
        "#version 120\n"
        "uniform sampler2D textureScene;"
        "uniform vec2 LensCenter;"
        "uniform vec2 ScreenCenter;"
        "uniform vec2 Scale;"
        "uniform vec2 ScaleIn;"
        "uniform vec4 WarpParam;"
        "uniform vec4 ChromAbParam;"
        "void main()"
        "{"
            "vec2 theta=(gl_TexCoord[0].xy-LensCenter)*ScaleIn;"
            "float rSq=theta.x*theta.x+theta.y*theta.y;"
            "vec2 theta1=theta*(WarpParam.x+WarpParam.y*rSq+WarpParam.z*rSq*rSq+WarpParam.w*rSq*rSq*rSq);"
            "vec2 thetaBlue=theta1*(ChromAbParam.z+ChromAbParam.w*rSq);"
            "vec2 tcBlue=LensCenter+Scale*thetaBlue;"
            "if (!all(equal(clamp(tcBlue, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tcBlue)))"
            "   discard;"
            "float blue=texture2D(textureScene, tcBlue).b;"
            "vec2 tcGreen=LensCenter+Scale*theta1;"
            "float green=texture2D(textureScene, tcGreen).g;"
            "vec2 thetaRed=theta1*(ChromAbParam.x+ChromAbParam.y*rSq);"
            "vec2 tcRed=LensCenter+Scale*thetaRed;"
            "float red=texture2D(textureScene, tcRed).r;"
            "gl_FragColor=vec4(red, green, blue, 1.0);"
        "}";
}

namespace VR
{
    class OculusRiftHmd: public HmdDevice
    {
    public:
        OVR::HMDDevice *hmdDevice;
        OVR::SensorDevice *hmdSensor;
        OVR::SensorFusion *sensorFusion;
        OVR::HMDInfo hmdi;

        GEO::Matrix4 orientationTransform;

        GFX::Framebuffer renderTarget;
        GFX::VertexBuffer rtvb;
        GFX::ShaderProgram distortionShader;

        OculusRiftHmd(OVR::HMDDevice *hmdDevice, OVR::SensorDevice *hmdSensor)
        {
            this->hmdDevice=hmdDevice;
            this->hmdSensor=hmdSensor;

            sensorFusion=new OVR::SensorFusion();
            sensorFusion->AttachToSensor(hmdSensor);
            sensorFusion->SetYawCorrectionEnabled(true);

            //read params from the device
            OVR::DeviceInfo di;
            if (hmdDevice->GetDeviceInfo(&di))
            {
                if (di.Type==OVR::Device_HMD)
                {
                    if (hmdDevice->GetDeviceInfo(&hmdi))
                    {
                        friendlyName=hmdi.ProductName;
                        displayName=hmdi.DisplayDeviceName;
                    }
                }
            }

            orientationTransform.MakeIdentity();
        }

        virtual ~OculusRiftHmd()
        {
            if (sensorFusion)
                delete sensorFusion;
            sensorFusion=nullptr;

            if (hmdSensor)
                hmdSensor->Release();
            hmdSensor=nullptr;

            if (hmdDevice)
                hmdDevice->Release();
            hmdDevice=nullptr;
        }

        void FreeOpenGLStuff()
        {
            renderTarget.Free();
            rtvb.Free();
            distortionShader.Free();
        }

        inline int GetResolutionX() const
        {
            return hmdi.HResolution;
        }

        inline int GetResolutionY() const
        {
            return hmdi.VResolution;
        }

        void ResetOrientation()
        {
            sensorFusion->Reset();
            orientationTransform.MakeIdentity();
        }

        int GetRenderingPasses()
        {
            return 2;
        }

        const RenderPassParameters& BeginRenderPass(int pass, const SceneParameters &sceneParameters)
        {
            const GFX::GraphicsSetup *gfx=GFX::GetWindowState();
            if (!gfx || gfx->Width<=0 || gfx->Height<=0)
                return currentRenderPass;

            OVR::Util::Render::StereoConfig stereoConfig(OVR::Util::Render::Stereo_LeftRight_Multipass, OVR::Util::Render::Viewport(0, 0, gfx->Width, gfx->Height));
            stereoConfig.SetHMDInfo(hmdi);

            //set up rendering to a texture
            int idealRenderWidth=(int)(gfx->Width*RiftOffscreenTextureScale);
            int idealRenderHeight=(int)(gfx->Height*RiftOffscreenTextureScale);
            if (idealRenderWidth&1)
                ++idealRenderWidth;
            if (idealRenderHeight&1)
                ++idealRenderHeight;

            int renderTargetTextureWidth, renderTargetTextureHeight;
            renderTarget.GetCreatedTextureTarget().GetDimensions(renderTargetTextureWidth, renderTargetTextureHeight);
            if (!renderTarget || renderTargetTextureWidth!=idealRenderWidth || renderTargetTextureHeight!=idealRenderHeight)
            {
                GFX::TextureCreateParameters textureProperties;
                textureProperties.GenerateMipMaps=true;
                textureProperties.Dimensions.Width=idealRenderWidth;
                textureProperties.Dimensions.Height=idealRenderHeight;

                GFX::Texture2D textureTarget;
                textureTarget.Create(textureProperties);
                renderTarget.Create(textureTarget, true, false);
            }

            renderTarget.Bind();

            //set up transforms and such
            bool isLeft=(pass==0);

            if (pass==0) //poll a new hmd orientation on the first pass
            {
                OVR::Matrix4f orientationMatrix=sensorFusion->GetOrientation();
                orientationTransform=GEO::Matrix4((const float*)&orientationMatrix).Transpose();
            }

            const OVR::Util::Render::StereoEyeParams &eyeParams=stereoConfig.GetEyeRenderParams(isLeft?OVR::Util::Render::StereoEye_Left:OVR::Util::Render::StereoEye_Right);

            currentRenderPass.ProjectionTransform=GEO::Matrix4((const float*)&eyeParams.Projection);
            currentRenderPass.ViewTransform=GEO::MatTranslate(GEO::Vector3(hmdi.InterpupillaryDistance*sceneParameters.MeterToWorldSpaceRatio*0.5f*(isLeft?1.0f:-1.0f), 0, 0));
            currentRenderPass.ViewTransform*=orientationTransform;
            currentRenderPass.ViewportLeft=isLeft?0:idealRenderWidth/2;
            currentRenderPass.ViewportBottom=0;
            currentRenderPass.ViewportWidth=idealRenderWidth/2;
            currentRenderPass.ViewportHeight=idealRenderHeight;

            ApplyCurrentRenderPassParameters();
            return currentRenderPass;
        }

        void FinalizeRenderPasses()
        {
            renderTarget.Unbind();

            const GFX::GraphicsSetup *gfx=GFX::GetWindowState();
            if (!gfx || gfx->Width<=0 || gfx->Height<=0)
                return;

            OVR::Util::Render::StereoConfig stereoConfig(OVR::Util::Render::Stereo_LeftRight_Multipass, OVR::Util::Render::Viewport(0, 0, gfx->Width, gfx->Height));
            stereoConfig.SetHMDInfo(hmdi);
            OVR::Util::Render::DistortionConfig distortion=stereoConfig.GetDistortionConfig();

            glViewport(0, 0, gfx->Width, gfx->Height);
            glScissor(0, 0, gfx->Width, gfx->Height);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int pass=0; pass<2; ++pass)
            {
                bool isLeftEye=(pass==0);

                //wtf math
                float w=0.5f;
                float h=1.0f;
                float x=isLeftEye?0.0f:0.5f;
                float y=0.0f;

                float as=(float)gfx->Width/2/gfx->Height;
                
                float centerOffsetMultiplier=isLeftEye?1.0f:-1.0f;
                float lensCenterX=x + (w + centerOffsetMultiplier*distortion.XCenterOffset * 0.5f)*0.5f;
                float lensCenterY=y + h*0.5f;

                float screenCenterX=x + w*0.5f;
                float screenCenterY=y + h*0.5f;

                float scaleFactor=1.0f/distortion.Scale;
                float scaleX=(w/2) * scaleFactor;
                float scaleY=(h/2) * scaleFactor * as;

                float scaleInX=2/w;
                float scaleInY=(2/h) / as;

                //load distortion shader if needed
                if (!distortionShader)
                {
                    GFX::ShaderCode vs;
                    vs.Compile(GL_VERTEX_SHADER, riftDistortionVsCode);
                    GFX::ShaderCode fs;
                    vs.Compile(GL_FRAGMENT_SHADER, riftDistortionFsCode);

                    std::vector<GFX::ShaderCode> sc;
                    sc.emplace_back(std::move(vs));
                    sc.emplace_back(std::move(fs));
                    distortionShader.CreateAndLink(sc);
                }

                //render texture to the main framebuffer
                DisableFancyStuff();

                GFX::AutoBindTexture rtBinder(renderTarget.GetCreatedTextureTarget());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                GFX::AutoBindShaderProgram dsBinder(distortionShader);
                distortionShader.FindVariable("textureScene").SetTextureStage(0);
                distortionShader.FindVariable("LensCenter").SetFloat2(lensCenterX, lensCenterY);
                distortionShader.FindVariable("ScreenCenter").SetFloat2(screenCenterX, screenCenterY);
                distortionShader.FindVariable("Scale").SetFloat2(scaleX, scaleY);
                distortionShader.FindVariable("ScaleIn").SetFloat2(scaleInX, scaleInY);
                distortionShader.FindVariable("WarpParam").SetFloat4(distortion.K[0], distortion.K[1], distortion.K[2], distortion.K[3]);
                distortionShader.FindVariable("ChromAbParam").SetFloat4(distortion.ChromaticAberration[0], distortion.ChromaticAberration[1], distortion.ChromaticAberration[2], distortion.ChromaticAberration[3]);

                if (!rtvb)
                {
                    GFX::InterleavedVertexFormat vbFormat;
                    vbFormat.Usage=GL_STATIC_DRAW;
                    vbFormat<<GFX::VertexComponentFormat(GFX::VertexPosition,           GL_FLOAT, 2);
                    vbFormat<<GFX::VertexComponentFormat(GFX::VertexTextureCoordinate0, GL_FLOAT, 2);

                    struct VertStruct
                    {
                        float pos[2];
                        float tex[2];
                    };
                    VertStruct verts[8]=
                    {
                        {{-1, -1}, {0,    0}},
                        {{0,  -1}, {0.5f, 0}},
                        {{0,   1}, {0.5f, 1}},
                        {{-1,  1}, {0,    1}},

                        {{0, -1}, {0.5f, 0}},
                        {{1, -1}, {1,    0}},
                        {{1,  1}, {1,    1}},
                        {{0,  1}, {0.5f, 1}}
                    };

                    rtvb.LoadInterleaved(vbFormat, verts, 8);
                }

                GFX::AutoBindVertexBuffer vbBinder(rtvb);
                glDrawArrays(GL_TRIANGLE_FAN, isLeftEye?0:4, 4);
            }
        }
    };

    //

    class MonoNonHmd: public HmdDevice
    {
    public:
        MonoNonHmd()
        {
            friendlyName="Default Mono Display";
            displayName="";
        }

        virtual ~MonoNonHmd()
        {
        }

        inline int GetResolutionX() const
        {
            const GFX::GraphicsSetup *gfx=GFX::GetWindowState();
            if (!gfx || gfx->Width<=0 || gfx->Height<=0)
                return 640;
            return gfx->Width;
        }

        inline int GetResolutionY() const
        {
            const GFX::GraphicsSetup *gfx=GFX::GetWindowState();
            if (!gfx || gfx->Width<=0 || gfx->Height<=0)
                return 480;
            return gfx->Height;
        }

        void ResetOrientation()
        {
        }

        int GetRenderingPasses()
        {
            return 1;
        }

        const RenderPassParameters& BeginRenderPass(int pass, const SceneParameters &sceneParameters)
        {
            const GFX::GraphicsSetup *gfx=GFX::GetWindowState();
            if (!gfx || gfx->Width<=0 || gfx->Height<=0)
                return currentRenderPass;

            currentRenderPass.ProjectionTransform=GEO::MatProjectionFoV(GEO::PI/4, (float)gfx->Height/gfx->Width, sceneParameters.NearClippingPlane, sceneParameters.FarClippingPlane);
            currentRenderPass.ViewportLeft=0;
            currentRenderPass.ViewportBottom=0;
            currentRenderPass.ViewportWidth=gfx->Width;
            currentRenderPass.ViewportHeight=gfx->Height;

            ApplyCurrentRenderPassParameters();
            return currentRenderPass;
        }

        void FinalizeRenderPasses()
        {
            DisableFancyStuff();
        }
    };
}

namespace
{
    std::list<VR::HmdDevice*> allDevices;

    OVR::DeviceManager *oculusRiftHmdManager=nullptr;

    void InitHmd()
    {
        //add rift devices
        OVR::System::Init();
        if (OVR::System::IsInitialized())
        {
            oculusRiftHmdManager=OVR::DeviceManager::Create();
            if (oculusRiftHmdManager)
            {
                OVR::DeviceEnumerator<OVR::HMDDevice> de=oculusRiftHmdManager->EnumerateDevices<OVR::HMDDevice>(true);
                while (de.GetType()!=OVR::Device_None)
                {
                    OVR::HMDDevice *hmdDevice=de.CreateDevice();
                    if (hmdDevice)
                    {
                        OVR::SensorDevice *hmdSensor=hmdDevice->GetSensor();
                        if (!hmdSensor)
                        {
                            hmdDevice->Release();
                            hmdDevice=nullptr;
                        }
                        else //it's looks good, add it
                        {
                            allDevices.push_back(new VR::OculusRiftHmd(hmdDevice, hmdSensor));
                            if (!allDevices.back()->GetResolutionX()) //must have been a problem, take it back out
                            {
                                delete allDevices.back();
                                allDevices.pop_back();
                            }
                        }
                    }

                    if (!de.Next())
                        break;
                }
            }
        }

        //add mono device
        allDevices.push_back(new VR::MonoNonHmd());

#ifdef VR_DEFAULT_TO_HMD_DISPLAY
            VR::ActiveDevice=allDevices.front();
#else
            VR::ActiveDevice=allDevices.back();
#endif
    }

    void ShutdownHmd()
    {
        VR::ActiveDevice=nullptr;

        for (auto i=allDevices.begin(); i!=allDevices.end(); ++i)
        {
            delete *i;
            *i=nullptr;
        }
        allDevices.clear();

        if (oculusRiftHmdManager)
        {
            oculusRiftHmdManager->Release();
            oculusRiftHmdManager=nullptr;

            OVR::System::Destroy();
        }
    }

    void WindowShutdownCallback()
    {
        //need to clean up any OpenGL resources before the window goes away
        for (auto i=allDevices.begin(); i!=allDevices.end(); ++i)
        {
            VR::OculusRiftHmd *riftHmd=dynamic_cast<VR::OculusRiftHmd*>(*i);
            if (riftHmd)
                riftHmd->FreeOpenGLStuff();
        }
    }

    class HookHmdSetup
    {
    public:
        HookHmdSetup()
        {
            GFX_INTERNAL::AddWindowShutdownCallback(WindowShutdownCallback);
            MPMA::Internal_AddInitCallback(InitHmd, 5000);
            MPMA::Internal_AddShutdownCallback(ShutdownHmd, 5000);
        }
    } autoHmdHook;   
}

namespace VR
{
    HmdDevice::HmdDevice()
    {
        memset(&currentRenderPass, 0, sizeof(currentRenderPass));
        currentRenderPass.ProjectionTransform.MakeIdentity();
        currentRenderPass.ViewTransform.MakeIdentity();
    }

    HmdDevice::~HmdDevice()
    {
    }

    void HmdDevice::ApplyCurrentRenderPassParameters()
    {
        glViewport(currentRenderPass.ViewportLeft, currentRenderPass.ViewportBottom, currentRenderPass.ViewportWidth, currentRenderPass.ViewportHeight);
        glScissor(currentRenderPass.ViewportLeft, currentRenderPass.ViewportBottom, currentRenderPass.ViewportWidth, currentRenderPass.ViewportHeight);
        glEnable(GL_SCISSOR_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(currentRenderPass.ProjectionTransform.ToGL());
    }

    //technically this is only needed for rendering the rift's offscreen texture, but to make sure mono is consistent with hmd mode, we'll us it for everything
    void HmdDevice::DisableFancyStuff()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    // --

    HmdDevice *ActiveDevice=nullptr;

    std::list<HmdDevice*> GetDevices()
    {
        return allDevices;
    }

    float RiftOffscreenTextureScale=1.25f;
}

bool mpmaForceReferenceToHmdCPP=false;

#endif
