#ifdef WIN32
#include <windows.h>
#include <mpma\base\File.h>

extern void AppMain();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    AppMain();
    return 0;
}
#endif