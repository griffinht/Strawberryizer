#pragma once
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

#include "Socket.h"

class ServerSocket
{
public:
	ServerSocket(int port);
	~ServerSocket();
	Socket listen();
private:
	WSADATA wsaData;
	bool wsaStarted;
	SOCKET sock = INVALID_SOCKET;
};