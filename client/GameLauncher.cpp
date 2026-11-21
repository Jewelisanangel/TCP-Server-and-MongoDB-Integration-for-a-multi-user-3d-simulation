#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

const std::string SERVER_IP = "192.168.100.102";
const int SERVER_PORT = 8080;

// ============================================================
// NETWORK FUNCTIONS
// ============================================================
std::string sendRequest(const std::string& jsonData) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return "{\"success\":false,\"message\":\"Socket creation failed\"}";
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP.c_str(), &addr.sin_addr);
    addr.sin_port = htons(SERVER_PORT);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return "{\"success\":false,\"message\":\"Connection failed\"}";
    }

    send(sock, jsonData.c_str(), (int)jsonData.length(), 0);

    char buffer[4096] = { 0 };
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    std::string response(buffer);

    closesocket(sock);
    WSACleanup();
    return response;
}

// ============================================================
// SIMPLE JSON PARSER (FIXED)
// ============================================================
std::string getJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) return "";

    // Skip to value
    size_t start = colonPos + 1;
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) {
        start++;
    }

    // Check if value is a string
    if (json[start] == '"') {
        size_t end = json.find("\"", start + 1);
        if (end == std::string::npos) return "";
        return json.substr(start + 1, end - start - 1);
    }

    // Check if value is a number or boolean
    size_t end = start;
    while (end < json.length() && (isdigit(json[end]) || json[end] == '.' || json[end] == '-' || json[end] == 'e' || json[end] == 'E')) {
        end++;
    }
    if (end > start) {
        return json.substr(start, end - start);
    }

    return "";
}

bool getJsonBool(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return false;

    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) return false;

    size_t start = colonPos + 1;
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t' || json[start] == '\n' || json[start] == '\r')) {
        start++;
    }

    return json.substr(start, 4) == "true";
}

float getJsonFloat(const std::string& json, const std::string& key) {
    std::string val = getJsonValue(json, key);
    if (val.empty()) return 0.0f;
    try {
        return std::stof(val);
    }
    catch (...) {
        return 0.0f;
    }
}

int getJsonInt(const std::string& json, const std::string& key) {
    std::string val = getJsonValue(json, key);
    if (val.empty()) return 0;
    try {
        return std::stoi(val);
    }
    catch (...) {
        return 0;
    }
}

// ============================================================
// GAME LAUNCHER
// ============================================================
class GameLauncher {
private:
    std::string username;
    std::string password;
    bool isLoggedIn;
    float posX, posY, posZ;
    int health;
    int level;
    int score;

public:
    GameLauncher() : isLoggedIn(false), posX(0), posY(0), posZ(0), health(100), level(1), score(0) {}

    void clearScreen() {
        system("cls");
    }

    void displayMenu() {
        clearScreen();
        std::cout << "========================================" << std::endl;
        std::cout << "          GAME LAUNCHER                " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        if (isLoggedIn) {
            displayPlayerData();
        }
        else {
            displayLoginMenu();
        }
    }

    void displayLoginMenu() {
        std::cout << "  1. Register" << std::endl;
        std::cout << "  2. Login" << std::endl;
        std::cout << "  3. Exit" << std::endl;
        std::cout << std::endl;
        std::cout << "Enter your choice: ";
    }

    void displayPlayerData() {
        std::cout << "  Welcome, " << username << "!" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "  Position: (" << posX << ", " << posY << ", " << posZ << ")" << std::endl;
        std::cout << "  Health: " << health << std::endl;
        std::cout << "  Level: " << level << std::endl;
        std::cout << "  Score: " << score << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << std::endl;
        std::cout << "  1. Play (Save & Continue)" << std::endl;
        std::cout << "  2. Save Game" << std::endl;
        std::cout << "  3. Logout" << std::endl;
        std::cout << std::endl;
        std::cout << "Enter your choice: ";
    }

