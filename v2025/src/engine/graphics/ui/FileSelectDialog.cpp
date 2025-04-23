//
// Created by mgrus on 11.04.2025.
//

#include <Windows.h>
#include <shlobj.h>
#include "FileSelectDialog.h"

#include <iostream>
#include <string>
#include <engine/graphics/Application.h>

std::string FileSelectDialog::open(const std::string& typeFilter) {
    //std::string typeFilter = "All\0*.*\0obj\0*.obj\0fbx\0*.fbx\0gltf\0*.gltf";

    OPENFILENAME ofn;       // Common dialog box structure
    char szFile[260] = {0}; // Buffer for file name
    HWND hwnd = getApplication()->hwnd();       // Owner window


    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = typeFilter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box
    if (GetOpenFileName(&ofn) == TRUE) {
        std::cout << "Selected file: " << ofn.lpstrFile << std::endl;
        return ofn.lpstrFile;

    }

    std::cout << "No file selected or operation canceled." << std::endl;
    return "";


}

std::wstring FileSelectDialog::openForDirectory() {
    IFileDialog* pFileDialog = nullptr;
    std::wstring folderPath;

    // Create the File Open Dialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr)) {
        // Set options on the dialog.
        DWORD dwOptions;
        if (SUCCEEDED(pFileDialog->GetOptions(&dwOptions))) {
            pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        // Show the dialog.
        hr = pFileDialog->Show(getApplication()->hwnd());
        if (SUCCEEDED(hr)) {
            // Get the result (the selected folder).
            IShellItem* pItem = nullptr;
            hr = pFileDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr)) {
                    folderPath = pszPath;
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        pFileDialog->Release();
    }
    return folderPath;
}
