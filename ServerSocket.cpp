#include "ServerSocket.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wS2_32.lib")//todo necessary?

#include <iostream>
#include <thread>

#define MAX_CONNECTION_ATTEMPTS -1
#define CONNECTION_ATTEMPT_TIMEOUT 0

ServerSocket::ServerSocket(int port)
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << stderr << "WSAStartup machine broke " << iResult << std::endl;
        wsaStarted = false;
        //todo make sure this never happens lol
    }
    else
    {
        wsaStarted = true;//todo change
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));//is this necessary
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        struct addrinfo* result = 0;
        iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
        if (iResult != 0)
        {
            std::cout << "getaddrinfo failed with error " << iResult << "\n";
        }

        sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            std::cout << "error opening socket" << WSAGetLastError();
            freeaddrinfo(result);
        }

        iResult = bind(sock, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            std::cout << "couldnt bind to port " << port << ", got " << WSAGetLastError();
            freeaddrinfo(result);
            //make sure sock is invalid_socket so it is closed
        }

        freeaddrinfo(result);
    }
    std::cout << "asd";
}

ServerSocket::~ServerSocket()
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

Socket ServerSocket::listen()
{
    std::cout << "tryna listen";
    int iResult = ::listen(sock, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "listen failed with " << WSAGetLastError();
        //todo
    }

    SOCKET clientSock = INVALID_SOCKET;
    clientSock = accept(sock, NULL, NULL);
    if (clientSock == INVALID_SOCKET)
    {
        std::cout << "couldnt accept connection " << WSAGetLastError();
        //todo
    }
    return Socket(clientSock);
}
