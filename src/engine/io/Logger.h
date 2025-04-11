//
// Created by mgrus on 11.04.2025.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

class Logger {
public:
    // Store the reference to the output stream.
    Logger(std::ostream& stream);

    void log(const std::string &message);

private:
    std::ostream &logStream;
};


#endif //LOGGER_H
