#include <Windows.h>
#include "Core/Application.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    Application app;
    if (!app.Initialize(hInstance))
    {
        return -1;
    }
    int result = app.Run();
    app.Shutdown();
    return result;
}