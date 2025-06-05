//
// Created by mgrus on 04.06.2025.
//


#include <cstdint>
#include <fstream>
#include "../include/util.h"

std::vector<uint8_t> read_file_binary_to_vector(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open GLB file.");
    }

    // Seek to end to get size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read all data
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    return data;
}

uint8_t* read_file_binary(const std::string& filename) {
    return read_file_binary_to_vector(filename).data();
}