#include "framework.h"

DWORD WINAPI MainThread(LPVOID)
{
    Render::Init();
   // Notify::Push("Eclipse", "Loaded", Notify::Type::Info, 2.5f); --- its buggy thats why
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }
    if (reason == DLL_PROCESS_DETACH)
    {
        Render::Shutdown();
	}
    return TRUE;
}