    void registerUser() {
        clearScreen();
        std::cout << "========================================" << std::endl;
        std::cout << "           REGISTER                    " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;

        std::string jsonData = "{";
        jsonData += "\"command\":\"REGISTER\",";
        jsonData += "\"player_id\":\"" + username + "\",";
        jsonData += "\"password\":\"" + password + "\"";
        jsonData += "}";

        std::cout << "\n[DEBUG] Sending: " << jsonData << std::endl;

        std::string response = sendRequest(jsonData);

        std::cout << "[DEBUG] Response: " << response << std::endl;

        if (getJsonBool(response, "success")) {
            std::cout << std::endl;
            std::cout << "✅ Account created successfully!" << std::endl;
            std::cout << "   You can now login." << std::endl;
        }
        else {
            std::cout << std::endl;
            std::cout << "❌ " << getJsonValue(response, "message") << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    void loginUser() {
        clearScreen();
        std::cout << "========================================" << std::endl;
        std::cout << "            LOGIN                      " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;

        std::string jsonData = "{";
        jsonData += "\"command\":\"LOGIN\",";
        jsonData += "\"player_id\":\"" + username + "\",";
        jsonData += "\"password\":\"" + password + "\"";
        jsonData += "}";

        std::cout << "\n[DEBUG] Sending: " << jsonData << std::endl;

        std::string response = sendRequest(jsonData);

        std::cout << "[DEBUG] Response: " << response << std::endl;

        if (getJsonBool(response, "success")) {
            isLoggedIn = true;

            // Parse player data from response
            // The response looks like: {"success":true,"data":{"position":{"x":0,"y":0,"z":0},"health":100,"level":1,"score":0}}
            // We need to extract from the "data" object
            size_t dataPos = response.find("\"data\"");
            if (dataPos != std::string::npos) {
                std::string dataStr = response.substr(dataPos);
                posX = getJsonFloat(dataStr, "x");
                posY = getJsonFloat(dataStr, "y");
                posZ = getJsonFloat(dataStr, "z");
                health = getJsonInt(dataStr, "health");
                level = getJsonInt(dataStr, "level");
                score = getJsonInt(dataStr, "score");
            }

            std::cout << std::endl;
            std::cout << "✅ Welcome back, " << username << "!" << std::endl;
        }
        else {
            std::cout << std::endl;
            std::cout << "❌ " << getJsonValue(response, "message") << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    void saveGame() {
        // Simulate game progress
        posX += 10.5f;
        posY += 5.3f;
        posZ += 2.1f;
        health = std::min(100, health + 10);
        if (health > 100) level++;
        score += 50;

        std::string jsonData = "{";
        jsonData += "\"command\":\"SAVE\",";
        jsonData += "\"player_id\":\"" + username + "\",";
        jsonData += "\"position\":{\"x\":" + std::to_string(posX) + ",\"y\":" + std::to_string(posY) + ",\"z\":" + std::to_string(posZ) + "},";
        jsonData += "\"health\":" + std::to_string(health) + ",";
        jsonData += "\"level\":" + std::to_string(level) + ",";
        jsonData += "\"score\":" + std::to_string(score);
        jsonData += "}";

        std::cout << "\n[DEBUG] Sending: " << jsonData << std::endl;

        std::string response = sendRequest(jsonData);

        std::cout << "[DEBUG] Response: " << response << std::endl;

        clearScreen();
        std::cout << "========================================" << std::endl;
        std::cout << "          SAVE GAME                   " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        if (getJsonBool(response, "success")) {
            std::cout << "✅ Game saved successfully!" << std::endl;
            std::cout << std::endl;
            std::cout << "   New Position: (" << posX << ", " << posY << ", " << posZ << ")" << std::endl;
            std::cout << "   Health: " << health << std::endl;
            std::cout << "   Level: " << level << std::endl;
            std::cout << "   Score: " << score << std::endl;
        }
        else {
            std::cout << "❌ Save failed: " << getJsonValue(response, "message") << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    void logoutUser() {
        isLoggedIn = false;
        username = "";
        password = "";
        std::cout << "✅ Logged out successfully!" << std::endl;
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    void run() {
        int choice;

        while (true) {
            displayMenu();
            std::cin >> choice;

            if (!isLoggedIn) {
                switch (choice) {
                case 1:
                    registerUser();
                    break;
                case 2:
                    loginUser();
                    break;
                case 3:
                    std::cout << "Goodbye!" << std::endl;
                    return;
                default:
                    std::cout << "Invalid choice!" << std::endl;
                    std::cin.ignore();
                    std::cin.get();
                }
            }
            else {
                switch (choice) {
                case 1:
                    saveGame();
                    break;
                case 2:
                    saveGame();
                    break;
                case 3:
                    logoutUser();
                    break;
                default:
                    std::cout << "Invalid choice!" << std::endl;
                    std::cin.ignore();
                    std::cin.get();
                }
            }
        }
    }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  GAME LAUNCHER - C++ VERSION" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    GameLauncher launcher;
    launcher.run();

    return 0;
}
