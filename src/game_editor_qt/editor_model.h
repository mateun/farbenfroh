//
// Created by mgrus on 17.04.2025.
//

#ifndef EDITOR_MODEL_H
#define EDITOR_MODEL_H

#include <string>
#include <vector>

struct Asset {
  std::string name;
  std::string relativePath;

};

struct Level {
  std::string name;
  std::string relativePath;

};

struct Project {
  std::string name;

  std::string systemFilePath; // absolute path to project folder

  // Only populated during editor runtime:
  std::vector<Level*> openLevels;      // currently open levels (e.g., tabs)
  std::vector<Asset*> loadedAssets;    // currently used/previewed assets

  // Optionally cached (but not serialized)
  std::vector<std::string> allLevelPaths; // discovered from Levels/
  std::vector<std::string> allAssetPaths; // discovered from Assets/
};





#endif //EDITOR_MODEL_H
