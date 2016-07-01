#include "Detour.hpp"
#include <Windows.h>

Detour::Detour(uint8_t* source, const uint8_t* destination, size_t length)
{
	uint8_t* jump = new uint8_t[length + 5];

	DWORD protection;
	VirtualProtect(source, length, PAGE_EXECUTE_READWRITE, &protection);
	VirtualProtect(jump, length + 5, PAGE_EXECUTE_READWRITE, &protection);

	memcpy(jump, source, length);

	*(jump + length) = 0xE9;
	*reinterpret_cast<ptrdiff_t*>(jump + length + 1) = source - jump - 5;

	*source = 0xE9;
	*reinterpret_cast<ptrdiff_t*>(source + 1) = destination - source - 5;

	for (uint32_t i = 5; i < length; i++)
	{
		*(source + i) = 0x90;
	}

	m_length = length;
	m_source = source;
	m_original = jump;
}

Detour::~Detour()
{
	memcpy(m_source, m_original, m_length);
	delete[] m_original;
}

void* Detour::getOriginal()
{
	return m_original;
}