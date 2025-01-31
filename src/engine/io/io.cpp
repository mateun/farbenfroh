//
// Created by mgrus on 20.12.2023.
//

#include "io.h"
#include <inttypes.h>
#include <string>
#include <mutex>
#include <queue>

#pragma comment(lib, "Ws2_32.lib")

static  SOCKET serverSocket = INVALID_SOCKET;
static  std::mutex queueMutex;
static std::queue<std::string> packetQueue;

std::string readFile(const std::string &fileName) {
    std::string result = "";

    printf("loading file: %s\n", fileName.c_str());
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f) {
        printf("file not found!\n");
        return result;
    }
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    rewind(f);

    char *buf = new char[size + 1];
    memset(buf, 0, sizeof(buf));
    int read = fread(buf, 1, size, f);
    if (read != size) {
        printf("error reading file\n.");
        exit(1);
    }
    buf[size] = '\0';
    result = buf;
    fclose(f);
    return result;

}


uint8_t *readFileBinary(const std::string &fileName, uint32_t *len) {
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f) {
        printf("ERROR: file not found %s\n", fileName.c_str());
        return nullptr;
    }

    // Get length in bytes
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    if (len) *len = size;
    rewind(f);


    uint8_t *buf = new uint8_t[size + 1];
    memset(buf, 0, sizeof(buf));
    int read = fread(buf, 1, size, f);
    if (read != size) {
        printf("error reading file\n.");
        exit(1);
    }
    fclose(f);
    return buf;
}

bool startSocketServer(int port) {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != NO_ERROR) {
        printf("WSAStartup failed with error %d\n", res);
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed with error %d\n", WSAGetLastError());
        return false;
    }

    struct sockaddr_in serverAddr;

    // Bind the socket to any address and the specified port.
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    // OR, you can do serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(serverSocket, (SOCKADDR *) & serverAddr, sizeof (serverAddr))) {
        printf("bind failed with error %d\n", WSAGetLastError());
        return false;
    }

    return true;

}

void receiveFromSocketServer() {
    int bytes_received;
    char serverBuf[1025];
    int serverBufLen = 1024;

    // Keep a seperate address struct to store sender information.
    struct sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof (SenderAddr);

    printf("Receiving datagrams on %s\n", "127.0.0.1");
    bytes_received = recvfrom(serverSocket, serverBuf, serverBufLen, 0 /* no flags*/, (SOCKADDR *) & SenderAddr, &SenderAddrSize);
    if (bytes_received == SOCKET_ERROR) {
        printf("recvfrom failed with error %d\n", WSAGetLastError());
    }
    serverBuf[bytes_received] = '\0';

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        packetQueue.push(serverBuf);
        printf("received: %s\n", serverBuf);
    }



}

std::string getLatestSocketMessage() {
    std::unique_lock<std::mutex> lock(queueMutex, std::defer_lock);
    if (lock.try_lock()) {
        if (!packetQueue.empty()) {
            // A packet has arrived
            auto packet = packetQueue.front();
            packetQueue.pop();
            lock.unlock(); // Unlock as soon as possible

            // Process the packet
            return packet;
        } else {
            lock.unlock(); // Unlock if no packet is available
        }
    }
    return {};
}

float bytesToFloat(const std::vector<uint8_t>& bytes) {
    float fval = 0;
    *((uint8_t *)(&fval) + 3) = bytes[3];
    *((uint8_t*)(&fval) + 2) = bytes[2];
    *((uint8_t*)(&fval) + 1) = bytes[1];
    *((uint8_t*)(&fval) + 0) = bytes[0];
    return fval;
}

uint16_t bytesToUint16(const std::vector<uint8_t>& bytes) {
    uint16_t val;
    *((uint8_t*)(&val) + 1) = bytes[1];
    *((uint8_t*)(&val) + 0) = bytes[0];
    return val;
}
