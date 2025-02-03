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
class Network {
private:
    // This mutex protects the shared message queue.
    inline static std::mutex queueMutex;
    // Underlying socket for the connection.
    SOCKET clientSocket = INVALID_SOCKET;
    // Background thread to receive messages.
    std::thread receiverThread;

public:
    // Atomic flags for connection status.
    std::atomic<bool> close = false;
    std::atomic<bool> isConnected = false;

    // Message queue to store received messages.
    std::vector<std::string> messageQueue;
    const size_t MAX_QUEUE_SIZE = 1000;

    // Close the connection and update flags.
    void CloseConnection() {
        close = true;
        if (clientSocket != INVALID_SOCKET) {
            shutdown(clientSocket, SD_BOTH);
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
        isConnected = false;
    }

    // Thread-safe method to add a message to the queue.
    void AddMessage(const std::string& msg) {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (messageQueue.size() >= MAX_QUEUE_SIZE) {
            messageQueue.erase(messageQueue.begin());
        }
        messageQueue.push_back(msg);
    }

    // Send a complete message over the socket.
    void Send(const std::string& sentence) {
        if (!isConnected || close)
            return;
        int totalSent = 0;
        while (totalSent < sentence.size()) {
            int sent = send(clientSocket, sentence.c_str() + totalSent,
                sentence.size() - totalSent, 0);
            if (sent == SOCKET_ERROR) {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;
                CloseConnection();
                return;
            }
            totalSent += sent;
        }
    }

    // Background receive function.
    // It reads data from the socket, splits messages on newline characters,
    // and then stores complete messages in the message queue.
    void Receive() {
        std::vector<char> buffer(4096);
        std::string pending;
        while (!close) {
            int bytes = recv(clientSocket, &buffer[0], static_cast<int>(buffer.size()), 0);
            if (bytes > 0) {
                pending.append(buffer.begin(), buffer.begin() + bytes);
                size_t pos;
                // Split received data by newline.
                while ((pos = pending.find('\n')) != std::string::npos) {
                    std::string msg = pending.substr(0, pos);
                    pending.erase(0, pos + 1);
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        messageQueue.push_back(msg);
                    }
                }
            }
            else if (bytes == 0) {
                // Connection closed gracefully by the server.
                CloseConnection();
                break;
            }
            else {
                // On error (other than a nonblocking condition) close the connection.
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    CloseConnection();
                break;
            }
        }
    }

    // Connect to a server given a host and port.
    // This method initializes WinSock, creates a socket, connects, and starts the receiver thread.
    bool Connect(const std::string& host, unsigned short port) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
            return false;
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return false;
        }

        sockaddr_in server_address = {};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        if (inet_pton(AF_INET, host.c_str(), &server_address.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported" << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&server_address),
            sizeof(server_address)) == SOCKET_ERROR) {
            std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Connected to the server." << std::endl;
        isConnected = true;
        close = false;
        // Start the background receiver thread.
        receiverThread = std::thread(&Network::Receive, this);
        return true;
    }

    // Retrieve and clear the current message queue in a thread-safe manner.
    std::vector<std::string> GetMessages() {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::vector<std::string> copy = messageQueue;
        messageQueue.clear();
        return copy;
    }

    // Return the current connection status.
    bool IsConnected() const {
        return isConnected;
    }

    // Stop the network operations and join the receiver thread.
    void Stop() {
        CloseConnection();
        if (receiverThread.joinable())
            receiverThread.join();
        WSACleanup();
    }

    ~Network() {
        Stop();
    }
};