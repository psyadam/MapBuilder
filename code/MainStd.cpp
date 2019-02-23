#ifndef WIN32
extern void AppMain();

int main(int cmdArgCount, char **cmdArgs)
{
    AppMain();
    return 0;
}
#endif