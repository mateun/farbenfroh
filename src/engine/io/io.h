//
// Created by mgrus on 20.12.2023.
//

#ifndef SIMPLE_KING_IO_H
#define SIMPLE_KING_IO_H
#include <string>
#include <vector>

std::string readFile(const std::string& fileName);
uint8_t* readFileBinary(const std::string& fileName, uint32_t *size);

bool startSocketServer(int port);
void receiveFromSocketServer();
std::string getLatestSocketMessage();

float bytesToFloat(const std::vector<uint8_t>& bytes);
uint16_t bytesToUint16(const std::vector<uint8_t>& bytes);


#endif //SIMPLE_KING_IO_H
