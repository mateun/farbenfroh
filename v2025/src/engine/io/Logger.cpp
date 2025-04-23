//
// Created by mgrus on 11.04.2025.
//

#include "Logger.h"


Logger::Logger(std::ostream& stream) : logStream(stream) {}


void Logger::log(const std::string &message) {
    logStream << message << std::endl;
}