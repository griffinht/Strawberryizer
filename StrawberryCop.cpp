#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wS2_32.lib")

#define HOST "localhost"
#define PORT "69"
#define MAX_CONNECTION_ATTEMPTS -1
#define CONNECTION_ATTEMPT_TIMEOUT 0

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
        std::cout << "getaddrinfo failed with error " << iResult << "\n";
        WSACleanup();
        return 1;
    }
    
    SOCKET sock = INVALID_SOCKET;
    int attempts = 0;
    ptr = result;
    while (MAX_CONNECTION_ATTEMPTS >= 0 ? attempts < MAX_CONNECTION_ATTEMPTS : true)//attempts to zero is allowed i guess, set to -1 for infinite attempts
    {
        attempts++;
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            std::cout << "failed to make socket with error " << WSAGetLastError();
            WSACleanup();
            return 1;
        }
        std::cout << "tryna cop a connection to " << inet_ntoa((((struct sockaddr_in*)ptr->ai_addr)->sin_addr)) << "\n";
        iResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
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
    std::ifstream ifs;
    ifs.open("testImages/david500.jpg", std::ios::in | std::ios::binary | std::ios::ate);//todo error checking
    int length = ifs.tellg();
    ifs.seekg(0);
    int bufferLength = 4 + length;
    char *buffer = new char[bufferLength];
    memcpy(buffer, &length, 4);
    ifs.read(buffer + 4, length);
    ifs.close();
    iResult = send(sock, buffer, bufferLength, 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send failed " << WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "sent " << iResult << " bytes (file is " << length << " bytes long + 4 bytes overhead\n";
    }
    delete[] buffer;

    iResult = shutdown(sock, SD_SEND);//only shuts down send operations
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "shutdown failed " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char recvLengthBuffer[4];
    iResult = recv(sock, recvLengthBuffer, 4, 0);
    if (iResult > 0)
    {
        std::cout << "got " << iResult << "\n";
        if (iResult != 4)
        {
            std::cout << "didnt get 4 bytes for length\n";
            //idk or something todo???
        }
        else
        {
            int recvLength = 0;
            memcpy(&recvLength, recvLengthBuffer, 4);
            std::cout << (int) recvLengthBuffer[0] << ", " << (int)recvLengthBuffer[1] << ", " << (int)recvLengthBuffer[2] << ", " << (int)recvLengthBuffer[3] << ",,, " << (int)*recvLengthBuffer << "\n";
            char *recvBuffer = new char[recvLength];//todo bounds checking in case of corruption or something
            iResult = recv(sock, recvBuffer, recvLength, 0);
            if (iResult > 0)
            {
                std::cout << "got " << iResult << "\n";
                if (iResult != recvLength)
                {
                    std::cout << "didn't get enough bytes (expected " << recvLength << ")\n";
                }
                else
                {
                    std::cout << "got it\n";
                    std::ofstream ofs;
                    ofs.open("testImages/recieve.jpg", std::ios::out | std::ios::binary | std::ios::trunc);//todo error
                    ofs.write(recvBuffer, recvLength);
                    ofs.close();
                    std::cout << "wrote it to file\n";
                }
            }
            else if (iResult == 0)
            {
                std::cout << "connection closed\n";
            }
            else
            {
                std::cout << "error recv " << WSAGetLastError() << "\n";
            }
            delete[] recvBuffer;
        }
    }
    else if (iResult == 0)
    {
        std::cout << "conneciton closd\n";
    }
    else 
    {
        std::cout << "recv failed" << WSAGetLastError() << "\n";
    }

    std::cout << "all done";
    
    closesocket(sock);
    WSACleanup();
    return 0;
}
