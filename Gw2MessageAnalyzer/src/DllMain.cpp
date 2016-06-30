#include <Windows.h>
#include "Library.hpp"

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	DisableThreadLibraryCalls(instance);

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		load();
		break;
	case DLL_PROCESS_DETACH:
		unload();
		break;
	}

	return TRUE;
}
