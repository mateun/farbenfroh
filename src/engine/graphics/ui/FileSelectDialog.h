//
// Created by mgrus on 11.04.2025.
//

#ifndef FILESELECTDIALOG_H
#define FILESELECTDIALOG_H
#include <string>


class FileSelectDialog {

  public:
    std::string open(const std::string& typeFilter);

    std::wstring openForDirectory();
};



#endif //FILESELECTDIALOG_H
