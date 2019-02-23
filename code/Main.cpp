#include <mpma/Setup.h>
#include <mpma/base/DebugRouter.h>
#include <mpma/gfxsetup/GFXSetup.h>
#include <mpma/gfx/Texture.h>
#include <mpma/gfx/Shader.h>
#include <mpma/gfx/TextWriter.h>
#include <mpma/input/Unified.h>

#include <GL/glu.h>

#include <chrono>

//Platform independent entry point for the app.
void AppMain()
{
	srand((unsigned int)time(0));

	//write errors to both stdout and file
	MPMA::RouterOutputFile fileErrorReport("_error.txt");
	MPMA::RouterOutputStdout stdErrorReport;
	MPMA::ErrorReport().AddOutputMethod(&stdErrorReport);
	MPMA::ErrorReport().AddOutputMethod(&fileErrorReport);

	//start mpma
	MPMA::InitAndShutdown mpma;

	//init graphics
	GFX::GraphicsSetup gfxsetup;
	gfxsetup.FullScreen = false;
	gfxsetup.Resizable = true;
	gfxsetup.Width = 1200;
	gfxsetup.Height = 900;
	gfxsetup.Name = "Example App";

	if (!GFX::SetupWindow(gfxsetup))
		return;

	//load resources for example app
	GFX::Texture2D pawImage;
	pawImage.CreateFromFile("data\\example.png");

	//run example app loop
	auto lastFrameTime = std::chrono::high_resolution_clock::now();

	float x = 0.0f, y = 0.0f;

	const GFX::GraphicsSetup *state;
	while (state = GFX::GetWindowState())
	{
		GFX::UpdateWindow();

		//handle time
		auto now = std::chrono::high_resolution_clock::now();
		float timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count() / 1000000.0f;
		lastFrameTime = now;

		//handle input
		for (auto &a: INPUT::GetCurrentlyPressedAxes())
		{
			x += timePassed * a->GetXValue() * 100;
			y -= timePassed * a->GetYValue() * 100;
		}

		//set up opengl for 2d rendering
		glViewport(0, 0, state->Width, state->Height);

		glClearColor(0.1f, 0.1f, 0.1f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, state->Width, 0, state->Height);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//draw paw
		{
			GFX::AutoBindTexture autoTexture(pawImage, 0);
			glBegin(GL_TRIANGLE_FAN);
			glColor3f(1, 1, 1);
			glTexCoord2f(0, 0); glVertex3f(x + 0,   y + 0,   0);
			glTexCoord2f(1, 0); glVertex3f(x + 100, y + 0,   0);
			glTexCoord2f(1, 1); glVertex3f(x + 100, y + 100, 0);
			glTexCoord2f(0, 1); glVertex3f(x + 0,   y + 100, 0);
			glEnd();
		}

		//draw text
		GFX::TextWriter textWriter;
		textWriter.Text<<"Here is a paw.";
		textWriter.RenderTexture(300, 300);
	}
}
