//
// Created by mgrus on 26.03.2025.
//

#include "FileHelper.h"
#include <string>

bool FileHelper::isPngFile(const std::string& fileName) {
    int len = fileName.size();
    return ((fileName[len-1] == 'g'  || fileName[len-1] == 'G') &&
            (fileName[len-2] == 'n' || fileName[len-2] == 'N') &&
            (fileName[len-3] == 'p' || fileName[len-3] == 'P') &&
        fileName[len-4] == '.' );
}

bool FileHelper::isJpgFile(const std::string& fileName) {
    int len = fileName.size();
    return (fileName[len-1] == 'g' &&
            fileName[len-2] == 'p' &&
            fileName[len-3] == 'j' &&
            fileName[len-4] == '.' );
}