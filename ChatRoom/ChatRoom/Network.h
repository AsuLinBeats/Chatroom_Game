#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <conio.h>
#include <mutex>
#include<queue>
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFFER_SIZE 1024
// same as server 
struct message {
    std::string content;
    std::string client_id;
};

std::mutex lock;

// Used to connect with server


class clientChat {
    std::atomic<bool> close = false;
    bool connected = false;
    std::queue<message> messagesReceiving;
    std::mutex queueMutex;

    std::queue<message> incomingMessages;
    std::queue<std::string> outgoingMessages;

public:
    //void Send(SOCKET  client_socket) {
    //     int count = 0;
    //    while (!close) {
    //        if (_kbhit()) { // non-blocking keyboard input 
    //            std::cout << "[You] ";
    //            std::cout << "Send(" << count++ << "): ";
    //            std::string sentence;

    //            std::getline(std::cin, sentence);

    //            if (sentence == "!bye") {
    //                close = true;
    //                std::cout << "Exiting\n";
    //            }

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
};