#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "socket.h"

#define HOST "localhost"
#define PORT 69

int main()
{
    std::cout << "Starting StrawberryCop...\n";

    std::ifstream ifs;
    ifs.open("testImages/in/tromp.jpg", std::ios::in | std::ios::binary | std::ios::ate);//todo error checking
    int fSize = ifs.tellg();
    ifs.seekg(0);
    char* buffer = new char[4 + fSize];
    memcpy(buffer, &fSize, 4);
    ifs.read(buffer + 4, fSize);
    ifs.close();
    
    Socket sock(HOST, PORT);

    sock.send(buffer, 4 + fSize);
    //delete[] buffer;

    char recvLengthBuffer[4];
    int length = sock.recv(recvLengthBuffer, 4);
    if (length > 0)
    {
        std::cout << "got " << length << "\n";
        if (length != 4)
        {
            std::cout << "didnt get 4 bytes for length\n";
            //idk or something todo???
        }
        else
        {
            int recvLength = 0;
            memcpy(&recvLength, recvLengthBuffer, 4);
            std::cout << recvLength << "\n";
            char *recvBuffer = new char[recvLength];//todo bounds checking in case of corruption or something
            length = sock.recv(recvBuffer, recvLength);
            if (length > 0)
            {
                std::cout << "got " << length << "\n";
                if (length != recvLength)
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
            delete[] recvBuffer;
        }
    }

    std::cout << "all done";
    
    return 0;
}
