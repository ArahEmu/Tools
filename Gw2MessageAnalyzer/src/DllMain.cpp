#include <Windows.h>
#include "Main.hpp"

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    DisableThreadLibraryCalls(instance);

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        main();
        break;
    }

    return TRUE;
}
