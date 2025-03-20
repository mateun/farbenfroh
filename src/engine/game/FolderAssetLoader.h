//
// Created by mgrus on 20.09.2024.
//

#ifndef SIMPLE_KING_FOLDERASSETLOADER_H
#define SIMPLE_KING_FOLDERASSETLOADER_H

#include <string>
#include <map>
#include <engine/sound/sound.h>

#include "graphics.h"

class FolderAssetLoader {

public:
    void load(const std::string& folder);
    std::shared_ptr<Texture> getTexture(const std::string& name);
    Sound* getSound(const std::string& name);
    Mesh* getMesh(const std::string& name);

private:
    std::map<std::string, std::shared_ptr<Texture>> _textureMap;
    std::map<std::string, Mesh*> _meshMap;
    std::map<std::string, Sound*> _soundMap;

};


#endif //SIMPLE_KING_FOLDERASSETLOADER_H
