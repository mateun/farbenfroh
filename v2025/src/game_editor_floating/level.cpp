//
// Created by mgrus on 14.04.2025.
//
#include <Windows.h>
#include <string>
#include <shlobj_core.h>
#include <shobjidl_core.h>
#include <engine/io/nljson.h>
#include <fstream>

void createEmptyLevel(const std::string& name, const std::string& projectFolder, HWND parentWindow) {
    std::string folderName = projectFolder + "/" + name;
    auto folderError = ((SHCreateDirectoryEx(parentWindow, folderName.c_str(), nullptr) != ERROR_SUCCESS &&GetLastError() != ERROR_ALREADY_EXISTS));
    if (folderError) {
        // TODO error display
        return;
    }

    nlohmann::json j;
    j["projectName"] =name;
    j["gameObjects"] = j.array();

    std::ofstream f(folderName + "/main.json");
    f << j.dump(4);

}
