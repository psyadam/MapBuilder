//tests that the vr library works
//Luke Lenhart, 2013
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "geo/Geo.h"
#include "gfxsetup/GFXSetup.h"
#include "gfxsetup/GL.h"
#include "gfxsetup/Extensions.h"
#include "vr/Hmd.h"
#include <iostream>
#include <cmath>

#ifdef MPMA_COMPILE_VR
#ifdef DECLARE_TESTS_CODE
class VrRendering: public UnitTest
{
public:
    bool Run()
    {
        GFX::GraphicsSetup setup;
        setup.FullScreen=true;
        bool ok=GFX::SetupWindow(setup);
        if (!ok)
            return false;

        GFX::UpdateWindow();

        //render a bit
        MPMA::Timer timer;
        for (double total=0; total<10; total+=timer.Step())
        {
            const GFX::GraphicsSetup *state=GFX::GetWindowState();
            if (state==0)
                break;

            for (int renderPass=0; renderPass<VR::ActiveDevice->GetRenderingPasses(); ++renderPass)
            {
                VR::RenderPassParameters renderPassParameters=VR::ActiveDevice->BeginRenderPass(renderPass);

                //draw a triangle!
                glClearColor(1, 1, 1, 1);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                glEnable(GL_DEPTH_TEST);

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                GEO::Matrix4 matView=GEO::MatViewLookAt(GEO::Vector3(1.5, 1.5, 2), GEO::Vector3(0, 0, 1), GEO::Vector3(0, 0, 1));
                matView=renderPassParameters.ViewTransform*matView;
                glLoadMatrixf(matView.ToGL());

                float amt=0.25f*(float)total*2*3.14f;
                glBegin(GL_TRIANGLES);
                glColor3f(1, 0, 0); glVertex3f(0, 0, 2);
                glColor3f(0, 1, 0); glVertex3f(std::sin(amt), 1+std::cos(amt), 0);
                glColor3f(0, 0, 1); glVertex3f(std::sin(amt+3), 1+std::cos(amt+3), 0);

                glColor3f(0.5f, 0.5f, 0.5f); glVertex3f(-5, -5, 0); glVertex3f(-5, 5, 0); glVertex3f(5, 5, 0);
                glColor3f(0.5f, 0.5f, 0.5f); glVertex3f(-5, -5, 0); glVertex3f(5, -5, 0); glVertex3f(5, 5, 0);
                glEnd();
            }

            VR::ActiveDevice->FinalizeRenderPasses();
            GFX::UpdateWindow();
        }

        GFX::ShutdownWindow();

        return ok;
    }
};
#endif
DECLARE_TEST_CASE(VrRendering);
#endif //MPMA_COMPILE_VR
