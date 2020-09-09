#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wS2_32.lib")

#define HOST "localhost"
#define PORT "69"
#define BUFFER_LENGTH 512
int main()
{
    std::cout << "Starting StrawberryCop...\n";
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << stderr << "WSAStartup machine broke " << iResult << std::endl;
        return 1;
    }
    struct addrinfo *result = 0, *ptr = 0, hints;

    memset(&hints, 0, sizeof(hints));//is this necessary
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(HOST, PORT, &hints, &result);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo failed " << iResult << std::endl;
        WSACleanup();
        return 1;
    }
    
    SOCKET sock = INVALID_SOCKET;
    for (ptr = result; ptr != 0; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            std::cout << "failed with " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        iResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult != 0)
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
            std::cout << "trying again" << std::endl;
            continue;
        }
        break;
    }
    freeaddrinfo(result);

    const char *buffer = "why hello there";
    iResult = send(sock, buffer, (int)strlen(buffer), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send failed " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    iResult = shutdown(sock, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "shutdown failed " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char recvBuffer[BUFFER_LENGTH];
    int recvBufferLength = BUFFER_LENGTH;
    do {
        iResult = recv(sock, recvBuffer, BUFFER_LENGTH, 0);
        if (iResult > 0)
        {
            std::cout << "got " << iResult << std::endl;
        }
        else if (iResult == 0)
        {
            std::cout << "conneciton closd" << std::endl;
        }
        else 
        {
            std::cout << "recv failed" << WSAGetLastError() << std::endl;
        }
    } while (iResult > 0);

    std::cout << "all done";
    
    closesocket(sock);
    WSACleanup();
    return 0;
}
