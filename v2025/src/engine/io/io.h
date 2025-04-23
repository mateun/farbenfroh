//
// Created by mgrus on 20.12.2023.
//

#ifndef IO_H
#define IO_H
#include <string>
#include <vector>

std::string readFile(const std::string& fileName);
uint8_t* readFileBinary(const std::string& fileName, uint32_t *size);

bool startSocketServer(int port);
void receiveFromSocketServer();
std::string getLatestSocketMessage();

float bytesToFloat(const std::vector<uint8_t>& bytes);
uint16_t bytesToUint16(const std::vector<uint8_t>& bytes);

// Windows native wstring to string converter
std::string WStringToUtf8(const std::wstring& wstr);

#endif //IO_H
