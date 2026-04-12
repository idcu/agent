#include "idcu/os.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

static int g_winsock_initialized = 0;

int idcu_sock_init(void)
{
    if (g_winsock_initialized) {
        return 0;
    }
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return -1;
    }
    g_winsock_initialized = 1;
    return 0;
}

void idcu_sock_cleanup(void)
{
    if (g_winsock_initialized) {
        WSACleanup();
        g_winsock_initialized = 0;
    }
}
