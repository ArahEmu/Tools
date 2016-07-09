#include "Detour.h"
#include <Windows.h>

#ifndef _WIN64
#define OPCODE_LENGTH 1
#else
#define OPCODE_LENGTH 2
#endif

#define POINTER_SIZE sizeof(uintptr_t)
#define JUMP_LENGTH (OPCODE_LENGTH + POINTER_SIZE + 2)

void writeJump(uint8_t* jump, const uint8_t* address, ptrdiff_t offset = 0)
{
    uint32_t writeIndex = 0;

#ifdef _WIN64
    jump[offset + writeIndex++] = 0x48;
#endif

    jump[offset + writeIndex++] = 0xBB;

    *reinterpret_cast<uintptr_t*>(jump + offset + writeIndex) = (uintptr_t)address;
    writeIndex += POINTER_SIZE;

    jump[offset + writeIndex++] = 0xFF;
    jump[offset + writeIndex] = 0xE3;
}

Detour::Detour(uint8_t* source, const uint8_t* destination, size_t length)
{
    uint8_t* jump = new uint8_t[length + JUMP_LENGTH];

    DWORD protection;
    VirtualProtect(source, length, PAGE_EXECUTE_READWRITE, &protection);
    VirtualProtect(jump, length + JUMP_LENGTH, PAGE_EXECUTE_READWRITE, &protection);

    memcpy(jump, source, length);

    writeJump(jump, source + length, length);
    writeJump(source, destination);

    for (uint32_t i = JUMP_LENGTH; i < length; i++)
    {
        source[i] = 0x90;
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