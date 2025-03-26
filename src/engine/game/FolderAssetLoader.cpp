//
// Created by mgrus on 20.09.2024.
//

#include "FolderAssetLoader.h"
#include <filesystem>

#include <iostream>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/StatefulRenderer.h>
#include <engine/io/MeshImporter.h>

FolderAssetLoader::FolderAssetLoader(HWND hwnd) : native_window_(hwnd){
}

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
                    std::string pureName = entry.path().filename().stem().string();
                    std::shared_ptr<Texture> texture = nullptr;
                    if (pureName.contains("_n") || pureName.contains("_norm")) {
                        // We use linear color space for normal maps, instead of sRGB for albedos.
                        texture = std::make_shared<Texture>(entry.path().string());
                    } else {
                        texture = std::make_shared<Texture>(entry.path().string());
                    }

                    _textureMap[pureName] = texture;
                }
                else if (extension == ".fbx" ||
                         extension == ".FBX" ||
                         extension == ".obj" ||
                         extension == ".glb" ||
                         extension == ".gltf") {
                    auto pureName = entry.path().filename().stem().string();
                    auto skeletonBaseFolder = assetFolder + "/skeletal/" + pureName;
                    auto mesh = AssimpMeshImporter().importMesh(entry.path().string(), skeletonBaseFolder);
                    _meshMap[pureName] = mesh;
                } else if (extension == ".wav") {auto sound = loadSoundFileExt(entry.path().string(), native_window_);
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

std::shared_ptr<Texture> FolderAssetLoader::getTexture(const std::string& name) {
    return _textureMap[name];
}

Sound * FolderAssetLoader::getSound(const std::string& name) {
    return _soundMap[name];
}

Mesh *FolderAssetLoader::getMesh(const std::string& name) {
    return _meshMap[name];
}
