#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

void SaveToMongoDB(const std::string& data) {
    // Save to backup file
    std::ofstream file("saved_data.json", std::ios::app);
    if (file.is_open()) {
        file << data << std::endl;
        file.close();
        std::cout << "[FILE] Data saved to file!" << std::endl;
    }

    // Save to temporary file for Python processing
    std::ofstream temp("temp_data.json");
    if (temp.is_open()) {
        temp << data;
        temp.close();
        std::cout << "[TEMP] Data written to temp_data.json" << std::endl;
    }
    else {
        std::cout << "[TEMP] ERROR: Could not create temp_data.json" << std::endl;
        return;
    }

    // Call Python script to process MongoDB operations
    std::string command = "python mongodb_saver.py temp_data.json";
    std::cout << "[PYTHON] Command: " << command << std::endl;
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "[MONGODB] Operation completed successfully!" << std::endl;
    }
    else {
        std::cout << "[MONGODB] Python execution failed (error code: " << result << ")" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  MONGODB SERVER" << std::endl;
    std::cout << "========================================" << std::endl;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed!" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Failed to create socket!" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Failed to bind port 8080!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cout << "Failed to listen!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[SUCCESS] Server started on port 8080" << std::endl;

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent* host = gethostbyname(hostname);
        if (host && host->h_addr_list[0]) {
            struct in_addr addr;
            addr.s_addr = *(u_long*)host->h_addr_list[0];
            std::cout << "[INFO] Your IP: " << inet_ntoa(addr) << std::endl;
        }
    }

    std::cout << "[INFO] Waiting for connections..." << std::endl << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        std::cout << "[CLIENT] Connected!" << std::endl;

        // ============================================================
        // KEEP CONNECTION OPEN - Buffer until complete JSON
        // ============================================================
        std::string buffer;

        while (true) {
            char chunk[4096] = { 0 };
            int bytesReceived = recv(clientSocket, chunk, sizeof(chunk) - 1, 0);

            if (bytesReceived <= 0) {
                // Client disconnected
                break;
            }

            buffer += chunk;

            // Process all complete JSON messages in the buffer
            size_t pos = 0;
            while (pos < buffer.length()) {
                // Find the start of a JSON object
                size_t start = buffer.find('{', pos);
                if (start == std::string::npos) break;

                // Find the matching closing brace
                int braceCount = 0;
                size_t end = start;
                bool foundComplete = false;

                for (size_t i = start; i < buffer.length(); i++) {
                    if (buffer[i] == '{') braceCount++;
                    else if (buffer[i] == '}') {
                        braceCount--;
                        if (braceCount == 0) {
                            end = i;
                            foundComplete = true;
                            break;
                        }
                    }
                }

                if (!foundComplete) break;  // Wait for more data

                // Extract the complete JSON
                std::string jsonMessage = buffer.substr(start, end - start + 1);

                // Remove it from the buffer
                buffer.erase(0, end + 1);

                // Process the complete JSON
                if (!jsonMessage.empty()) {
                    // Hide passwords from logs
                    std::string displayData = jsonMessage;
                    size_t passwordPos = displayData.find("\"password\"");
                    if (passwordPos != std::string::npos) {
                        size_t colonPos = displayData.find(":", passwordPos) + 1;
                        size_t commaPos = displayData.find(",", colonPos);
                        if (commaPos == std::string::npos) commaPos = displayData.find("}", colonPos);
                        if (commaPos != std::string::npos) {
                            displayData.replace(colonPos, commaPos - colonPos, " \"********\"");
                        }
                    }
                    std::cout << "[CLIENT DATA] " << displayData << std::endl;

                    // Process data in MongoDB via Python
                    SaveToMongoDB(jsonMessage);

                    // Read output generated by mongodb_saver.py
                    std::string responsePayload = "{\"success\":false,\"message\":\"No response generated\"}";
                    std::ifstream response_file("response.json");
                    if (response_file.is_open()) {
                        responsePayload.assign((std::istreambuf_iterator<char>(response_file)),
                            std::istreambuf_iterator<char>());
                        response_file.close();
                    }
                    else {
                        std::cout << "[PYTHON] No response file found!" << std::endl;
                    }

                    // Send response back to the game client
                    send(clientSocket, responsePayload.c_str(), static_cast<int>(responsePayload.length()), 0);
                    std::cout << "[SERVER SENT] " << responsePayload << std::endl;
                }
            }
        }

        // Client disconnected
        closesocket(clientSocket);
        std::cout << "[CLIENT] Disconnected." << std::endl << std::endl;
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}