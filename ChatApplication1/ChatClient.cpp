// ChatBot_Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>

#pragma comment (lib, "ws2_32.lib")

std::string g_name;

void OutputData(std::string message)
{
    std::cout << message << "\n";
}

void CleanUp(SOCKET soc)
{
    closesocket(soc);
    WSACleanup();
}

void send_handle(SOCKET soc)
{
    std::string data;
    do
    {
        std::cout << g_name << " : ";
        std::getline(std::cin, data);
        if (data.size() > 0)
        {
            data = g_name + ": " + data;
            if (send(soc, data.c_str(), strlen(data.c_str()), 0) == SOCKET_ERROR)
            {
                std::cout << "send failed: " << WSAGetLastError() << "\n";
                CleanUp(soc);
                std::getchar();
                return;
            }
        }
    } while (data.size() > 0);
}

void recv_handle(SOCKET soc)
{
    while (true)
    {
        char recvbuf[512];
        ZeroMemory(recvbuf, 512);
        auto iRecv = recv(soc, recvbuf, 512, 0);
        if (iRecv > 0)
        {
            std::cout << '\r';
            std::cout << recvbuf << "\n";
            std::cout << g_name << ": ";
        }
        else if (iRecv == 0) {
            std::cout << "Connection closed" << "\n";
            return;
        }
        else {
            std::cout << "recv failed: " << WSAGetLastError() << "\n";
            CleanUp(soc);
            std::getchar();
            return;
        }
    }
}

void Client()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        OutputData("WSAStartup failed");
        return;
    }
    OutputData("WSAStartup successfull");

    auto clientSoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSoc == INVALID_SOCKET)
    {
        OutputData("Socket creation failed");
        CleanUp(clientSoc);
        return;
    }
    OutputData("Socket creation successfull");

    // fill server address
    struct sockaddr_in TCPServerAdd;
    TCPServerAdd.sin_family = AF_INET;
    TCPServerAdd.sin_port = htons(8000);

    InetPton(AF_INET, _T("127.0.0.1"), &TCPServerAdd.sin_addr.s_addr);

    // connect to server
    auto iConnect = connect(clientSoc, (SOCKADDR*)&TCPServerAdd, sizeof(TCPServerAdd));
    if (iConnect == SOCKET_ERROR)
    {
        OutputData("connect failed");
        CleanUp(clientSoc);
        return;
    }
    OutputData("Connect successfull");

    std::cout << "How do we call you, Your name please!";
    std::getline(std::cin, g_name);
    std::cout << "\n";

    auto iSend = send(clientSoc, g_name.c_str(), strlen(g_name.c_str()), 0);
    if (iSend == SOCKET_ERROR)
    {
        std::cout << "Sending Failed " << "\n";
        std::cout << "Error No-> " << WSAGetLastError() << "\n";
        CleanUp(clientSoc);
        return;
    }

    std::thread senderThread = std::thread(send_handle, clientSoc);
    senderThread.detach();

    std::thread recverThread = std::thread(recv_handle, clientSoc);
    recverThread.join();

    CleanUp(clientSoc);
}

int main()
{
    Client();
}