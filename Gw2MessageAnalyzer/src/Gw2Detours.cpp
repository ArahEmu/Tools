#include "Gw2Detours.hpp"
#include "Detour.hpp"
#include <memory>
#include <Windows.h>

std::unique_ptr<Detour> m_updateSendBufferDetour;
std::unique_ptr<Detour> m_copyRecvBufferDetour;

void __fastcall updateSendBuffer(void* msgCon, uint32_t edx)
{
	CALL_ORIGINAL(m_updateSendBufferDetour, updateSendBuffer, msgCon, edx);
}

int __fastcall copyRecvBuffer(void* msgCon, void* unk, uint8_t** rawDataBuffer, size_t* rawBytes)
{
	return CALL_ORIGINAL(m_copyRecvBufferDetour, copyRecvBuffer, msgCon, unk, rawDataBuffer, rawBytes);
}

void Gw2Detours::init()
{
	HMODULE hModule = GetModuleHandleA(NULL);

	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPCVOID)(hModule + 0x1000), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	uint8_t* start = (uint8_t*)mbi.BaseAddress;
	uint8_t* end = start + mbi.RegionSize;

	uint8_t updateSendBufferPattern[] = { 0x8B, 0xC4, 0x8D, 0x9E, 0xD4, 0x02, 0x00, 0x00, 0x50, 0x53, 0x57, 0x8D, 0x8E, 0xC0, 0x01, 0x00 };
	uint8_t copyRecvBufferPattern[] = { 0x8B, 0x7D, 0x08, 0x8B, 0xF2, 0x89, 0x4D, 0xFC, 0x85, 0xFF };

	while (start < end)
	{
		if (!memcmp(start, updateSendBufferPattern, sizeof(updateSendBufferPattern)))
		{
			m_updateSendBufferDetour = std::make_unique<Detour>(start - 82, (uint8_t*)updateSendBuffer, 6);
		}
		else if (!memcmp(start, copyRecvBufferPattern, sizeof(copyRecvBufferPattern)))
		{
			m_copyRecvBufferDetour = std::make_unique<Detour>(start - 6, (uint8_t*)copyRecvBuffer, 5);
		}

		start++;
	}
}
