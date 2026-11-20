#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

const std::string SERVER_IP = "192.168.100.102";
const int SERVER_PORT = 8080;

// =======================
// SEND MESSAGE TO SERVER
// =======================
std::string sendToServer(const std::string& jsonData) {
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
        return "{\"success\":false,\"message\":\"Connection failed. Is server running?\"}";
    }

    send(sock, jsonData.c_str(), (int)jsonData.length(), 0);

    char buffer[4096] = { 0 };
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    std::string response(buffer);

    closesocket(sock);
    WSACleanup();
    return response;
}

float getFloatValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return 0.0f;
    
    size_t start = pos + searchKey.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
    
    size_t end = start;
    while (end < json.length() && (isdigit(json[end]) || json[end] == '.' || json[end] == '-')) end++;
    
    if (end > start) {
        return std::stof(json.substr(start, end - start));
    }
    return 0.0f;
}

int getIntValue(const std::string& json, const std::string& key) {
    return (int)getFloatValue(json, key);
}

std::string getStringValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    size_t start = pos + searchKey.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;
    
    if (json[start] != '"') return "";
    
    size_t end = json.find("\"", start + 1);
    if (end == std::string::npos) return "";
    
    return json.substr(start + 1, end - start - 1);
}

// ================
// SAVE GAME DATA
// ================
bool saveGameToServer(const std::string& playerName, float x, float y, float z, int health) {
    std::string json = "{";
    json += "\"command\":\"SAVE\",";
    json += "\"player_id\":\"" + playerName + "\",";
    json += "\"position\":{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(y) + ",\"z\":" + std::to_string(z) + "},";
    json += "\"health\":" + std::to_string(health);
    json += "}";

    std::string response = sendToServer(json);
    
    if (response.find("\"success\":true") != std::string::npos) {
        std::cout << "[SAVE] Game saved successfully!" << std::endl;
        return true;
    } else {
        std::cout << "[SAVE] Failed to save!" << std::endl;
        return false;
    }
}

// ================
// LOAD GAME DATA
// ================
bool loadGameFromServer(const std::string& playerName, float& x, float& y, float& z, int& health) {
    std::string json = "{\"command\":\"LOAD\",\"player_id\":\"" + playerName + "\"}";
    std::string response = sendToServer(json);

    if (response.find("\"success\":true") == std::string::npos) {
        std::cout << "[LOAD] Failed to load data!" << std::endl;
        return false;
    }

    // Parse position
    x = getFloatValue(response, "x");
    y = getFloatValue(response, "y");
    z = getFloatValue(response, "z");
    
    // Parse health
    health = getIntValue(response, "health");

    std::cout << "[LOAD] Loaded: Position(" << x << ", " << y << ", " << z << ") Health: " << health << std::endl;
    return true;
}

// =====================
// REGISTER NEW PLAYER
// =====================
bool registerPlayer(const std::string& username, const std::string& password) {
    std::string json = "{";
    json += "\"command\":\"REGISTER\",";
    json += "\"player_id\":\"" + username + "\",";
    json += "\"password\":\"" + password + "\"";
    json += "}";

    std::string response = sendToServer(json);

    if (response.find("\"success\":true") != std::string::npos) {
        std::cout << "[REGISTER] Account created successfully!" << std::endl;
        return true;
    } else {
        std::cout << "[REGISTER] Failed: " << getStringValue(response, "message") << std::endl;
        return false;
    }
}

// ==============
// LOGIN PLAYER
// =============
bool loginPlayer(const std::string& username, const std::string& password) {
    std::string json = "{";
    json += "\"command\":\"LOGIN\",";
    json += "\"player_id\":\"" + username + "\",";
    json += "\"password\":\"" + password + "\"";
    json += "}";

    std::string response = sendToServer(json);

    if (response.find("\"success\":true") != std::string::npos) {
        std::cout << "[LOGIN] Welcome back, " << username << "!" << std::endl;
        return true;
    } else {
        std::cout << "[LOGIN] Failed: " << getStringValue(response, "message") << std::endl;
        return false;
    }
}

// =================================
// QUICK SAVE & LOAD FOR GAME LOOP
// ================================
class GameClient {
public:
    GameClient(const std::string& playerName) : m_playerName(playerName) {}
    
    void save(float x, float y, float z, int health) {
        saveGameToServer(m_playerName, x, y, z, health);
    }
    
    bool load(float& x, float& y, float& z, int& health) {
        return loadGameFromServer(m_playerName, x, y, z, health);
    }
    
    bool registerPlayer(const std::string& password) {
        return registerPlayer(m_playerName, password);
    }
    
    bool loginPlayer(const std::string& password) {
        return loginPlayer(m_playerName, password);
    }

private:
    std::string m_playerName;
};