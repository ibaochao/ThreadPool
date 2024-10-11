#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include <WS2tcpip.h> // 添加这个头文件inet_pton
#pragma comment(lib,"ws2_32")
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;


int server() {

    std::cout << "This is server windows" << std::endl;

    // 初始化网络环境
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  // 0成功，其它失败
    {
        std::cerr << "Failed to initialize Windsock" << std::endl;
        return 1;
    }

    // 创建服务器socket套接字
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    // 服务器地址：IP和端口
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9999);
    if (inet_pton(AF_INET, "10.142.221.111", &(serverAddr.sin_addr)) <= 0)
    {
        std::cerr << "无效的服务器地址" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 服务器socket套接字绑定服务器地址
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 服务器开启监听，队列长度是5（自定义）
    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        std::cerr << "Failed to listen onf socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 服务器启动
    sockaddr_in addrCli;
    int len = sizeof(addrCli);
    SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&addrCli, &len);
    while (true)
    {
        //SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Failed to accept client connection" << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }
        const char* client_IP = inet_ntoa(addrCli.sin_addr);  //将网络字节序转换为127.0.0.1字符串
        // 接收信息
        char revBuffer[1024] = "";
        //int ret;
        if (recv(clientSocket, revBuffer, sizeof(revBuffer), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to receive data" << std::endl;
            closesocket(clientSocket);
            continue;
        }
        //revBuffer[ret] = '\0';
        std::cout << "收到客户端[" << client_IP<<"]消息: "<< revBuffer << std::endl;
        // 发送信息
        std::string message = "ACK client";
        /*std::stringstream ss;
        ss<<"客户端[" << client_IP << "]: ";
        std::string message = ss.str();
        message += revBuffer+'\n';
        std::cout << ret << std::endl;
        std::cout << revBuffer << std::endl;
        std::cout << message << std::endl;*/
        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data" << std::endl;
        }
        
    }
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

	/*system("pause");
	return 0;*/
}



int client() {

    // 初始化网络环境
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "初始化网络库失败" << std::endl;
        WSACleanup();
        return 1;
    }

    // 创建客户端socket套接字
    SOCKET clientsocket;
    if ((clientsocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cerr << "创建套接字失败" << std::endl;
        closesocket(clientsocket);
        WSACleanup();
        return 1;
    }

    // 服务器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    if (inet_pton(AF_INET, "10.142.221.111", &(server_addr.sin_addr)) <= 0)
    {
        std::cerr << "无效的服务器地址" << std::endl;
        closesocket(clientsocket);
        WSACleanup();
        return 1;
    }

    // 与服务器建立连接
    if (connect(clientsocket, (sockaddr*)&server_addr, sizeof(sockaddr)) == SOCKET_ERROR)
    {
        std::cerr << "连接到服务器端失败" << std::endl;
        closesocket(clientsocket);
        WSACleanup();
        return 1;
    }

    // 通信
    while (true)
    {   
        std::cout << "Enter a message to send (or 'quit' to exit): ";
        std::string message;
        std::getline(std::cin, message);
        // Check if the user wants to quit
        if (message == "quit")
        {
            break;
        }
        // 发送
        if (send(clientsocket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data" << std::endl;
            closesocket(clientsocket);
            WSACleanup();
            return 1;
        }
        // 接收
        char revBuffer[1024] = "";
        //int ret;
        if (recv(clientsocket, revBuffer, sizeof(revBuffer), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to receive data" << std::endl;
            closesocket(clientsocket);
            WSACleanup();
            return 1;
        }
        //revBuffer[ret] = '\0';
        std::cout << "服务器：" << revBuffer << std::endl;
    }

    closesocket(clientsocket);
    WSACleanup();

    /*system("pause");
    return 0;*/
}


//int main() {
//
//	thread s(server);
//	thread c(client);
//
//	s.join();
//	c.join();
//
//
//	system("pause");
//	return 0;
//}