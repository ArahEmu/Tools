#pragma once

#define CALL_ORIGINAL(detour, function, ...) static_cast<decltype(function)*>(detour->getOriginal())(__VA_ARGS__)

class Detour
{
private:
	void* m_original;
public:
	Detour(void* sourceFunc, const void* destinationFunc, size_t length);
	void* getOriginal();
};
