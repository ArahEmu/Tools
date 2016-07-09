#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "WinsockDetours.h"
#include "Detour.h"
#include <memory>
#include <string>
#include <algorithm>
#include <map>
#include <WinSock2.h>
#include <MSWSock.h>

std::unique_ptr<Detour> g_getHostByNameDetour;
std::unique_ptr<Detour> g_connectExDetour;
std::map<uint32_t, std::string> g_hostnameMap;

hostent* PASCAL getHostByNameDetour(const char* name)
{
    std::string hostname = name;
    std::transform(hostname.begin(), hostname.end(), hostname.begin(), tolower);

    auto hosts = CALL_ORIGINAL(g_getHostByNameDetour, getHostByNameDetour, name);
    auto ips = reinterpret_cast<in_addr**>(hosts->h_addr_list);

    for (int i = 0; ips[i]; i++)
    {
        auto ip = ips[i]->s_addr;

        if (g_hostnameMap.find(ip) == g_hostnameMap.end())
        {
            g_hostnameMap.insert(std::pair<uint64_t, std::string>(ip, hostname));
        }
    }

    return hosts;
}

BOOL PASCAL connectExDetour(SOCKET s, const sockaddr* name, int namelen, PVOID lpSendBuffer,
    DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped)
{
    auto addr = reinterpret_cast<sockaddr_in*>(const_cast<sockaddr*>(name));

    if (g_hostnameMap.find(addr->sin_addr.s_addr) != g_hostnameMap.end())
    {
        // Retrieve hostname associated with the specfied IP.
        std::string hostname = g_hostnameMap[addr->sin_addr.s_addr];

        if (!hostname.find("cligate"))
        {
            // Redirect portal server.
            addr->sin_port = htons(6112);
        }
        else if (!hostname.find("auth"))
        {
            // Redirect auth server.
            addr->sin_port = htons(7112);
            addr->sin_addr.s_addr = inet_addr("127.0.0.1");
        }
    }
    else
    {
        // The hostname map does not contain the specified IP.
        // Redirect game server.
        addr->sin_port = htons(8112);
        addr->sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    return CALL_ORIGINAL(g_connectExDetour, connectExDetour, s, name, namelen, lpSendBuffer,
        dwSendDataLength, lpdwBytesSent, lpOverlapped);
}

void WinsockDetours::init()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    GUID connectExGuid = WSAID_CONNECTEX;
    LPFN_CONNECTEX connectExFunc;
    DWORD bytesReturned;

    // Get function pointer to ConnectEx
    WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &connectExGuid, sizeof(connectExGuid),
        &connectExFunc, sizeof(connectExFunc), &bytesReturned, NULL, NULL);

    closesocket(s);

    // Create detours
#ifndef _WIN64
    g_getHostByNameDetour = std::make_unique<Detour>(reinterpret_cast<uint8_t*>(gethostbyname),
        reinterpret_cast<uint8_t*>(getHostByNameDetour), 11);

    g_connectExDetour = std::make_unique<Detour>(reinterpret_cast<uint8_t*>(connectExFunc),
        reinterpret_cast<uint8_t*>(connectExDetour), 7);
#else
    g_getHostByNameDetour = std::make_unique<Detour>(reinterpret_cast<uint8_t*>(gethostbyname),
        reinterpret_cast<uint8_t*>(getHostByNameDetour), 19);

    g_connectExDetour = std::make_unique<Detour>(reinterpret_cast<uint8_t*>(connectExFunc),
        reinterpret_cast<uint8_t*>(connectExDetour), 15);
#endif
}