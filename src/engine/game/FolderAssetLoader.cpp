//
// Created by mgrus on 20.09.2024.
//

#include "FolderAssetLoader.h"
#include <filesystem>
#include <iostream>

extern HWND getWindow();

void FolderAssetLoader::load(const std::string &assetFolder) {
    namespace fs = std::filesystem;
    try {
        for (const auto & entry : fs::directory_iterator(assetFolder)) {
            if (fs::is_regular_file(entry.status())) {
                printf("%s\n", entry.path().string().c_str());

                // Decide on file type which importer to trigger
                auto extension = entry.path().extension().string();
                if (extension == ".png" ||
                    extension == ".bmp" ||
                    extension == ".jpg" ) {
                    auto texture = createTextureFromFile(entry.path().string(), ColorFormat::RGBA);
                    _textureMap[entry.path().filename().stem().string()] = texture;
                }
                else if (extension == ".fbx" ||
                         extension == ".FBX" ||
                         extension == ".obj" ||
                         extension == ".glb" ||
                         extension == ".gltf") {
                    auto mesh = MeshImporter().importMesh(entry.path().string());
                    _meshMap[entry.path().filename().stem().string()] = mesh;
                } else if (extension == ".wav") {auto sound = loadSoundFileExt(entry.path().string(), getWindow());
                    _soundMap[entry.path().filename().stem().string()] = sound;

                }


            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
    }
}

Texture *FolderAssetLoader::getTexture(const std::string name) {
    return _textureMap[name];
}

Sound * FolderAssetLoader::getSound(const std::string name) {
    return _soundMap[name];
}

Mesh *FolderAssetLoader::getMesh(const std::string name) {
    return _meshMap[name];
}
