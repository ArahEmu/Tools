#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "WinsockDetours.h"
#include "Detour.h"
#include <memory>
#include <string>
#include <algorithm>
#include <WinSock2.h>
#include <MSWSock.h>

enum ServerType
{
    None,
    Portal,
    Auth,
    Unspecified
};

std::unique_ptr<Detour> g_getHostByNameDetour;
std::unique_ptr<Detour> g_connectExDetour;
bool g_detoursInitialized;
ServerType g_serverType;

hostent* WINAPI getHostByNameDetour(const char* name)
{
    if (!g_detoursInitialized) g_detoursInitialized = true;

    std::string hostname = name;
    std::transform(hostname.begin(), hostname.end(), hostname.begin(), tolower);

    if (!hostname.find("cligate"))
    {
        g_serverType = ServerType::Portal;
    }
    else if (!hostname.find("auth"))
    {
        g_serverType = ServerType::Auth;
    }
    else
    {
        g_serverType = ServerType::Unspecified;
    }

    return CALL_ORIGINAL(g_getHostByNameDetour, getHostByNameDetour, name);
}

BOOL WINAPI connectExDetour(SOCKET s, const sockaddr* name, int namelen, PVOID lpSendBuffer,
    DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped)
{
    if (g_detoursInitialized)
    {
        // Cast the sockaddr to an IPv4 sockaddr_in
        auto addr = reinterpret_cast<sockaddr_in*>(const_cast<sockaddr*>(name));

        if (g_serverType == ServerType::Portal)
        {
            // Redirect portal server.

            addr->sin_port = htons(6112);
        }
        else if (g_serverType == ServerType::Auth)
        {
            // Redirect authentication server.

            addr->sin_port = htons(7112);
            addr->sin_addr.s_addr = inet_addr("127.0.0.1");
        }
        else if (g_serverType == ServerType::None)
        {
            // gethostbyname hasn't been called prior to this function.
            // Redirect game server.

            addr->sin_port = htons(8112);
            addr->sin_addr.s_addr = inet_addr("127.0.0.1");
        }

        g_serverType = ServerType::None;
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
    g_getHostByNameDetour = std::make_unique<Detour>((uint8_t*)gethostbyname, (uint8_t*)getHostByNameDetour, 5);
    g_connectExDetour = std::make_unique<Detour>((uint8_t*)connectExFunc, (uint8_t*)connectExDetour, 5);
}