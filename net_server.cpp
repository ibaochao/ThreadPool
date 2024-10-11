#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include <WS2tcpip.h> // ������ͷ�ļ�inet_pton
#pragma comment(lib,"ws2_32")
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;


int server() {

    std::cout << "This is server windows" << std::endl;

    // ��ʼ�����绷��
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)  // 0�ɹ�������ʧ��
    {
        std::cerr << "Failed to initialize Windsock" << std::endl;
        return 1;
    }

    // ����������socket�׽���
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    // ��������ַ��IP�Ͷ˿�
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9999);
    if (inet_pton(AF_INET, "10.142.221.111", &(serverAddr.sin_addr)) <= 0)
    {
        std::cerr << "��Ч�ķ�������ַ" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // ������socket�׽��ְ󶨷�������ַ
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // �������������������г�����5���Զ��壩
    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        std::cerr << "Failed to listen onf socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // ����������
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
        const char* client_IP = inet_ntoa(addrCli.sin_addr);  //�������ֽ���ת��Ϊ127.0.0.1�ַ���
        // ������Ϣ
        char revBuffer[1024] = "";
        //int ret;
        if (recv(clientSocket, revBuffer, sizeof(revBuffer), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to receive data" << std::endl;
            closesocket(clientSocket);
            continue;
        }
        //revBuffer[ret] = '\0';
        std::cout << "�յ��ͻ���[" << client_IP<<"]��Ϣ: "<< revBuffer << std::endl;
        // ������Ϣ
        std::string message = "ACK client";
        /*std::stringstream ss;
        ss<<"�ͻ���[" << client_IP << "]: ";
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

    // ��ʼ�����绷��
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "��ʼ�������ʧ��" << std::endl;
        WSACleanup();
        return 1;
    }

    // �����ͻ���socket�׽���
    SOCKET clientsocket;
    if ((clientsocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cerr << "�����׽���ʧ��" << std::endl;
        closesocket(clientsocket);
        WSACleanup();
        return 1;
    }

    // ��������ַ
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    if (inet_pton(AF_INET, "10.142.221.111", &(server_addr.sin_addr)) <= 0)
    {
        std::cerr << "��Ч�ķ�������ַ" << std::endl;
        closesocket(clientsocket);
        WSACleanup();
        return 1;
    }

    // ���������������
    if (connect(clientsocket, (sockaddr*)&server_addr, sizeof(sockaddr)) == SOCKET_ERROR)
    {
        std::cerr << "���ӵ���������ʧ��" << std::endl;
        closesocket(clientsocket);
        WSACleanup();
        return 1;
    }

    // ͨ��
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
        // ����
        if (send(clientsocket, message.c_str(), message.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data" << std::endl;
            closesocket(clientsocket);
            WSACleanup();
            return 1;
        }
        // ����
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
        std::cout << "��������" << revBuffer << std::endl;
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