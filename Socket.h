#pragma once
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

class Socket
{
public:
	Socket(std::string addr, int port);
	~Socket();
	int connect();
	int send(char* buffer, int bufferLength);
	int recv(char* buffer, int bufferLength);
private:
	WSADATA wsaData;
	bool wsaStarted;
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo* result = 0;
};