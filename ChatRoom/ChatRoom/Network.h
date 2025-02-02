#pragma once
#define WIN32_LEAN_AND_MEAN  // Reduces unnecessary includes from Windows.h

#include <winsock2.h>        // Must come before windows.h
#include <ws2tcpip.h>
      // Ensure this comes after winsock2.h

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <conio.h>
#include<vector>
#include<mutex>
#pragma comment(lib, "ws2_32.lib")


#define DEFAULT_BUFFER_SIZE 1024
//
//class Network {
//    WSADATA wsaData;
//    bool wsaInitialized = false;
//    SOCKET clientSocket = INVALID_SOCKET;
//
//    inline static std::vector<std::string> messageQueue;
//    inline static std::mutex queueMutex;
//
//public:
//    std::atomic<bool> close = false;
//    std::atomic<bool> isConnected = false;
//
//    Network() {
//        if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
//            wsaInitialized = true;
//        }
//        else {
//            std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
//        }
//    }
//
//    ~Network() {
//        Disconnect();
//        if (wsaInitialized) {
//            WSACleanup();
//        }
//    }
//
//    void Disconnect() {
//        close = true;
//        isConnected = false;
//        if (clientSocket != INVALID_SOCKET) {
//            closesocket(clientSocket);
//            clientSocket = INVALID_SOCKET;
//        }
//    }
//
//    bool client(const char* host = "127.0.0.1", unsigned int port = 65432) {
//        if (!wsaInitialized) return false;
//
//        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//        if (clientSocket == INVALID_SOCKET) {
//            std::cerr << "Socket failed: " << WSAGetLastError() << std::endl;
//            return false;
//        }
//
//        sockaddr_in server_address = {};
//        server_address.sin_family = AF_INET;
//        server_address.sin_port = htons(port);
//        if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
//            std::cerr << "Invalid address" << std::endl;
//            closesocket(clientSocket);
//            return false;
//        }
//
//        if (connect(clientSocket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
//            std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
//
//            closesocket(clientSocket);
//            return false;
//        }
//
//        isConnected = true;
//        close = false;
//        std::thread(&Network::Receive, this).detach();
//        return true;
//    }
//
//    void Send(const std::string& sentence) {
//        if (!isConnected || close) return;
//
//        int result = send(clientSocket, sentence.c_str(), sentence.size(), 0);
//        if (result == SOCKET_ERROR) {
//            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
//            Disconnect();
//        }
//    }
//
//    void Receive() {
//        char buffer[1024];
//        while (!close) {
//            int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
//            if (bytes > 0) {
//                buffer[bytes] = 0;
//                std::lock_guard<std::mutex> lock(queueMutex);
//                messageQueue.push_back(buffer);
//            }
//            else if (bytes == 0) {
//                std::cout << "Connection closed\n";
//                Disconnect();
//                break;
//            }
//            else {
//                if (!close) {
//                    std::cerr << "Receive error: " << WSAGetLastError() << std::endl;
//                    Disconnect();
//                }
//                break;
//            }
//        }
//    }
//
//    std::vector<std::string> GetMessages() {
//        std::lock_guard<std::mutex> lock(queueMutex);
//        auto copy = messageQueue;
//        messageQueue.clear();
//        return copy;
//    }
//};




class Network {
    // SOCKET clientSocket = INVALID_SOCKET;
    inline static std::vector<std::string> messageQueue;
    inline static std::mutex queueMutex;
    SOCKET clientSocket = INVALID_SOCKET;


public:
    std::atomic<bool> close = false ;
    std::atomic<bool> isConnected = false ; 

    void Send(std::string sentence) {
        int count = 0;
        while (!close) {
            if (isConnected && !close) {
                send(clientSocket, sentence.c_str(), sentence.size(), 0);
                break;
            }
        }
        closesocket(clientSocket);

    }

   

