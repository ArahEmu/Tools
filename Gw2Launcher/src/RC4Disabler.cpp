#include "RC4Disabler.h"
#include <stdint.h>
#include <Windows.h>

void RC4Disabler::init()
{
    HMODULE hModule = GetModuleHandleA(NULL);

    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery((LPCVOID)(hModule + 0x1000), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

    uint8_t* start = (uint8_t*)mbi.BaseAddress;
    uint8_t* end = start + mbi.RegionSize;

    uint8_t xorPattern[] = { 0x32, 0x44, 0x19, 0xFF };

    while (start < end)
    {
        if (!memcmp(start, xorPattern, sizeof(xorPattern)))
        {
            DWORD protection;
            VirtualProtect(start, 1, PAGE_EXECUTE_READWRITE, &protection);
            *start = 0x8A;
        }

        start++;
    }
}