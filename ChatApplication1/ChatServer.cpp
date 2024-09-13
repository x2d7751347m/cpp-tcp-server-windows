// Chatbot_server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>

#pragma comment (lib, "ws2_32.lib")

#define NEWLINE "\n"

fd_set master;

void CleanUp(SOCKET soc)
{
    closesocket(soc);
    WSACleanup();
}

void connection_handler(SOCKET soc)
{
    char clientName[50];
    ZeroMemory(clientName, 50);
    recv(soc, clientName, sizeof(clientName), 0);

    std::string welcome_message = "Server: Welcome to chat server ";
    welcome_message = welcome_message + clientName;
    for (auto i = 0; i < master.fd_count; i++)
    {
        send(master.fd_array[i], welcome_message.c_str(), strlen(welcome_message.c_str()), 0);
    }

    std::cout << '\r';
    std::cout << clientName << " connected" << NEWLINE;
    std::cout << "Server: ";

    while (true)
    {
        char recvbuf[512];
        ZeroMemory(recvbuf, 512);

        auto iRecv = recv(soc, recvbuf, sizeof(recvbuf), 0);
        if (iRecv > 0)
        {
            std::cout << '\r';
            std::cout << recvbuf << NEWLINE;
            std::cout << "Server: ";
            for (int i = 0; i < master.fd_count; i++)
            {
                if (master.fd_array[i] != soc)
                    send(master.fd_array[i], recvbuf, strlen(recvbuf), 0);
            }
        }
        else if (iRecv == 0) {
            FD_CLR(soc, &master);
            std::cout << '\r';
            std::cout << "A client disconnected" << NEWLINE;
            std::cout << "Server: ";
            // Annouce to others
            for (int i = 0; i < master.fd_count; i++) {
                char disconnected_message[] = "Server : A client disconnected";
                send(master.fd_array[i], disconnected_message, strlen(disconnected_message), 0);
            }
            return;
        }
        else {
            FD_CLR(soc, &master);
            std::cout << '\r';
            std::cout << "a client receive failed: " << WSAGetLastError() << NEWLINE;
            std::cout << "Server: ";
            // Annouce to others
            for (int i = 0; i < master.fd_count; i++) {
                char disconnected_message[] = "Server : A client disconnected";
                send(master.fd_array[i], disconnected_message, strlen(disconnected_message), 0);
            }
            return;
        }
    }
}

// multiple client using threads for each client
void accept_handler(SOCKET soc)
{
    while (true)
    {
        auto acceptSoc = ::accept(soc, nullptr, nullptr);
        if (acceptSoc == INVALID_SOCKET) {
            std::cout << '\r';
            std::cout << "accept error: " << WSAGetLastError() << NEWLINE;
            std::cout << "Server: ";
            closesocket(acceptSoc);
        }
        else
        {
            std::thread newCon = std::thread(connection_handler, acceptSoc);
            newCon.detach();
            FD_SET(acceptSoc, &master);
        }
    }
}

void send_handler(SOCKET soc)
{
    std::string data;
    do
    {
        std::cout << "Server: ";
        getline(std::cin, data);
        data = "Server: " + data;
        for (auto i = 0; i < master.fd_count; i++)
        {
            auto result = send(master.fd_array[i], data.c_str(), strlen(data.c_str()), 0);
            if (result == SOCKET_ERROR) {
                std::cout << '\r';
                std::cout << "send to client " << i << " failed: " << WSAGetLastError() << NEWLINE;
                std::cout << "Server: ";
                return;
            }
        }
    } while (data.size() > 0);
}

void OutputData(std::string message)
{
    std::cout << message << "\n";
}

void StartServer()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        OutputData("WSAStartup failed");
        return;
    }
    OutputData("WSAStartup successfull");

    auto serverSoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSoc == INVALID_SOCKET)
    {
        OutputData("Socket creation failed");
        return;
    }
    OutputData("Socket creation successfull");

    // fill server address
    struct sockaddr_in TCPServerAdd;
    TCPServerAdd.sin_family = AF_INET;
    TCPServerAdd.sin_port = htons(8000);

    InetPton(AF_INET, _T("127.0.0.1"), &TCPServerAdd.sin_addr.s_addr);

    auto result = bind(serverSoc, (sockaddr*)&TCPServerAdd, sizeof(TCPServerAdd));
    if (result == SOCKET_ERROR)
    {
        OutputData("listen failed");
        CleanUp(serverSoc);
        return;
    }
    OutputData("bind successfull");

    result = listen(serverSoc, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        OutputData("listen failed");
        CleanUp(serverSoc);
        return;
    }
    OutputData("listen successfull");

    std::thread acceptThread = std::thread(accept_handler, serverSoc);
    acceptThread.detach();

    std::thread sendThread = std::thread(send_handler, serverSoc);
    sendThread.join();

    CleanUp(serverSoc);
}

int main()
{
    StartServer();
}