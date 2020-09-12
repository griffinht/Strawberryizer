#pragma once
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

class Socket
{
public:
	Socket(std::string addr, int port);
	Socket(SOCKET sock);
	~Socket();
	int send(char* buffer, int bufferLength);
	int recv(char* buffer, int bufferLength);
private:
	int connect(struct addrinfo* result);
private:
	bool wsaStarted;
	SOCKET sock = INVALID_SOCKET;
};