    void Receive(SOCKET clientSocket) {

        char buffer[1024];
        while (!close) {
            int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = 0;
                std::lock_guard<std::mutex> lock(Network::queueMutex);
                messageQueue.push_back(buffer);
            }
        }

    }

    bool client() {

        const char* host = "127.0.0.1"; // Server IP address
        unsigned int port = 65432;

        // Initialize WinSock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
            return false;
        }

        // Create a socket
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return false;
        }

        // Resolve the server address and port
        sockaddr_in server_address = {};
        server_address.sin_family = AF_INET;
        //server_address.sin_port = htons(std::stoi(port));
        server_address.sin_port = htons(port);
        if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported" << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        // Connect to the server
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
            std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Connected to the server." << std::endl;

        isConnected = true; // 连接成功时设置状态
        close = false;
        std::thread(&Network::Receive, this, clientSocket).detach();


        WSACleanup();
        return true;
    }

    std::vector<std::string> GetMessages() {
        std::lock_guard<std::mutex> lock(queueMutex);
        auto copy = messageQueue;
        messageQueue.clear();
        return copy;
    }

};




//std::atomic<bool> close = false;
//
//void Send(SOCKET  client_socket) {
//    int count = 0;
//    while (!close) {
//        if (_kbhit()) { // non-blocking keyboard input 
//            std::cout << "[You] ";
//            std::cout << "Send(" << count++ << "): ";
//            std::string sentence;
//
//            std::getline(std::cin, sentence);
//
//            if (sentence == "!bye") {
//                close = true;
//                std::cout << "Exiting\n";
//            }
//
//            // Send the sentence to the server
//            if (send(client_socket, sentence.c_str(), static_cast<int>(sentence.size()), 0) == SOCKET_ERROR) {
//                if (close) std::cout << "Terminating\n";
//                else std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
//                break;
//            }
//        }
//    }
//    closesocket(client_socket);
//}
//
//void Receive(SOCKET client_socket) {
//    int count = 0;
//    while (!close) {
//        // Receive the reversed sentence from the server
//        char buffer[DEFAULT_BUFFER_SIZE] = { 0 };
//        int bytes_received = recv(client_socket, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
//        if (bytes_received > 0) {
//            std::cout << "[Public] " << buffer << std::endl;
//            buffer[bytes_received] = '\0'; // Null-terminate the received data
//            std::cout << "Received(" << count++ << "): " << buffer << std::endl;
//        }
//        else if (bytes_received == 0) {
//            std::cout << "Connection closed by server." << std::endl;
//        }
//        else if (close) {
//            std::cout << "Terminating connection\n";
//        }
//        else {
//            std::cerr << "Receive failed with error: " << WSAGetLastError() << std::endl;
//        }
//        if (strcmp(buffer, "!bye") == 0) {
//            close = true;
//        }
//    }
//}
//
//void client() {
//
//    const char* host = "127.0.0.1"; // Server IP address
//    unsigned int port = 65432;
//
//    // Initialize WinSock
//    WSADATA wsaData;
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
//        return;
//    }
//
//    // Create a socket
//    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (client_socket == INVALID_SOCKET) {
//        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
//        WSACleanup();
//        return;
//    }
//
//    // Resolve the server address and port
//    sockaddr_in server_address = {};
//    server_address.sin_family = AF_INET;
//    //server_address.sin_port = htons(std::stoi(port));
//    server_address.sin_port = htons(port);
//    if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
//        std::cerr << "Invalid address/ Address not supported" << std::endl;
//        closesocket(client_socket);
//        WSACleanup();
//        return;
//    }
//
//    // Connect to the server
//    if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
//        std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
//        closesocket(client_socket);
//        WSACleanup();
//        return;
//    }
//
//    std::cout << "Connected to the server." << std::endl;
//
//    //  Send(client_socket);
//     // Receive(client_socket);
//    std::thread t1 = std::thread(Send, client_socket);
//    std::thread t2 = std::thread(Receive, client_socket);
//
//    t1.join();
//    t2.join();
//
//    // Cleanup
//  //  closesocket(client_socket);
//    WSACleanup();
//}
//
////int main() {
////    client();
////}