#pragma once

#include <memory>

#define CALL_ORIGINAL(detour, function, ...) static_cast<decltype(function)*>(detour->getOriginal())(__VA_ARGS__)

class Detour
{
private:
	size_t m_length;
	uint8_t* m_source;
	uint8_t* m_original;
public:
	Detour(uint8_t* source , const uint8_t* destination, size_t length);
	~Detour();
	void* getOriginal();
};
