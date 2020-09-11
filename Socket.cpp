#include "Socket.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wS2_32.lib")//todo necessary?

#include <iostream>
#include <thread>

#define MAX_CONNECTION_ATTEMPTS -1
#define CONNECTION_ATTEMPT_TIMEOUT 0

Socket::Socket(std::string addr, int port)
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << stderr << "WSAStartup machine broke " << iResult << std::endl;
        wsaStarted = false;
        //todo make sure this never happens lol
    }
    else
    {
        wsaStarted = true;
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));//is this necessary
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        iResult = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &result);
        if (iResult != 0)
        {
            std::cout << "getaddrinfo failed with error " << iResult << "\n";
        }
    }
}

Socket::~Socket()
{
    //todo make sure this works
    if (wsaStarted)
    {
        WSACleanup();
    }
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);//todo returns error
    }
}

int Socket::connect()
{
    sock = INVALID_SOCKET;
    int attempts = 0;
    struct addrinfo* ptr = result;
    while (MAX_CONNECTION_ATTEMPTS >= 0 ? attempts < MAX_CONNECTION_ATTEMPTS : true)//attempts to zero is allowed i guess, set to -1 for infinite attempts
    {
        attempts++;
        sock = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            std::cout << "failed to make socket with error " << WSAGetLastError();
            return 1;
        }
        std::cout << "tryna cop a connection to " << inet_ntoa((((struct sockaddr_in*)ptr->ai_addr)->sin_addr)) << "\n";//todo deprecated
        int iResult = ::connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
            std::cout << "trying connection again\n";
            ptr = ptr->ai_next;
            if (ptr == NULL)
            {
                ptr = result;
            }
            if (CONNECTION_ATTEMPT_TIMEOUT > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(CONNECTION_ATTEMPT_TIMEOUT));
            }
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    std::cout << "probably connected to server\n";//todo timer
    return 0;
}
int Socket::send(char* buffer, int bufferLength)
{
    int iResult = ::send(sock, buffer, bufferLength, 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send failed " << WSAGetLastError();
        closesocket(sock);
        return 1;
    }
    return 0;
}

int Socket::recv(char* buffer, int bufferLength)
{
    int iResult = ::recv(sock, buffer, bufferLength, 0);
    if (iResult > 0)
    {

    }
    else if (iResult == 0)
    {
        std::cout << "connection closed\n";
    }
    else
    {
        std::cout << "error recv " << WSAGetLastError() << "\n";
    }
    return iResult;
}
