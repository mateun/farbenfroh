//
// Created by mgrus on 26.03.2025.
//

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <string>

class FileHelper {
public:

    static bool isPngFile(const std::string& fileName);
    static bool isJpgFile(const std::string& fileName);
};



#endif //FILEHELPER_H
