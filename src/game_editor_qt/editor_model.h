//
// Created by mgrus on 17.04.2025.
//

#ifndef EDITOR_MODEL_H
#define EDITOR_MODEL_H

#include <string>
#include <vector>
#include <glm/vec3.hpp>

namespace edqt {

  struct GameObject;
  struct Component {
    virtual ~Component() {}
    GameObject* owner = nullptr;
    bool enabled = true;

  };

  struct TransformComponent : public Component {
    glm::vec3 position = {0,0, 0};
    glm::vec3 orientation_euler = {0, 0, 0};
    glm::vec3 scale = {1, 1, 1};
  };

  struct MeshComponent : public Component {

  };

  struct GameObject {
    std::string name;
    std::vector<std::unique_ptr<Component>> components;

    template<typename T>
    std::vector<T*> getComponents() {
      std::vector<T*> result;
      for (auto& comp : components) {
        if (auto casted = dynamic_cast<T*>(comp.get())) {
          result.push_back(casted);
        }
      }
      return result;
    }

    template<typename T>
    void addComponent(std::unique_ptr<T> component) {
      components.push_back(component);
    }

  };

struct Asset {
  std::string name;
  std::string relativePath;

};

struct Level {
  std::string name;
  std::string relativePath;

  std::vector<GameObject*> gameObjects;

};

struct Project {
  std::string name;
  std::string editor_startup_level;

  std::string systemFilePath; // absolute path to project folder

  // Only populated during editor runtime:
  std::vector<Level*> openLevels;      // currently open levels (e.g., tabs)
  std::vector<Asset*> loadedAssets;    // currently used/previewed assets

  // Optionally cached (but not serialized)
  std::vector<std::string> allLevelPaths; // discovered from Levels/
  std::vector<std::string> allAssetPaths; // discovered from Assets/
};


}





#endif //EDITOR_MODEL_H
